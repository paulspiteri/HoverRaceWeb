import { ConnectionStatus } from '@/ConnectionStatus.tsx';
import { GameList } from '@/GameList.tsx';
import { Button } from '@/components/ui/button.tsx';
import { useGameData } from '@/useGameData.ts';

function App() {
  const { connectionId, games, gameTokens, commands } = useGameData(
    'http://localhost:3001/api'
  );

  const handleJoinGame = (gameId: string) => {
    if (connectionId) {
      commands.joinGame(gameId, connectionId);
    }
  };

  const handleLeaveGame = (gameId: string) => {
    const gameToken = gameTokens.get(gameId);
    if (gameToken) {
      commands.leaveGame(gameId, gameToken);
    }
  };

  return (
    <div className="min-h-screen bg-background flex">
      {/* Main content area */}
      <div className="flex-1 p-8">
        <div className="max-w-4xl space-y-8">
          <h1 className="text-4xl font-bold text-center">
            HoverRace Game Lobby
          </h1>

          <div className="space-y-6">
            <ConnectionStatus connectionId={connectionId} />
            <div>
              <Button
                onClick={() => commands.createGame(connectionId!)}
                disabled={!connectionId}
              >
                Create New Game
              </Button>
            </div>
          </div>
        </div>
      </div>
      {/* Right sidebar for game list */}
      <div className="w-96 p-8 border-l bg-card/30">
        <GameList
          games={games}
          connectionId={connectionId}
          onJoinGame={handleJoinGame}
          onLeaveGame={handleLeaveGame}
        />
      </div>
    </div>
  );
}

export default App;
