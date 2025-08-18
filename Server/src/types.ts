export type Player = {
  connectionId: string;
};

export type Game = {
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

export type PublicGameData = {
  id: string;
  name: string;
  playerCount: number;
  maxPlayers: number;
  createdAt: Date;
  creatorConnectionId: string;
};
