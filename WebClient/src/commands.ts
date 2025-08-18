import type { CreateGameRequest } from '@/types.ts';

export const createCommands = (baseUrl: string) => {
  const createGame = async (connectionId: string) => {
    try {
      const response = await fetch(`${baseUrl}/games`, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({
          name: `Game ${Date.now()}`,
          maxPlayers: 8,
          creatorConnectionId: connectionId,
        } satisfies CreateGameRequest),
      });

      if (!response.ok) {
        throw new Error('Failed to create game');
      }

      const newGame = await response.json();
      console.log('Game created:', newGame);
    } catch (error) {
      console.error('Error creating game:', error);
    }
  };

  const joinGame = async (gameId: string, connectionId: string) => {
    try {
      const response = await fetch(`${baseUrl}/games/${gameId}/join`, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({
          connectionId: connectionId,
        }),
      });

      if (!response.ok) {
        throw new Error('Failed to join game');
      }

      const result = await response.json();
      console.log('Successfully joined game:', result);
    } catch (error) {
      console.error('Error joining game:', error);
    }
  };

  return {
    createGame,
    joinGame,
  };
};
