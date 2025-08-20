import { useEffect, useMemo, useState } from 'react';
import type { Game, ServerMessage } from '@/types.ts';
import { createCommands } from '@/commands.ts';
import type { ActiveGame } from '@/App.tsx';

export const useGameData = (
  baseUrl: string,
  setActiveGame: (activeGame: ActiveGame | undefined) => void
) => {
  const url = `${baseUrl}/games/stream`;
  const [eventSource] = useState<EventSource>(() => new EventSource(url));
  const [connectionId, setConnectionId] = useState<string>();
  const [games, setGames] = useState<Game[]>([]);

  useEffect(() => {
    eventSource.onopen = () => {
      console.log('SSE connection opened to games stream');
    };

    eventSource.onmessage = (event) => {
      try {
        const data = JSON.parse(event.data) as ServerMessage;
        console.log('Received SSE message:', data);

        switch (data.type) {
          case 'connectionId':
            setConnectionId(data.connectionId);
            console.log('Connection ID received:', data.connectionId);
            break;
          case 'gameList':
            setGames(data.games);
            console.log('Games list updated:', data.games);
            break;

          case 'gameDeleted':
            setGames((prev) => prev.filter((game) => game.id !== data.gameId));
            console.log('Game deleted:', data.gameId);
            break;

          case 'gameUpdated':
          case 'gameUpdatedFull':
            setGames((prev) => {
              const existingIndex = prev.findIndex(
                (g) => g.id === data.game.id
              );
              if (existingIndex !== -1) {
                // Update existing game at same position
                const newGames = [...prev];
                newGames[existingIndex] = data.game;
                return newGames;
              } else {
                // New game - add to head of array
                return [data.game, ...prev];
              }
            });
            console.log('Game updated:', data.game);
            break;

          case 'signal':
            break;

          default:
            console.log('Unknown message type:', (data as ServerMessage).type);
        }
      } catch (error) {
        console.error('Error parsing SSE message:', error);
      }
    };

    eventSource.onerror = (error) => {
      console.error('SSE connection error:', error);
    };
  }, [eventSource]);

  const commands = useMemo(
    () => createCommands(baseUrl, setActiveGame),
    [baseUrl, setActiveGame]
  );

  return {
    connectionId,
    games,
    commands,
    eventSource,
  };
};
