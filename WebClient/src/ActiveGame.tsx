import * as React from 'react';
import { Button } from '@/components/ui/button';
import {
  Card,
  CardContent,
  CardDescription,
  CardHeader,
  CardTitle,
} from '@/components/ui/card';
import type { JoinedGame } from './types';
import { useMemo } from 'react';

interface PlayerListProps {
  players: JoinedGame['players'];
  creatorConnectionId: string;
  peerStatuses:
    | ('connecting' | 'connected' | 'disconnected' | undefined)[]
    | undefined;
  currentConnectionId: string | undefined;
}

const PlayerList: React.FC<PlayerListProps> = ({
  players,
  creatorConnectionId,
  peerStatuses,
  currentConnectionId,
}) => {
  return (
    <div>
      <h3 className="text-lg font-semibold mb-4">Players</h3>
      <div className="space-y-3">
        {players.map((player, index) => (
          <div
            key={player.connectionId}
            className="flex items-center justify-between p-3 border rounded-lg"
          >
            <div className="flex items-center space-x-3">
              <div className="w-8 h-8 bg-primary/10 rounded-full flex items-center justify-center">
                <span className="text-sm font-medium">{index + 1}</span>
              </div>
              <div>
                <p className="font-medium">
                  Player {index + 1}
                  {player.connectionId === creatorConnectionId && (
                    <span className="ml-2 px-2 py-1 text-xs bg-primary/20 rounded">
                      Host
                    </span>
                  )}
                </p>
                <p className="text-sm text-muted-foreground">
                  ID: {player.connectionId.slice(0, 8)}...
                </p>
              </div>
            </div>
            <div className="flex items-center space-x-2">
              {(() => {
                if (player.connectionId === currentConnectionId) {
                  return (
                    <>
                      <div className="w-2 h-2 bg-blue-500 rounded-full"></div>
                      <span className="text-sm text-muted-foreground">You</span>
                    </>
                  );
                }

                const status = peerStatuses?.[index] || 'disconnected';
                const statusColor =
                  status === 'connected'
                    ? 'bg-green-500'
                    : status === 'connecting'
                      ? 'bg-yellow-500'
                      : 'bg-red-500';
                const statusText =
                  status === 'connected'
                    ? 'Connected'
                    : status === 'connecting'
                      ? 'Connecting'
                      : 'Disconnected';
                return (
                  <>
                    <div
                      className={`w-2 h-2 ${statusColor} rounded-full`}
                    ></div>
                    <span className="text-sm text-muted-foreground">
                      {statusText}
                    </span>
                  </>
                );
              })()}
            </div>
          </div>
        ))}
      </div>
    </div>
  );
};

interface ActiveGameProps {
  game: JoinedGame;
  onClose: () => void;
  onStartGame: () => void;
  isCreator: boolean;
  peerStatuses:
    | ('connecting' | 'connected' | 'disconnected' | undefined)[]
    | undefined;
  currentConnectionId: string | undefined;
}

export const ActiveGame: React.FC<ActiveGameProps> = ({
  game,
  onClose,
  onStartGame,
  isCreator,
  peerStatuses,
  currentConnectionId,
}) => {
  // Check if all peers are connected (excluding self)
  const allPeersConnected = useMemo(() => {
    if (!peerStatuses) return false;

    // Check if all other players have connected peers
    for (let i = 0; i < game.players.length; i++) {
      const player = game.players[i];
      if (player.connectionId === currentConnectionId) continue; // Skip self

      const status = peerStatuses[i];
      if (status !== 'connected') return false;
    }

    return game.players.length > 1; // Need at least 2 players total
  }, [game.players, peerStatuses, currentConnectionId]);
  return (
    <Card className="mt-8">
      <CardHeader>
        <CardTitle>{game.name}</CardTitle>
        <CardDescription>
          Game Lobby • {game.players.length}/{game.maxPlayers} Players
        </CardDescription>
      </CardHeader>
      <CardContent className="space-y-6">
        <PlayerList
          players={game.players}
          creatorConnectionId={game.creatorConnectionId}
          peerStatuses={peerStatuses}
          currentConnectionId={currentConnectionId}
        />

        {/* Action Buttons */}
        <div className="flex justify-between pt-4">
          <Button variant="outline" onClick={onClose}>
            {isCreator ? 'Cancel Game' : 'Leave Game'}
          </Button>

          {isCreator && (
            <Button
              onClick={onStartGame}
              disabled={game.players.length < 2 || !allPeersConnected}
            >
              Start Game
            </Button>
          )}
        </div>
      </CardContent>
    </Card>
  );
};
