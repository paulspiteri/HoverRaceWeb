import type { ServerGame, CreateGameRequest } from "./types";
import { EventEmitter } from "events";
import { v4 as uuidv4 } from "uuid";

export class GameManager extends EventEmitter {
    private games: Map<string, ServerGame> = new Map();
    private nextId = 1;

    createGame(gameData: CreateGameRequest): {
        game: ServerGame;
        creatorToken: string;
    } {
        const creatorToken = uuidv4();
        const players = new Array(gameData.maxPlayers).fill(undefined);
        players[0] = {
            connectionId: gameData.creatorConnectionId,
            gameToken: creatorToken,
            name: gameData.creatorName,
        };

        const game: ServerGame = {
            id: this.generateId(),
            name: gameData.name,
            players,
            maxPlayers: gameData.maxPlayers,
            createdAt: new Date(),
            status: "waiting",
        };

        this.games.set(game.id, game);
        this.emit("gameCreated", game);
        return { game, creatorToken };
    }

    deleteGame(id: string): boolean {
        const game = this.games.get(id);
        if (game) {
            this.games.delete(id);
            this.emit("gameDeleted", id);
            return true;
        }
        return false;
    }

    deleteGameByToken(gameId: string, gameToken: string): boolean {
        const game = this.games.get(gameId);
        if (!game || !gameToken) return false;

        // Find the creator by checking if they have the token
        const creatorPlayer = game.players[0];
        if (creatorPlayer?.gameToken !== gameToken) return false;

        this.games.delete(gameId);
        this.emit("gameDeleted", gameId);
        return true;
    }

    getGame(id: string): ServerGame | undefined {
        return this.games.get(id);
    }

    getAllGames(): ServerGame[] {
        return Array.from(this.games.values());
    }

    joinGame(gameId: string, connectionId: string, name?: string): string | undefined {
        const game = this.games.get(gameId);
        if (!game) return undefined;

        if (game.status !== "waiting") return undefined;

        if (game.players.some((p) => p?.connectionId === connectionId)) return undefined;

        const emptySlotIndex = game.players.findIndex((p) => p === undefined);
        if (emptySlotIndex === -1 || emptySlotIndex === 0) return undefined;

        const gameToken = uuidv4();
        game.players[emptySlotIndex] = { connectionId, gameToken, name };
        this.emit("gameUpdated", game);
        return gameToken;
    }

    leaveGame(gameId: string, gameToken: string): boolean {
        const game = this.games.get(gameId);
        if (!game) return false;

        const playerIndex = game.players.findIndex((p) => p?.gameToken === gameToken);
        if (playerIndex === -1) return false;

        // If this is the creator leaving, only delete the game if it hasn't started yet
        if (playerIndex === 0 && game.status === "waiting") {
            this.deleteGame(gameId);
        } else {
            game.players[playerIndex] = undefined;
            this.emit("gameUpdated", game);
        }
        return true;
    }

    updatePlayer(gameId: string, gameToken: string, name: string): boolean {
        const game = this.games.get(gameId);
        if (!game) return false;

        const player = game.players.find((p) => p?.gameToken === gameToken);
        if (!player) return false;

        player.name = name;
        this.emit("gameUpdated", game);
        return true;
    }

    startGame(gameId: string, creatorToken: string): boolean {
        const game = this.games.get(gameId);
        if (!game || !creatorToken) return false;

        if (game.players[0]?.gameToken !== creatorToken) return false;

        // Can only start games that are waiting
        if (game.status !== "waiting") return false;

        game.status = "playing";
        this.emit("gameUpdated", game);
        return true;
    }

    private generateId(): string {
        return (this.nextId++).toString();
    }
}

export const gameManager = new GameManager();
