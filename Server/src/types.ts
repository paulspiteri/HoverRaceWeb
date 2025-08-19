export type Player = {
  connectionId: string;
};

export type ServerGame = {
  id: string;
  name: string;
  players: Player[];
  maxPlayers: number;
  createdAt: Date;
  creatorConnectionId: string;
};

export type CreateGameRequest = {
  name: string;
  maxPlayers: number;
  creatorConnectionId: string;
};

export type JoinGameRequest = {
  connectionId: string;
};

export type AvailableGame = {
  id: string;
  name: string;
  playerCount: number;
  maxPlayers: number;
  createdAt: Date;
  creatorConnectionId: string;
};

export type JoinedGame = ServerGame;

export type ConnectionIdMessage = {
  type: 'connectionId';
  connectionId: string;
};

export type GameListMessage = {
  type: 'gameList';
  games: AvailableGame[];
};

export type GameUpdatedMessage = {
  type: 'gameUpdated';
  game: AvailableGame;
};

export type GameUpdatedFullMessage = {
  type: 'gameUpdatedFull';
  game: ServerGame;
};

export type GameDeletedMessage = {
  type: 'gameDeleted';
  gameId: string;
};

export type ServerMessage = 
  | ConnectionIdMessage
  | GameListMessage
  | GameUpdatedMessage
  | GameUpdatedFullMessage
  | GameDeletedMessage;
