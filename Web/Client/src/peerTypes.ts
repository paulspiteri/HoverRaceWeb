type ConnectionId = string;

export interface PeerConnectionStatusMessage {
  type: 'peerConnectionStatus';
  peers: Record<ConnectionId, { isConnected: boolean }>;
}
