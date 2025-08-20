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
  eventSource: EventSource
) => {
  const peers = useRef<GamePeer[]>(undefined);

  const onSignalReceived = useCallback((signal: SignalMessage) => {
    const gamePeer = peers.current?.find(
      (x) => x.connectionId === signal.fromConnectionId
    );
    if (gamePeer) {
      // todo add logging
      gamePeer.peer.signal(signal.signalData);
    } else {
      // todo - add logging
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
      }
      for (let i = 0; i < game.maxPlayers; i++) {
        if (i === myPlayerIndex) {
          break;
        }
        if (
          peers.current[i] &&
          peers.current[i].connectionId !== game.players[i]?.connectionId
        ) {
          // peer changed
          peers.current[i].peer.destroy();
          peers.current[i] = undefined!;
        }

        if (!peers.current[i] && game.players[i]) {
          peers.current[i] = {
            connectionId: game.players[i].connectionId,
            peer: new SimplePeer({ initiator: myPlayerIndex > i }),
          };
          peers.current[i].peer.on('signal', (data) => {});
        }
      }
    } else {
      if (peers.current) {
        for (const gamePeer of peers.current) {
          if (gamePeer) {
            gamePeer.peer.destroy();
          }
        }
        peers.current = undefined;
      }
    }
  }, [game, connectionId]);

  return {
    onSignalReceived,
  };
};
