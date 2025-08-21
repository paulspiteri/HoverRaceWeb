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
import { PlayerList } from './PlayerList';


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

    const totalPlayers = game.players.filter((p) => p !== undefined).length;
    if (totalPlayers < 2) return false; // Need at least 2 players total

    // Check if all other players have connected peers
    return game.players.every((player, index) => {
      if (!player || player.connectionId === currentConnectionId) return true; // Skip empty slots and self
      return peerStatuses[index] === 'connected';
    });
  }, [game.players, peerStatuses, currentConnectionId]);
  return (
    <Card className="mt-8">
      <CardHeader>
        <CardTitle>{game.name}</CardTitle>
        <CardDescription>
          Game Lobby • {game.players.filter((p) => p !== undefined).length}/
          {game.maxPlayers} Players
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
              disabled={
                game.players.filter((p) => p !== undefined).length < 2 ||
                !allPeersConnected
              }
            >
              Start Game
            </Button>
          )}
        </div>
      </CardContent>
    </Card>
  );
};
