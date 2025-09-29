import express from "express";
import cors from "cors";
import dotenv from "dotenv";
import { v4 as uuidv4 } from "uuid";
import { gameManager } from "./gameManager.ts";
import type {
    CreateGameRequest,
    JoinGameRequest,
    UpdateGameRequest,
    ChatMessage,
    AvailableGame,
    ServerGame,
    JoinedGame,
    ConnectionIdMessage,
    GameListMessage,
    GameUpdatedMessage,
    GameUpdatedFullMessage,
    GameDeletedMessage,
    SignalMessage,
    ChatMessageServerMessage,
    ServerMessage,
} from "./types";

dotenv.config({ path: ".env" });

if (!process.env.CLIENT_URL) {
    console.error("❌ FATAL: CLIENT_URL environment variable is not set");
    process.exit(1);
}
if (!process.env.PORT) {
    console.error("❌ FATAL: PORT environment variable is not set");
    process.exit(1);
}

const app = express();
const port = Number(process.env.PORT);
const clientUrl = process.env.CLIENT_URL;

console.log(`🔧 Server configuration:`);
console.log(`   PORT: ${port}`);
console.log(`   CLIENT_URL: ${clientUrl}`);
console.log(`   NODE_ENV: ${process.env.NODE_ENV}`);

app.use(
    cors({
        origin: clientUrl.split(","),
    }),
);
app.use(express.json());

app.get("/health", (req, res) => {
    res.status(200).json({
        status: "OK",
        timestamp: new Date().toISOString(),
        uptime: process.uptime(),
    });
});

// Store SSE connections with associated creator IDs
const sseClients = new Map<express.Response, string>();

// SSE endpoint for real-time game list updates
app.get("/api/games/stream", (req, res) => {
    res.writeHead(200, {
        "Content-Type": "text/event-stream",
        "Cache-Control": "no-cache",
        Connection: "keep-alive",
        "Access-Control-Allow-Credentials": "true",
        "Access-Control-Allow-Headers": "Cache-Control",
    });

    // Generate unique connection ID for this client
    const connectionId = uuidv4();
    console.log(`📡 SSE connection established   🔑 Generated connectionId: ${connectionId}`);

    // Send connection ID and current games immediately (public data only)
    const games = gameManager.getAllGames();
    const publicGames = games.filter((x) => x.status === "waiting").map(toPublicGameData);
    const connectionIdMessage: ConnectionIdMessage = {
        type: "connectionId",
        connectionId,
    };
    const gameListMessage: GameListMessage = {
        type: "gameList",
        games: publicGames,
    };

    res.write(`data: ${JSON.stringify(connectionIdMessage)}\n\n`);
    res.write(`data: ${JSON.stringify(gameListMessage)}\n\n`);

    sseClients.set(res, connectionId);

    req.on("close", () => {
        const disconnectedConnectionId = sseClients.get(res);
        console.log(`🔌 SSE connection closed: ${disconnectedConnectionId}`);
        sseClients.delete(res);

        // Remove disconnected players from games (including creators)
        if (disconnectedConnectionId) {
            const games = gameManager.getAllGames();
            games.forEach((game) => {
                const player = game.players.find((p) => p?.connectionId === disconnectedConnectionId);
                if (player && player.gameToken) {
                    console.log(
                        `🚪 Auto-removing player ${player.name || disconnectedConnectionId} from game ${game.id} (player disconnected)`,
                    );
                    gameManager.leaveGame(game.id, player.gameToken);
                }
            });
        }
    });
});

// REST endpoint to create a game
app.post("/api/games", (req, res) => {
    console.log("🎮 POST /api/games - Create game request");
    try {
        const gameData: CreateGameRequest = req.body;
        console.log(`📝 Game data:`, gameData);

        if (!gameData.maxPlayers || !gameData.creatorConnectionId) {
            console.log("❌ Missing required fields");
            return res.status(400).json({ error: "Missing required fields: maxPlayers, creatorConnectionId" });
        }

        const { game, creatorToken } = gameManager.createGame(gameData);
        console.log(`✅ Game created: ${game.id} - "${game.name}"`);
        res.status(201).json({
            game: toJoinedGame(game), // Safe version without tokens in players array
            creatorToken, // But creator gets their token separately
        });
    } catch (error) {
        console.log("💥 Error creating game:", error);
        res.status(500).json({ error: "Failed to create game" });
    }
});

// REST endpoint to join a game
app.post("/api/games/:id/join", (req, res) => {
    console.log("👥 POST /api/games/:id/join - Join game request");
    try {
        const { id } = req.params;
        const { connectionId, name }: JoinGameRequest = req.body;
        console.log(`🎯 Joining game ${id} with connectionId: ${connectionId}, name: ${name}`);

        if (!connectionId) {
            console.log("❌ Missing connectionId");
            return res.status(400).json({ error: "Missing connectionId" });
        }

        // Validate connectionId exists in active SSE connections
        const isValidConnectionId = Array.from(sseClients.values()).includes(connectionId);
        if (!isValidConnectionId) {
            console.log("❌ Invalid or inactive connectionId");
            return res.status(400).json({ error: "Invalid or inactive connectionId" });
        }

        const gameToken = gameManager.joinGame(id, connectionId, name);

        if (gameToken) {
            console.log(`✅ Successfully joined game ${id}`);
            res.status(200).json({
                message: "Joined game successfully",
                gameToken,
            });
        } else {
            console.log(`❌ Failed to join game ${id}`);
            res.status(400).json({ error: "Cannot join game" });
        }
    } catch (error) {
        console.log("💥 Error joining game:", error);
        res.status(500).json({ error: "Failed to join game" });
    }
});

// REST endpoint to leave a game
app.post("/api/games/:id/leave", (req, res) => {
    console.log("🚪 POST /api/games/:id/leave - Leave game request");
    try {
        const { id } = req.params;
        const gameToken = req.headers.authorization?.replace("Bearer ", "");
        console.log(`🎯 Leaving game ${id} with token`);

        if (!gameToken) {
            console.log("❌ Missing gameToken in Authorization header");
            return res.status(400).json({ error: "Missing gameToken in Authorization header" });
        }

        const left = gameManager.leaveGame(id, gameToken);

        if (left) {
            console.log(`✅ Successfully left game ${id}`);
            res.status(200).json({ message: "Left game successfully" });
        } else {
            console.log(`❌ Failed to leave game ${id} - invalid token`);
            res.status(400).json({
                error: "Invalid gameToken or cannot leave game",
            });
        }
    } catch (error) {
        console.log("💥 Error leaving game:", error);
        res.status(500).json({ error: "Failed to leave game" });
    }
});

// REST endpoint to delete a game (token-based)
app.delete("/api/games/:id", (req, res) => {
    console.log("🗑️ DELETE /api/games/:id - Delete game request");
    try {
        const { id } = req.params;
        const creatorToken = req.headers.authorization?.replace("Bearer ", "");
        console.log(`🎯 Deleting game ${id} with creator token`);

        if (!creatorToken) {
            console.log("❌ Missing creatorToken in Authorization header");
            return res.status(400).json({ error: "Missing creatorToken in Authorization header" });
        }

        const deleted = gameManager.deleteGameByToken(id, creatorToken);

        if (deleted) {
            console.log(`✅ Successfully deleted game ${id}`);
            res.status(204).send();
        } else {
            console.log(`❌ Failed to delete game ${id} - invalid token`);
            res.status(400).json({
                error: "Invalid creatorToken or cannot delete game",
            });
        }
    } catch (error) {
        console.log("💥 Error deleting game:", error);
        res.status(500).json({ error: "Failed to delete game" });
    }
});

// REST endpoint to send WebRTC signaling data to specific peer
app.post("/api/games/:id/signal", (req, res) => {
    console.log("📡 POST /api/games/:id/signal - Send signal request");
    try {
        const { id } = req.params;
        const gameToken = req.headers.authorization?.replace("Bearer ", "");
        const { targetConnectionId, signalData } = req.body;
        console.log(`🎯 Sending signal in game ${id} to ${targetConnectionId}`);

        if (!targetConnectionId || !gameToken || !signalData) {
            console.log("❌ Missing required fields");
            return res.status(400).json({
                error: "Missing required fields: targetConnectionId, gameToken (Authorization header), signalData",
            });
        }

        const game = gameManager.getGame(id);
        if (!game) {
            console.log(`❌ Game ${id} not found`);
            return res.status(404).json({ error: "Game not found" });
        }

        // Find sender by gameToken
        const sender = game.players.find((p) => p?.gameToken === gameToken);
        if (!sender) {
            console.log("❌ Invalid gameToken or sender not in game");
            return res.status(403).json({
                error: "Invalid gameToken or not a member of this game",
            });
        }

        // Verify target is also in the same game
        const target = game.players.find((p) => p?.connectionId === targetConnectionId);
        if (!target) {
            console.log(`❌ Target ${targetConnectionId} not in game ${id}`);
            return res.status(400).json({ error: "Target player not in this game" });
        }

        // Find target's SSE connection
        let targetClient: express.Response | undefined;
        for (const [client, connectionId] of sseClients.entries()) {
            if (connectionId === targetConnectionId) {
                targetClient = client;
                break;
            }
        }

        if (!targetClient) {
            console.log(`❌ Target ${targetConnectionId} not connected via SSE`);
            return res.status(400).json({ error: "Target player not connected" });
        }

        // Send signal only to target
        const signalMessage: SignalMessage = {
            type: "signal",
            fromConnectionId: sender.connectionId,
            signalData,
        };

        try {
            targetClient.write(`data: ${JSON.stringify(signalMessage)}\n\n`);
            console.log(`✅ Signal sent from ${sender.connectionId} to ${targetConnectionId}`);
            res.status(200).json({ message: "Signal sent successfully" });
        } catch (writeError) {
            console.log(`❌ Failed to send signal to ${targetConnectionId}:`, writeError);
            // Remove dead connection
            sseClients.delete(targetClient);
            res.status(500).json({ error: "Failed to deliver signal" });
        }
    } catch (error) {
        console.log("💥 Error sending signal:", error);
        res.status(500).json({ error: "Failed to send signal" });
    }
});

// REST endpoint to update player information
app.put("/api/games/:id/player", (req, res) => {
    console.log("✏️ PUT /api/games/:id/player - Update player request");
    try {
        const { id } = req.params;
        const gameToken = req.headers.authorization?.replace("Bearer ", "");
        const { name } = req.body;
        console.log(`🎯 Updating player in game ${id} with name: ${name}`);

        if (!gameToken || !name) {
            console.log("❌ Missing required fields");
            return res.status(400).json({ error: "Missing required fields: gameToken (Authorization header), name" });
        }

        const updated = gameManager.updatePlayer(id, gameToken, name);

        if (updated) {
            console.log(`✅ Successfully updated player in game ${id}`);
            res.status(200).json({ message: "Player updated successfully" });
        } else {
            console.log(`❌ Failed to update player in game ${id} - invalid token or game not found`);
            res.status(400).json({
                error: "Invalid gameToken or game not found",
            });
        }
    } catch (error) {
        console.log("💥 Error updating player:", error);
        res.status(500).json({ error: "Failed to update player" });
    }
});

// REST endpoint to update a game (creator only)
app.put("/api/games/:id", (req, res) => {
    console.log("⚙️ PUT /api/games/:id - Update game request");
    try {
        const { id } = req.params;
        const creatorToken = req.headers.authorization?.replace("Bearer ", "");
        const updates: UpdateGameRequest = req.body;
        console.log(`🎯 Updating game ${id} with creator token:`, updates);

        if (!creatorToken) {
            console.log("❌ Missing creatorToken in Authorization header");
            return res.status(400).json({ error: "Missing creatorToken in Authorization header" });
        }

        const updated = gameManager.updateGame(id, creatorToken, updates);

        if (updated) {
            console.log(`✅ Successfully updated game ${id}`);
            res.status(200).json({ message: "Game updated successfully" });
        } else {
            console.log(`❌ Failed to update game ${id} - invalid token, game not found, or already started`);
            res.status(400).json({
                error: "Invalid creatorToken, game not found, or game already started",
            });
        }
    } catch (error) {
        console.log("💥 Error updating game:", error);
        res.status(500).json({ error: "Failed to update game" });
    }
});

// REST endpoint to start a game
app.post("/api/games/:id/start", (req, res) => {
    console.log("🚀 POST /api/games/:id/start - Start game request");
    try {
        const { id } = req.params;
        const creatorToken = req.headers.authorization?.replace("Bearer ", "");
        console.log(`🎯 Starting game ${id} with creator token`);

        if (!creatorToken) {
            console.log("❌ Missing creatorToken in Authorization header");
            return res.status(400).json({ error: "Missing creatorToken in Authorization header" });
        }

        const started = gameManager.startGame(id, creatorToken);

        if (started) {
            console.log(`✅ Successfully started game ${id}`);
            res.status(200).json({ message: "Game started successfully" });
        } else {
            console.log(`❌ Failed to start game ${id} - invalid token, game not found, or already started`);
            res.status(400).json({
                error: "Invalid creatorToken, game not found, or game already started",
            });
        }
    } catch (error) {
        console.log("💥 Error starting game:", error);
        res.status(500).json({ error: "Failed to start game" });
    }
});

// REST endpoint to get chat history for a game
app.get("/api/games/:id/chat-history", (req, res) => {
    console.log("📜 GET /api/games/:id/chat-history - Get chat history request");
    try {
        const { id } = req.params;
        const gameToken = req.headers.authorization?.replace("Bearer ", "");
        console.log(`🎯 Getting chat history for game ${id}`);

        if (!gameToken) {
            console.log("❌ Missing gameToken in Authorization header");
            return res.status(400).json({ error: "Missing gameToken in Authorization header" });
        }

        const game = gameManager.getGame(id);

        // Find player by gameToken to verify they're in the game
        const player = game.players.find((p) => p?.gameToken === gameToken);
        if (!player) {
            console.log("❌ Invalid gameToken or player not in game");
            return res.status(403).json({
                error: "Invalid gameToken or not a member of this game",
            });
        }

        console.log(`✅ Sending ${game.chatMessages.length} chat messages to ${player.name || player.connectionId}`);
        res.status(200).json({ messages: game.chatMessages });
    } catch (error) {
        console.log("💥 Error getting chat history:", error);
        res.status(500).json({ error: "Failed to get chat history" });
    }
});

// REST endpoint to send chat message to a game
app.post("/api/games/:id/chat", (req, res) => {
    console.log("💬 POST /api/games/:id/chat - Send chat message request");
    try {
        const { id } = req.params;
        const gameToken = req.headers.authorization?.replace("Bearer ", "");
        const { message } = req.body;
        console.log(`🎯 Sending chat message to game ${id}`);

        if (!message || !gameToken) {
            console.log("❌ Missing required fields");
            return res
                .status(400)
                .json({ error: "Missing required fields: message, gameToken (Authorization header)" });
        }

        const game = gameManager.getGame(id);
        if (!game) {
            console.log(`❌ Game ${id} not found`);
            return res.status(404).json({ error: "Game not found" });
        }

        // Find sender by gameToken
        const sender = game.players.find((p) => p?.gameToken === gameToken);
        if (!sender) {
            console.log("❌ Invalid gameToken or sender not in game");
            return res.status(403).json({
                error: "Invalid gameToken or not a member of this game",
            });
        }

        // Create chat message
        const chatMessage: ChatMessage = {
            id: uuidv4(),
            message,
            senderId: sender.connectionId,
            senderName: sender.name,
            timestamp: new Date(),
            gameId: id,
        };

        // Store the message in the game
        game.chatMessages.push(chatMessage);

        // Broadcast to all players in the game
        broadcastChatMessage(game, chatMessage);

        console.log(`✅ Chat message sent to game ${id} by ${sender.name || sender.connectionId}`);
        res.status(200).json({ message: "Chat message sent successfully" });
    } catch (error) {
        console.log("💥 Error sending chat message:", error);
        res.status(500).json({ error: "Failed to send chat message" });
    }
});

// Convert Game to PublicGameData
function toPublicGameData(game: ServerGame): AvailableGame {
    return {
        id: game.id,
        name: game.name,
        playerCount: game.players.filter((p) => p !== undefined).length,
        maxPlayers: game.maxPlayers,
        createdAt: game.createdAt,
        status: game.status,
        trackName: game.trackName,
        hasWeapons: game.hasWeapons,
        laps: game.laps,
    };
}

// Convert ServerGame to JoinedGame (removes tokens)
function toJoinedGame(game: ServerGame): JoinedGame {
    return {
        id: game.id,
        name: game.name,
        playerCount: game.players.filter((p) => p !== undefined).length,
        players: game.players.map((p) => (p ? { connectionId: p.connectionId, name: p.name } : undefined)),
        maxPlayers: game.maxPlayers,
        createdAt: game.createdAt,
        status: game.status,
        trackName: game.trackName,
        hasWeapons: game.hasWeapons,
        laps: game.laps,
    };
}

// Broadcast public game updates to non-participant SSE clients only
function broadcastPublicUpdate(data: ServerMessage, excludeConnectionIds?: string[]) {
    const message = `data: ${JSON.stringify(data)}\n\n`;
    let sentCount = 0;
    sseClients.forEach((connectionId, client) => {
        if (!excludeConnectionIds || !excludeConnectionIds.includes(connectionId)) {
            try {
                client.write(message);
                sentCount++;
            } catch {
                sseClients.delete(client);
            }
        }
    });
    console.log(`📤 Public broadcast sent to ${sentCount} non-participant clients`);
}

// Broadcast private game updates only to participants
function broadcastPrivateUpdate(game: ServerGame, data: ServerMessage) {
    const message = `data: ${JSON.stringify(data)}\n\n`;
    const participantConnectionIds = game.players.filter((p) => p !== undefined).map((p) => p!.connectionId);

    sseClients.forEach((connectionId, client) => {
        if (participantConnectionIds.includes(connectionId)) {
            try {
                client.write(message);
            } catch {
                sseClients.delete(client);
            }
        }
    });
}

// Broadcast chat message only to participants of the game
function broadcastChatMessage(game: ServerGame, chatMessage: ChatMessage) {
    const chatMessageServerMessage: ChatMessageServerMessage = {
        type: "chatMessage",
        chatMessage,
    };
    broadcastPrivateUpdate(game, chatMessageServerMessage);
}

// Shared function to broadcast game updates
function broadcastGameUpdate(game: ServerGame) {
    const participantConnectionIds = game.players.filter((p) => p !== undefined).map((p) => p!.connectionId);

    // Send public data to non-participants
    const gameUpdatedMessage: GameUpdatedMessage = {
        type: "gameUpdated",
        game: toPublicGameData(game),
    };
    broadcastPublicUpdate(gameUpdatedMessage, participantConnectionIds);

    // Send full data to participants only (without tokens)
    const gameUpdatedFullMessage: GameUpdatedFullMessage = {
        type: "gameUpdatedFull",
        game: toJoinedGame(game),
    };
    broadcastPrivateUpdate(game, gameUpdatedFullMessage);
}

// Listen for game events and broadcast
gameManager.on("gameCreated", (game: ServerGame) => {
    console.log(`📡 Broadcasting gameUpdated for game ${game.id} (created)`);
    broadcastGameUpdate(game);
});

gameManager.on("gameDeleted", (gameId: string) => {
    console.log(`📡 Broadcasting gameDeleted for game ${gameId}`);

    // Token cleanup is now handled automatically since tokens are stored on players

    // Game deletion is public information
    const gameDeletedMessage: GameDeletedMessage = {
        type: "gameDeleted",
        gameId,
    };
    broadcastPublicUpdate(gameDeletedMessage);
});

gameManager.on("gameUpdated", (game: ServerGame) => {
    console.log(`📡 Broadcasting gameUpdated for game ${game.id}`);
    broadcastGameUpdate(game);
});

app.listen(port, "0.0.0.0", () => {
    console.log(`Server running on port ${port}`);
});
