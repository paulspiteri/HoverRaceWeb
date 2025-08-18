import { useState, useEffect } from 'react';
import type { PublicGameData, CreateGameRequest } from '../../Server/src/types';

function App() {
  const [games, setGames] = useState<PublicGameData[]>([]);
  const [connectionId, setConnectionId] = useState<string | null>(null);
  const [isCreating, setIsCreating] = useState(false);

  useEffect(() => {
    const eventSource = new EventSource(
      'http://localhost:3001/api/games/stream'
    );

    eventSource.onopen = () => {
      console.log('SSE connection opened to games stream');
    };

    eventSource.onmessage = (event) => {
      try {
        const data = JSON.parse(event.data);
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
          case 'gameCreated':
            setGames((prev) => [...prev, data.game]);
            console.log('Game created:', data.game);
            break;
          case 'gameDeleted':
            setGames((prev) => prev.filter((game) => game.id !== data.gameId));
            console.log('Game deleted:', data.gameId);
            break;
          case 'gameUpdated':
            setGames((prev) =>
              prev.map((game) => (game.id === data.game.id ? data.game : game))
            );
            console.log('Game updated:', data.game);
            break;
          default:
            console.log('Unknown message type:', data.type);
        }
      } catch (error) {
        console.error('Error parsing SSE message:', error);
      }
    };

    eventSource.onerror = (error) => {
      console.error('SSE connection error:', error);
    };

    return () => {
      eventSource.close();
      console.log('SSE connection closed');
    };
  }, []);

  const createGame = async () => {
    if (!connectionId) {
      console.error('No connection ID available');
      return;
    }

    setIsCreating(true);
    try {
      const response = await fetch('http://localhost:3001/api/games', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({
          name: `Game ${Date.now()}`,
          maxPlayers: 4,
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
    } finally {
      setIsCreating(false);
    }
  };

  return (
    <div className="min-h-screen bg-background p-8">
      <div className="max-w-4xl mx-auto space-y-8">
        <h1 className="text-4xl font-bold text-center">HoverRace Game Lobby</h1>

        <div>
          <div>
            <div>Connection Status</div>
            <div>Connection ID: {connectionId || 'Connecting...'}</div>
          </div>
          <div>
            <button
              onClick={createGame}
              disabled={!connectionId || isCreating}
              className="w-full sm:w-auto"
            >
              {isCreating ? 'Creating...' : 'Start New Game'}
            </button>
          </div>
        </div>

        <div>
          <div>
            <div>Available Games ({games.length})</div>
            <div>Join an existing game or create a new one</div>
          </div>
          <div>
            {games.length === 0 ? (
              <p className="text-muted-foreground">No games available</p>
            ) : (
              <div className="grid gap-4">
                {games.map((game) => (
                  <div
                    key={game.id}
                    className="border-2 hover:border-primary/20 transition-colors"
                  >
                    <div className="p-4">
                      <div className="flex justify-between items-start">
                        <div>
                          <h3 className="font-semibold text-lg">{game.name}</h3>
                          <p className="text-sm text-muted-foreground">
                            Players: {game.playerCount}/{game.maxPlayers}
                          </p>
                          <p className="text-xs text-muted-foreground">
                            Created: {new Date(game.createdAt).toLocaleString()}
                          </p>
                          <p className="text-xs text-muted-foreground">
                            Game ID: {game.id}
                          </p>
                        </div>
                        <button>Join Game</button>
                      </div>
                    </div>
                  </div>
                ))}
              </div>
            )}
          </div>
        </div>
      </div>
    </div>
  );
}

export default App;
