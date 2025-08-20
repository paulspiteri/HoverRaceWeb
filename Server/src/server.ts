import express from "express";
import cors from "cors";
import { v4 as uuidv4 } from "uuid";
import { gameManager } from "./gameManager.ts";
import type {
    CreateGameRequest,
    JoinGameRequest,
    LeaveGameRequest,
    DeleteGameRequest,
    SignalRequest,
    AvailableGame,
    ServerGame,
    JoinedGame,
    ClientPlayer,
    ConnectionIdMessage,
    GameListMessage,
    GameUpdatedMessage,
    GameUpdatedFullMessage,
    GameDeletedMessage,
    SignalMessage,
    ServerMessage,
} from "./types";

const app = express();
const port = 3001;

app.use(cors());
app.use(express.json());

// Store SSE connections with associated creator IDs
const sseClients = new Map<express.Response, string>();

// SSE endpoint for real-time game list updates
app.get("/api/games/stream", (req, res) => {
    res.writeHead(200, {
        "Content-Type": "text/event-stream",
        "Cache-Control": "no-cache",
        Connection: "keep-alive",
        "Access-Control-Allow-Origin": "*",
    });

    // Generate unique connection ID for this client
    const connectionId = uuidv4();
    console.log(
        `📡 SSE connection established   🔑 Generated connectionId: ${connectionId}`
    );

    // Send connection ID and current games immediately (public data only)
    const games = gameManager.getAllGames();
    const publicGames = games.map(toPublicGameData);
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

        // If this connection created any games, delete them
        if (disconnectedConnectionId) {
            const games = gameManager.getAllGames();
            games.forEach((game) => {
                if (game.creatorConnectionId === disconnectedConnectionId) {
                    console.log(
                        `🗑️ Auto-deleting game ${game.id} (creator disconnected)`
                    );
                    gameManager.deleteGame(game.id);
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

        if (
            !gameData.name ||
            !gameData.maxPlayers ||
            !gameData.creatorConnectionId
        ) {
            console.log("❌ Missing required fields");
            return res.status(400).json({ error: "Missing required fields" });
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
        const { connectionId }: JoinGameRequest = req.body;
        console.log(`🎯 Joining game ${id} with connectionId: ${connectionId}`);

        if (!connectionId) {
            console.log("❌ Missing connectionId");
            return res.status(400).json({ error: "Missing connectionId" });
        }

        // Validate connectionId exists in active SSE connections
        const isValidConnectionId = Array.from(sseClients.values()).includes(
            connectionId
        );
        if (!isValidConnectionId) {
            console.log("❌ Invalid or inactive connectionId");
            return res
                .status(400)
                .json({ error: "Invalid or inactive connectionId" });
        }

        const gameToken = gameManager.joinGame(id, connectionId);

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
        const { gameToken }: LeaveGameRequest = req.body;
        console.log(`🎯 Leaving game ${id} with token`);

        if (!gameToken) {
            console.log("❌ Missing gameToken");
            return res.status(400).json({ error: "Missing gameToken" });
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
        const { creatorToken }: DeleteGameRequest = req.body;
        console.log(`🎯 Deleting game ${id} with creator token`);

        if (!creatorToken) {
            console.log("❌ Missing creatorToken");
            return res.status(400).json({ error: "Missing creatorToken" });
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
        const { targetConnectionId, gameToken, signalData }: SignalRequest = req.body;
        console.log(`🎯 Sending signal in game ${id} to ${targetConnectionId}`);

        if (!targetConnectionId || !gameToken || !signalData) {
            console.log("❌ Missing required fields");
            return res.status(400).json({ error: "Missing required fields: targetConnectionId, gameToken, signalData" });
        }

        const game = gameManager.getGame(id);
        if (!game) {
            console.log(`❌ Game ${id} not found`);
            return res.status(404).json({ error: "Game not found" });
        }

        // Find sender by gameToken
        const sender = game.players.find(p => p.gameToken === gameToken);
        if (!sender) {
            console.log("❌ Invalid gameToken or sender not in game");
            return res.status(403).json({ error: "Invalid gameToken or not a member of this game" });
        }

        // Verify target is also in the same game
        const target = game.players.find(p => p.connectionId === targetConnectionId);
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
            signalData
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

// Convert Game to PublicGameData
function toPublicGameData(game: ServerGame): AvailableGame {
    return {
        id: game.id,
        name: game.name,
        playerCount: game.players.length,
        maxPlayers: game.maxPlayers,
        createdAt: game.createdAt,
        creatorConnectionId: game.creatorConnectionId,
    };
}

// Convert ServerGame to JoinedGame (removes tokens)
function toJoinedGame(game: ServerGame): JoinedGame {
    return {
        id: game.id,
        name: game.name,
        players: game.players.map(
            (p): ClientPlayer => ({ connectionId: p.connectionId })
        ),
        maxPlayers: game.maxPlayers,
        createdAt: game.createdAt,
        creatorConnectionId: game.creatorConnectionId,
    };
}

// Broadcast public game updates to non-participant SSE clients only
function broadcastPublicUpdate(
    data: ServerMessage,
    excludeConnectionIds?: string[]
) {
    const message = `data: ${JSON.stringify(data)}\n\n`;
    let sentCount = 0;
    sseClients.forEach((connectionId, client) => {
        if (
            !excludeConnectionIds ||
            !excludeConnectionIds.includes(connectionId)
        ) {
            try {
                client.write(message);
                sentCount++;
            } catch {
                sseClients.delete(client);
            }
        }
    });
    console.log(
        `📤 Public broadcast sent to ${sentCount} non-participant clients`
    );
}

// Broadcast private game updates only to participants
function broadcastPrivateUpdate(game: ServerGame, data: ServerMessage) {
    const message = `data: ${JSON.stringify(data)}\n\n`;
    const participantConnectionIds = game.players.map((p) => p.connectionId);

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

// Shared function to broadcast game updates
function broadcastGameUpdate(game: ServerGame) {
    const participantConnectionIds = game.players.map((p) => p.connectionId);

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

app.listen(port, () => {
    console.log(`Server running on port ${port}`);
});
