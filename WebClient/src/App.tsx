import { ConnectionStatus } from '@/ConnectionStatus.tsx';
import { GameList } from '@/GameList.tsx';
import { Button } from '@/components/ui/button.tsx';
import { useGameData } from '@/useGameData.ts';
import { useState } from 'react';
import { ActiveGame } from '@/ActiveGame.tsx';
import type { JoinedGame } from './types';
import { usePeers } from '@/usePeers.ts';

export interface ActiveGame {
  gameId: string;
  token: string;
}

function App() {
  const [activeGame, setActiveGame] = useState<ActiveGame>();

  const { connectionId, games, commands, eventSource } = useGameData(
    'http://localhost:3001/api',
    setActiveGame
  );

  const currentGame =
    activeGame &&
    (games.find((g) => g.id === activeGame.gameId && 'players' in g) as
      | JoinedGame
      | undefined);

  const { peerStatuses } = usePeers(
    connectionId,
    currentGame,
    eventSource,
    commands.sendSignal,
    activeGame?.token
  );

  const handleJoinGame = (gameId: string) => {
    if (connectionId) {
      const savedName = localStorage.getItem('hoverrace-player-name');
      commands.joinGame(gameId, connectionId, savedName || undefined);
    }
  };

  const handleLeaveGame = () => {
    if (activeGame) {
      commands.leaveGame(activeGame.gameId, activeGame.token);
    }
  };

  const handleStartGame = () => {
    console.log('Start game clicked');
  };

  const handleUpdatePlayer = async (name: string) => {
    if (activeGame) {
      await commands.updatePlayer(activeGame.gameId, activeGame.token, name);
    }
  };

  return (
    <div className="min-h-screen bg-background flex">
      {/* Main content area */}
      <div className="flex-1 p-8 flex flex-col items-center">
        <div className="max-w-4xl w-full space-y-8">
          {/* Enhanced Header */}
          <div className="text-center">
            <h1 className="text-4xl font-bold bg-gradient-to-r from-blue-600 to-purple-600 bg-clip-text text-transparent">
              HoverRace
            </h1>
          </div>

          {!activeGame && (
            <div className="space-y-6">
              <ConnectionStatus connectionId={connectionId} />
              <div className="flex justify-center">
                <Button
                  onClick={() => {
                    const savedName = localStorage.getItem(
                      'hoverrace-player-name'
                    );
                    commands.createGame(connectionId!, savedName || undefined);
                  }}
                  disabled={!connectionId}
                  size="lg"
                  className="px-8"
                >
                  Create New Game
                </Button>
              </div>
            </div>
          )}
          {activeGame && currentGame && (
            <ActiveGame
              game={currentGame}
              onClose={handleLeaveGame}
              onStartGame={handleStartGame}
              peerStatuses={peerStatuses}
              currentConnectionId={connectionId}
              onUpdatePlayer={handleUpdatePlayer}
            />
          )}
        </div>
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
