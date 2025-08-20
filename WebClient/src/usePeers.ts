import { useCallback, useEffect, useRef } from 'react';
import type { JoinedGame, ServerMessage, SignalMessage } from '@/types.ts';
import SimplePeer from 'simple-peer';

interface GamePeer {
  peer: SimplePeer.Instance;
  connectionId: string;
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
  const peers = useRef<(GamePeer | undefined)[]>(undefined);

  const onSignalReceived = useCallback((signal: SignalMessage) => {
    console.log(`📡 Received signal from ${signal.fromConnectionId}`);
    const gamePeer = peers.current?.find(
      (x) => x && x.connectionId === signal.fromConnectionId
    );
    if (gamePeer) {
      gamePeer.peer.signal(signal.signalData);
    } else {
      console.warn(
        `❌ No peer found for signal from ${signal.fromConnectionId}`
      );
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
    eventSource.addEventListener('message', onMessage);
    return () => eventSource.removeEventListener('message', onMessage);
  }, [eventSource, onSignalReceived]);

  useEffect(() => {
    if (game) {
      const myPlayerIndex = game.players.findIndex(
        (x) => x.connectionId === connectionId
      );
      if (myPlayerIndex === -1) {
        throw new Error('Current connection not found in game players');
      }

      if (peers.current === undefined) {
        peers.current = new Array<GamePeer>(game.maxPlayers);
        console.log(`📋 Created peers array for ${game.maxPlayers} players`);
      }
      for (let i = 0; i < game.maxPlayers; i++) {
        if (i === myPlayerIndex) {
          continue;
        }
        if (
          peers.current[i] &&
          peers.current[i]!.connectionId !== game.players[i]?.connectionId
        ) {
          console.log(`🔄 Player ${i} changed, destroying old peer`);
          peers.current[i]!.peer.destroy();
          peers.current[i] = undefined!;
        }

        if (!peers.current[i] && game.players[i]) {
          const isInitiator = myPlayerIndex > i;
          console.log(
            `🤝 Creating peer for player ${i} (${game.players[i].connectionId}) - initiator: ${isInitiator}`
          );
          peers.current[i] = {
            connectionId: game.players[i].connectionId,
            peer: new SimplePeer({ initiator: isInitiator, trickle: false }),
          };
          peers.current[i]!.peer.on('signal', (data) => {
            console.log(
              `📤 Sending signal to ${peers.current![i]!.connectionId}`
            );
            if (game && gameToken && peers.current) {
              sendSignal(
                game.id,
                peers.current[i]!.connectionId,
                gameToken,
                JSON.stringify(data)
              );
            }
          });
          peers.current[i]!.peer.on('connect', () => {
            console.log(
              `🔗 Connected to peer ${peers.current![i]!.connectionId}`
            );
          });
          peers.current[i]!.peer.on('error', (err) => {
            console.error(
              `❌ Peer error with ${peers.current![i]!.connectionId}:`,
              err
            );
          });
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
      }
    }
  }, [game, connectionId, sendSignal, gameToken]);

  return {
    onSignalReceived,
  };
};
