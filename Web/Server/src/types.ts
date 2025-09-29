export type Player = {
    connectionId: string;
    gameToken: string;
    name?: string;
};

export type ClientPlayer = Omit<Player, "gameToken">;

export type ServerGame = {
    id: string;
    name: string;
    players: (Player | undefined)[];
    maxPlayers: number;
    createdAt: Date;
    status: "waiting" | "playing";
    chatMessages: ChatMessage[];
    trackName: string;
    hasWeapons: boolean;
    laps: number;
};

export type CreateGameRequest = {
    name?: string;
    maxPlayers: number;
    creatorConnectionId: string;
    creatorName?: string;
    trackName?: string;
    hasWeapons?: boolean;
    laps?: number;
};

export type JoinGameRequest = {
    connectionId: string;
    name?: string;
};

export type LeaveGameRequest = {};

export type DeleteGameRequest = {};

export type SignalRequest = {
    targetConnectionId: string;
    signalData: string;
};

export type UpdatePlayerRequest = {
    name: string;
};

export type UpdateGameRequest = {
    trackName?: string;
    hasWeapons?: boolean;
    laps?: number;
};

export type StartGameRequest = {};

export type SendChatMessageRequest = {
    message: string;
    // gameToken is now passed via Authorization: Bearer header
};

export type ChatMessage = {
    id: string;
    message: string;
    senderId: string;
    senderName?: string;
    timestamp: Date;
    gameId: string;
};

export type AvailableGame = {
    id: string;
    name: string;
    playerCount: number;
    maxPlayers: number;
    createdAt: Date;
    status: "waiting" | "playing";
    trackName: string;
    hasWeapons: boolean;
    laps: number;
};

export type JoinedGame = AvailableGame & {
    players: (ClientPlayer | undefined)[];
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

export type ChatMessageServerMessage = {
    type: "chatMessage";
    chatMessage: ChatMessage;
};

export type ChatHistoryMessage = {
    type: "chatHistory";
    messages: ChatMessage[];
};

export type ServerMessage =
    | ConnectionIdMessage
    | GameListMessage
    | GameUpdatedMessage
    | GameUpdatedFullMessage
    | GameDeletedMessage
    | SignalMessage
    | ChatMessageServerMessage
    | ChatHistoryMessage;
