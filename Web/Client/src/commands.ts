import type {
    CreateGameRequest,
    SignalRequest,
    JoinGameRequest,
    UpdatePlayerRequest,
    UpdateGameRequest,
    StartGameRequest,
    SendChatMessageRequest,
    ChatMessage,
} from "@/types.ts";

export const createCommands = (baseUrl: string, setActiveGame: (id: string | undefined, token?: string) => void) => {
    const createGame = async (connectionId: string, creatorName?: string) => {
        try {
            const response = await fetch(`${baseUrl}/games`, {
                method: "POST",
                headers: {
                    "Content-Type": "application/json",
                },
                body: JSON.stringify({
                    name: `Game ${Date.now()}`,
                    maxPlayers: 8,
                    creatorConnectionId: connectionId,
                    creatorName,
                } satisfies CreateGameRequest),
            });

            if (!response.ok) {
                throw new Error("Failed to create game");
            }

            const result = await response.json();
            console.log("Game created:", result);

            // Store the creator token
            if (result.creatorToken && result.game) {
                setActiveGame(result.game.id, result.creatorToken);
            }
        } catch (error) {
            console.error("Error creating game:", error);
        }
    };

    const joinGame = async (gameId: string, connectionId: string, name?: string) => {
        try {
            const response = await fetch(`${baseUrl}/games/${gameId}/join`, {
                method: "POST",
                headers: {
                    "Content-Type": "application/json",
                },
                body: JSON.stringify({
                    connectionId: connectionId,
                    name,
                } satisfies JoinGameRequest),
            });

            if (!response.ok) {
                throw new Error("Failed to join game");
            }

            const result = await response.json();
            console.log("Successfully joined game:", result);

            // Store the game token
            if (result.gameToken) {
                setActiveGame(gameId, result.gameToken);
            }
        } catch (error) {
            console.error("Error joining game:", error);
        }
    };

    const leaveGame = async (gameId: string, gameToken: string) => {
        try {
            const response = await fetch(`${baseUrl}/games/${gameId}/leave`, {
                method: "POST",
                headers: {
                    "Content-Type": "application/json",
                    "Authorization": `Bearer ${gameToken}`,
                },
                body: JSON.stringify({}),
            });

            if (!response.ok) {
                throw new Error("Failed to leave game");
            }

            const result = await response.json();
            console.log("Successfully left game:", result);

            setActiveGame(undefined);
        } catch (error) {
            console.error("Error leaving game:", error);
        }
    };

    const sendSignal = async (gameId: string, targetConnectionId: string, gameToken: string, signalData: string) => {
        try {
            const response = await fetch(`${baseUrl}/games/${gameId}/signal`, {
                method: "POST",
                headers: {
                    "Content-Type": "application/json",
                    "Authorization": `Bearer ${gameToken}`,
                },
                body: JSON.stringify({
                    targetConnectionId,
                    signalData,
                } satisfies SignalRequest),
            });

            if (!response.ok) {
                throw new Error("Failed to send signal");
            }

            const result = await response.json();
            console.log("Signal sent successfully:", result);
        } catch (error) {
            console.error("Error sending signal:", error);
        }
    };

    const updatePlayer = async (gameId: string, gameToken: string, name: string) => {
        try {
            const response = await fetch(`${baseUrl}/games/${gameId}/player`, {
                method: "PUT",
                headers: {
                    "Content-Type": "application/json",
                    "Authorization": `Bearer ${gameToken}`,
                },
                body: JSON.stringify({
                    name,
                } satisfies UpdatePlayerRequest),
            });

            if (!response.ok) {
                throw new Error("Failed to update player");
            }

            const result = await response.json();
            console.log("Player updated successfully:", result);
        } catch (error) {
            console.error("Error updating player:", error);
        }
    };

    const updateGame = async (gameId: string, creatorToken: string, updates: UpdateGameRequest) => {
        try {
            const response = await fetch(`${baseUrl}/games/${gameId}`, {
                method: "PUT",
                headers: {
                    "Content-Type": "application/json",
                    "Authorization": `Bearer ${creatorToken}`,
                },
                body: JSON.stringify(updates),
            });

            if (!response.ok) {
                throw new Error("Failed to update game");
            }

            const result = await response.json();
            console.log("Game updated successfully:", result);
        } catch (error) {
            console.error("Error updating game:", error);
        }
    };

    const startGame = async (gameId: string, creatorToken: string) => {
        try {
            const response = await fetch(`${baseUrl}/games/${gameId}/start`, {
                method: "POST",
                headers: {
                    "Content-Type": "application/json",
                    "Authorization": `Bearer ${creatorToken}`,
                },
                body: JSON.stringify({} satisfies StartGameRequest),
            });

            if (!response.ok) {
                throw new Error("Failed to start game");
            }

            const result = await response.json();
            console.log("Game started successfully:", result);
        } catch (error) {
            console.error("Error starting game:", error);
        }
    };

    const sendChatMessage = async (gameId: string, gameToken: string, message: string) => {
        try {
            const response = await fetch(`${baseUrl}/games/${gameId}/chat`, {
                method: "POST",
                headers: {
                    "Content-Type": "application/json",
                    "Authorization": `Bearer ${gameToken}`,
                },
                body: JSON.stringify({
                    message,
                } satisfies SendChatMessageRequest),
            });

            if (!response.ok) {
                throw new Error("Failed to send chat message");
            }

            const result = await response.json();
            console.log("Chat message sent successfully:", result);
        } catch (error) {
            console.error("Error sending chat message:", error);
        }
    };

    const getChatHistory = async (gameId: string, gameToken: string): Promise<ChatMessage[]> => {
        try {
            const response = await fetch(`${baseUrl}/games/${gameId}/chat-history`, {
                method: "GET",
                headers: {
                    "Authorization": `Bearer ${gameToken}`,
                },
            });

            if (!response.ok) {
                throw new Error("Failed to get chat history");
            }

            const result = await response.json();
            console.log("Chat history retrieved successfully:", result);
            return result.messages;
        } catch (error) {
            console.error("Error getting chat history:", error);
            return [];
        }
    };

    return {
        createGame,
        joinGame,
        leaveGame,
        sendSignal,
        updatePlayer,
        updateGame,
        startGame,
        sendChatMessage,
        getChatHistory,
    };
};

export type Commands = ReturnType<typeof createCommands>;
