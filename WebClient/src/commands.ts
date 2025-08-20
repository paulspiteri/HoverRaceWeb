import type { CreateGameRequest, SignalRequest } from '@/types.ts';
import type { ActiveGame } from '@/App.tsx';

export const createCommands = (
  baseUrl: string,
  setActiveGame: (activeGame: ActiveGame | undefined) => void
) => {
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
        setActiveGame({ gameId: result.game.id, token: result.creatorToken });
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
        setActiveGame({ gameId, token: result.gameToken });
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

      setActiveGame(undefined);
    } catch (error) {
      console.error('Error leaving game:', error);
    }
  };

  const sendSignal = async (
    gameId: string,
    targetConnectionId: string,
    gameToken: string,
    signalData: string
  ) => {
    try {
      const response = await fetch(`${baseUrl}/games/${gameId}/signal`, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({
          targetConnectionId,
          gameToken,
          signalData,
        } satisfies SignalRequest),
      });

      if (!response.ok) {
        throw new Error('Failed to send signal');
      }

      const result = await response.json();
      console.log('Signal sent successfully:', result);
    } catch (error) {
      console.error('Error sending signal:', error);
    }
  };

  return {
    createGame,
    joinGame,
    leaveGame,
    sendSignal,
  };
};
