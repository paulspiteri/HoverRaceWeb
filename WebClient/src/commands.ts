import type { CreateGameRequest } from '@/types.ts';

export const createCommands = (baseUrl: string, gameTokens: Map<string, string>) => {
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

      const result = await response.json();
      console.log('Game created:', result);
      
      // Store the creator token
      if (result.creatorToken && result.game) {
        gameTokens.set(result.game.id, result.creatorToken);
      }
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
      
      // Store the game token
      if (result.gameToken) {
        gameTokens.set(gameId, result.gameToken);
      }
    } catch (error) {
      console.error('Error joining game:', error);
    }
  };

  const leaveGame = async (gameId: string, gameToken: string) => {
    try {
      const response = await fetch(`${baseUrl}/games/${gameId}/leave`, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({
          gameToken: gameToken,
        }),
      });

      if (!response.ok) {
        throw new Error('Failed to leave game');
      }

      const result = await response.json();
      console.log('Successfully left game:', result);
      
      // Remove the game token since we left
      gameTokens.delete(gameId);
    } catch (error) {
      console.error('Error leaving game:', error);
    }
  };

  return {
    createGame,
    joinGame,
    leaveGame,
  };
};
