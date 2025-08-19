import * as React from 'react';
import { Button } from '@/components/ui/button';
import type { Game } from '@/types.ts';

interface GameTileProps {
  game: Game;
  isJoined: boolean;
  isCreator: boolean;
  disabled?: boolean;
  onJoinGame?: (gameId: string) => void;
  onLeaveGame?: (gameId: string) => void;
}

export const GameTile: React.FC<GameTileProps> = ({
  game,
  isJoined,
  isCreator,
  disabled,
  onJoinGame,
  onLeaveGame,
}) => {
  const playerCount =
    'players' in game ? game.players.length : game.playerCount;
  const isGameFull = playerCount >= game.maxPlayers;
  return (
    <div className="border-2 hover:border-primary/20 transition-colors">
      <div className="p-4 space-y-4">
        <div>
          <h3 className="font-semibold text-lg">{game.name}</h3>
          <p className="text-sm text-muted-foreground">
            Players: {playerCount}/{game.maxPlayers}
          </p>
          <p className="text-xs text-muted-foreground">
            Created: {new Date(game.createdAt).toLocaleString()}
          </p>
          <p className="text-xs text-muted-foreground">Game ID: {game.id}</p>
        </div>

        <Button
          variant="outline"
          size="sm"
          onClick={() => {
            if (isJoined) {
              onLeaveGame?.(game.id);
            } else {
              onJoinGame?.(game.id);
            }
          }}
          disabled={(!isJoined && isGameFull) || disabled}
          className="w-full"
        >
          {isCreator
            ? 'Cancel Game'
            : isJoined
              ? 'Leave Game'
              : isGameFull
                ? 'Game Full'
                : 'Join Game'}
        </Button>
      </div>
    </div>
  );
};
