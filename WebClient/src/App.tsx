import { ConnectionStatus } from '@/ConnectionStatus.tsx';
import { GameList } from '@/GameList.tsx';
import { Button } from '@/components/ui/button.tsx';
import { useGameData } from '@/useGameData.ts';
import { useState } from 'react';
import { ActiveGame } from '@/ActiveGame.tsx';

export interface ActiveGame {
  gameId: string;
  token: string;
}

function App() {
  const [activeGame, setActiveGame] = useState<ActiveGame>();

  const { connectionId, games, commands } = useGameData(
    'http://localhost:3001/api',
    setActiveGame
  );

  const handleJoinGame = (gameId: string) => {
    if (connectionId) {
      commands.joinGame(gameId, connectionId);
    }
  };

  const handleLeaveGame = (gameId: string) => {
    commands.leaveGame(gameId, activeGame!.token);
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
                disabled={!connectionId || activeGame !== undefined}
              >
                Create New Game
              </Button>
            </div>
          </div>
        </div>
        {activeGame && <ActiveGame />}
      </div>
      {/* Right sidebar for game list */}
      <div className="w-96 p-8 border-l bg-card/30">
        <GameList
          games={games}
          connectionId={connectionId}
          disabled={activeGame !== undefined}
          onJoinGame={handleJoinGame}
          onLeaveGame={handleLeaveGame}
        />
      </div>
    </div>
  );
}

export default App;
