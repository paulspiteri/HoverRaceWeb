import { useCallback, useEffect, useState } from 'react';
import type { JoinedGame, ServerMessage, SignalMessage } from '@/types.ts';
import SimplePeer from 'simple-peer';

export interface GamePeer {
  peer: SimplePeer.Instance;
  connectionId: string;
  status: 'connecting' | 'connected' | 'disconnected';
}

export const usePeers = (
  connectionId: string | undefined,
  game: JoinedGame | undefined,
  eventSource: EventSource,
  sendSignal: (
    gameId: string,
    targetConnectionId: string,
    gameToken: string,
    signalData: string
  ) => Promise<void>,
  gameToken: string | undefined
) => {
  const [peers, setPeers] = useState<(GamePeer | undefined)[] | undefined>(undefined);

  const onSignalReceived = useCallback(
    (signal: SignalMessage) => {
      console.log(`📡 Received signal from ${signal.fromConnectionId}`);
      const gamePeer = peers?.find(
        (x) => x && x.connectionId === signal.fromConnectionId
      );
      if (gamePeer) {
        gamePeer.peer.signal(signal.signalData);
      } else {
        console.warn(
          `❌ No peer found for signal from ${signal.fromConnectionId}`
        );
      }
    },
    [peers]
  );

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
    eventSource.addEventListener('message', onMessage);
    return () => eventSource.removeEventListener('message', onMessage);
  }, [eventSource, onSignalReceived]);

  useEffect(() => {
    if (game) {
      console.log(`🎮 Setting up peers for game ${game.id}`);
      const myPlayerIndex = game.players.findIndex(
        (x) => x.connectionId === connectionId
      );
      if (myPlayerIndex === -1) {
        throw new Error('Current connection not found in game players');
      }
      console.log(`👤 My player index: ${myPlayerIndex}`);

      setPeers((prevPeers) => {
        const currentPeers =
          prevPeers || new Array<GamePeer | undefined>(game.maxPlayers);

        for (let i = 0; i < game.maxPlayers; i++) {
          if (i === myPlayerIndex) {
            continue;
          }
          if (
            currentPeers[i] &&
            currentPeers[i]!.connectionId !== game.players[i]?.connectionId
          ) {
            console.log(`🔄 Player ${i} changed, destroying old peer`);
            currentPeers[i]!.peer.destroy();
            currentPeers[i] = undefined;
          }

          if (!currentPeers[i] && game.players[i]) {
            const isInitiator = myPlayerIndex > i;
            const connectionId = game.players[i].connectionId;
            console.log(
              `🤝 Creating peer for player ${i} (${connectionId}) - initiator: ${isInitiator}`
            );

            const newPeer: GamePeer = {
              connectionId,
              peer: new SimplePeer({ initiator: isInitiator, trickle: false }),
              status: 'connecting',
            };
            currentPeers[i] = newPeer;

            newPeer.peer.on('signal', (data: SimplePeer.SignalData) => {
              console.log(`📤 Sending signal to ${connectionId}`);
              if (game && gameToken) {
                sendSignal(
                  game.id,
                  connectionId,
                  gameToken,
                  JSON.stringify(data)
                );
              }
            });

            newPeer.peer.on('connect', () => {
              console.log(`🔗 Connected to peer ${connectionId}`);
              setPeers((prev) => {
                if (!prev) return prev;
                const updated = [...prev];
                if (updated[i]) {
                  updated[i]!.status = 'connected';
                }
                return updated;
              });
            });

            newPeer.peer.on('close', () => {
              console.log(`🔌 Peer disconnected: ${connectionId}`);
              setPeers((prev) => {
                if (!prev) return prev;
                const updated = [...prev];
                if (updated[i]) {
                  updated[i]!.status = 'disconnected';
                }
                return updated;
              });
            });

            newPeer.peer.on('error', (err: Error) => {
              console.error(`❌ Peer error with ${connectionId}:`, err);
              setPeers((prev) => {
                if (!prev) return prev;
                const updated = [...prev];
                if (updated[i]) {
                  updated[i]!.status = 'disconnected';
                }
                return updated;
              });
            });
          }
        }

        return currentPeers;
      });
    } else {
      setPeers((prevPeers) => {
        if (prevPeers) {
          console.log(`🧹 Cleaning up all peers (no active game)`);
          for (const gamePeer of prevPeers) {
            if (gamePeer) {
              console.log(`🗑️ Destroying peer ${gamePeer.connectionId}`);
              gamePeer.peer.destroy();
            }
          }
        }
        return undefined;
      });
    }
  }, [game, connectionId, sendSignal, gameToken]);

  return {
    peers,
  };
};
