type ConnectionId = string;

export interface PeerConnectionStatusMessage {
    type: "peerConnectionStatus";
    peers: Record<ConnectionId, { isConnected: boolean }>;
    isLoadingGameData: boolean;
}

export interface PeerPingMessage {
    type: "peerConnectionPing";
    timestamp: number;
    pingType: "ping" | "pong";
}

export type PeerMessage = PeerConnectionStatusMessage | PeerPingMessage;
