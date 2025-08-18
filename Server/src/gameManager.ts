import type { Game, CreateGameRequest } from './types';
import { EventEmitter } from 'events';

export class GameManager extends EventEmitter {
  private games: Map<string, Game> = new Map();
  private nextId = 1;

  createGame(gameData: CreateGameRequest): Game {
    const game: Game = {
      id: this.generateId(),
      name: gameData.name,
      players: [{ connectionId: gameData.creatorConnectionId }],
      maxPlayers: gameData.maxPlayers,
      createdAt: new Date(),
      creatorConnectionId: gameData.creatorConnectionId,
    };

    this.games.set(game.id, game);
    this.emit('gameCreated', game);
    return game;
  }

  deleteGame(id: string): boolean {
    const game = this.games.get(id);
    if (game) {
      this.games.delete(id);
      this.emit('gameDeleted', id);
      return true;
    }
    return false;
  }

  getGame(id: string): Game | undefined {
    return this.games.get(id);
  }

  getAllGames(): Game[] {
    return Array.from(this.games.values());
  }

  joinGame(gameId: string, connectionId: string): boolean {
    const game = this.games.get(gameId);
    if (!game) return false;

    if (game.creatorConnectionId === connectionId) return false;

    if (game.players.length >= game.maxPlayers) return false;

    if (game.players.some((p) => p.connectionId === connectionId)) return false;

    game.players.push({ connectionId });
    this.emit('gameUpdated', game);
    return true;
  }

  leaveGame(gameId: string, connectionId: string): boolean {
    const game = this.games.get(gameId);
    if (!game) return false;

    if (game.creatorConnectionId === connectionId) {
      this.deleteGame(gameId);
      return true;
    }

    const playerIndex = game.players.findIndex((p) => p.connectionId === connectionId);
    if (playerIndex === -1) return false;

    game.players.splice(playerIndex, 1);
    this.emit('gameUpdated', game);
    return true;
  }

  private generateId(): string {
    return (this.nextId++).toString();
  }
}

export const gameManager = new GameManager();
