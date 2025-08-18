import { ConnectionStatus } from '@/ConnectionStatus.tsx';
import { GameList } from '@/GameList.tsx';
import { Button } from '@/components/ui/button.tsx';
import { useGameData } from '@/useGameData.ts';

function App() {
  const { connectionId, games, commands } = useGameData(
    'http://localhost:3001/api'
  );

  return (
    <div className="min-h-screen bg-background p-8">
      <div className="max-w-4xl mx-auto space-y-8">
        <h1 className="text-4xl font-bold text-center">HoverRace Game Lobby</h1>

        <div>
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

        <GameList games={games} />
      </div>
    </div>
  );
}

export default App;
