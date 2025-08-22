import { useCallback, useEffect, useRef, useState } from 'react';
import type { JoinedGame, ServerMessage, SignalMessage } from '@/types.ts';
import type { PeerConnectionStatusMessage } from '@/peerTypes.ts';
import SimplePeer from 'simple-peer';

export interface GamePeer {
  peer: SimplePeer.Instance;
  connectionId: string;
}

export const usePeers = (
  connectionId: string | undefined,
  game: JoinedGame | undefined,
  eventSource: EventSource | undefined,
  sendSignal: (gameId: string, targetConnectionId: string, gameToken: string, signalData: string) => Promise<void>,
  gameToken: string | undefined
) => {
  const peers = useRef<(GamePeer | undefined)[] | undefined>(undefined);
  const [peerStatuses, setPeerStatuses] = useState<('connecting' | 'connected' | 'disconnected' | undefined)[]>();
  const [peersActualStatuses, setPeersActualStatuses] = useState<PeerConnectionStatusMessage[]>(); // only used at the host

  // Send status update to HOST peer
  const sendStatusUpdateToHost = useCallback(() => {
    if (!game || !peers.current || !peerStatuses) {
      return;
    }

    const hostPeer = peers.current?.find((p) => p?.connectionId === game.creatorConnectionId);
    if (!hostPeer) {
      return;
    }

    const statusMessage: PeerConnectionStatusMessage = {
      type: 'peerConnectionStatus',
      peers: game.players.map((player, index) => {
        if (!player) return undefined;
        return {
          connectionId: player.connectionId,
          isConnected: peerStatuses[index] === 'connected',
        };
      }),
    };

    try {
      if (hostPeer.peer.connected) {
        hostPeer.peer.send(JSON.stringify(statusMessage));
        console.log('📊 Sent status update to host:', statusMessage);
      }
    } catch (error) {
      console.error('❌ Failed to send status update to host:', error);
    }
  }, [game, peerStatuses]);

  const onSignalReceived = useCallback((signal: SignalMessage) => {
    console.log(`📡 Received signal from ${signal.fromConnectionId}`);
    const gamePeer = peers.current?.find((x) => x && x.connectionId === signal.fromConnectionId);
    if (gamePeer) {
      gamePeer.peer.signal(signal.signalData);
    } else {
      console.warn(`❌ No peer found for signal from ${signal.fromConnectionId}`);
    }
  }, []);

  useEffect(() => {
    const onMessage = (event: MessageEvent) => {
      try {
        const data = JSON.parse(event.data) as ServerMessage;
        if (data.type === 'signal') {
          onSignalReceived(data);
        }
      } catch (error) {
        console.error('Error parsing SSE message in usePeers:', error);
      }
    };
    eventSource?.addEventListener('message', onMessage);
    return () => eventSource?.removeEventListener('message', onMessage);
  }, [eventSource, onSignalReceived]);

  useEffect(() => {
    if (game) {
      console.log(`🎮 Setting up peers for game ${game.id}`);
      const myPlayerIndex = game.players.findIndex((x) => x?.connectionId === connectionId);
      if (myPlayerIndex === -1) {
        throw new Error('Current connection not found in game players');
      }
      console.log(`👤 My player index: ${myPlayerIndex}`);

      if (!peers.current) {
        peers.current = new Array<GamePeer | undefined>(game.maxPlayers);
      }

      setPeerStatuses((prev) => {
        if (!prev || prev.length !== game.maxPlayers) {
          return new Array(game.maxPlayers).fill(undefined);
        }
        return prev;
      });

      for (let i = 0; i < game.maxPlayers; i++) {
        if (i === myPlayerIndex) {
          continue;
        }
        const currentPeer = peers.current[i];
        const gamePlayer = game.players[i];

        // Only destroy peer if player actually changed
        if (currentPeer && (!gamePlayer || currentPeer.connectionId !== gamePlayer.connectionId)) {
          console.log(`🔄 Player ${i} changed, destroying old peer`);
          currentPeer.peer.destroy();
          peers.current[i] = undefined;
          setPeerStatuses((prev) => {
            const updated = [...(prev || [])];
            updated[i] = undefined;
            return updated;
          });
        }

        // Only create peer if slot is empty and player exists and it's not us
        if (!peers.current[i] && gamePlayer && gamePlayer.connectionId !== connectionId) {
          const isInitiator = myPlayerIndex > i;
          const playerConnectionId = gamePlayer.connectionId;
          console.log(`🤝 Creating peer for player ${i} (${playerConnectionId}) - initiator: ${isInitiator}`);

          const newPeer: GamePeer = {
            connectionId: playerConnectionId,
            peer: new SimplePeer({ initiator: isInitiator, trickle: false }),
          };
          peers.current[i] = newPeer;

          setPeerStatuses((prev) => {
            const updated = [...(prev || [])];
            updated[i] = 'connecting';
            return updated;
          });

          newPeer.peer.on('signal', (data: SimplePeer.SignalData) => {
            console.log(`📤 Sending signal to ${playerConnectionId}`);
            if (game && gameToken) {
              sendSignal(game.id, playerConnectionId, gameToken, JSON.stringify(data));
            }
          });

          const updatePeerStatus = (status: 'connected' | 'disconnected') => {
            setPeerStatuses((prev) => {
              const updated = [...(prev || [])];
              // Find the current index of this peer (don't rely on closure)
              const currentIndex = peers.current?.findIndex((p) => p?.connectionId === playerConnectionId);
              if (currentIndex !== undefined && currentIndex >= 0) {
                updated[currentIndex] = status;
              }
              return updated;
            });
          };

          newPeer.peer.on('connect', () => {
            console.log(`🔗 Connected to peer ${playerConnectionId}`);
            updatePeerStatus('connected');
          });

          newPeer.peer.on('close', () => {
            console.log(`🔌 Peer disconnected: ${playerConnectionId}`);
            updatePeerStatus('disconnected');
          });

          newPeer.peer.on('error', (err: Error) => {
            console.error(`❌ Peer error with ${playerConnectionId}:`, err);
            updatePeerStatus('disconnected');
          });

          // Only add data event handler if we are the host
          if (connectionId === game.creatorConnectionId) {
            newPeer.peer.on('data', (data: ArrayBuffer) => {
              try {
                const message = JSON.parse(new TextDecoder().decode(data));
                console.log(`📩 Received data from ${playerConnectionId}:`, message);

                // Check if it's a PeerConnectionStatusMessage using type field
                if (message.type === 'peerConnectionStatus') {
                  console.log(`📊 Received status update from ${playerConnectionId}:`, message);
                  setPeersActualStatuses((prev) => {
                    const updated = prev ? [...prev] : new Array(game.maxPlayers).fill(undefined);
                    updated[i] = message as PeerConnectionStatusMessage;
                    return updated;
                  });
                }
              } catch (error) {
                console.error(`❌ Failed to parse data from ${playerConnectionId}:`, error);
              }
            });
          }
        }
      }
    } else {
      if (peers.current) {
        console.log(`🧹 Cleaning up all peers (no active game)`);
        for (const gamePeer of peers.current) {
          if (gamePeer) {
            console.log(`🗑️ Destroying peer ${gamePeer.connectionId}`);
            gamePeer.peer.destroy();
          }
        }
        peers.current = undefined;
        setPeerStatuses(undefined);
      }
    }
  }, [game, connectionId, sendSignal, gameToken]);

  useEffect(() => sendStatusUpdateToHost(), [peerStatuses, sendStatusUpdateToHost]);

  return {
    peerStatuses,
    peersActualStatuses,
  };
};
