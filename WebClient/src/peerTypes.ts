export interface PeerConnectionStatusMessage {
  type: 'peerConnectionStatus';
  peers: ({ connectionId: string; isConnected: boolean } | undefined)[];
}
