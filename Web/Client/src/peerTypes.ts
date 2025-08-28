type ConnectionId = string;

export interface PeerConnectionStatusMessage {
    type: "peerConnectionStatus";
    peers: Record<ConnectionId, { isConnected: boolean }>;
}

export interface PeerPingMessage {
    type: "peerConnectionPing";
    timestamp: number;
    pingType: "ping" | "pong";
}

export type PeerMessage = PeerConnectionStatusMessage | PeerPingMessage;
