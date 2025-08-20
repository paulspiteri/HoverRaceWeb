export type Player = {
    connectionId: string;
    gameToken?: string; // Only present for joined players, not creators
};

export type ClientPlayer = {
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

export type LeaveGameRequest = {
    gameToken: string;
};

export type DeleteGameRequest = {
    creatorToken: string;
};

export type SignalRequest = {
    targetConnectionId: string;
    gameToken: string;
    signalData: string;
};

export type AvailableGame = {
    id: string;
    name: string;
    playerCount: number;
    maxPlayers: number;
    createdAt: Date;
    creatorConnectionId: string;
};

export type JoinedGame = {
    id: string;
    name: string;
    players: ClientPlayer[];
    maxPlayers: number;
    createdAt: Date;
    creatorConnectionId: string;
};

export type ConnectionIdMessage = {
    type: "connectionId";
    connectionId: string;
};

export type GameListMessage = {
    type: "gameList";
    games: AvailableGame[];
};

export type GameUpdatedMessage = {
    type: "gameUpdated";
    game: AvailableGame;
};

export type GameUpdatedFullMessage = {
    type: "gameUpdatedFull";
    game: JoinedGame;
};

export type GameDeletedMessage = {
    type: "gameDeleted";
    gameId: string;
};

export type SignalMessage = {
    type: "signal";
    fromConnectionId: string;
    signalData: string;
};

export type ServerMessage =
    | ConnectionIdMessage
    | GameListMessage
    | GameUpdatedMessage
    | GameUpdatedFullMessage
    | GameDeletedMessage
    | SignalMessage;
