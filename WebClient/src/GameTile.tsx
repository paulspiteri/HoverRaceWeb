import type { PublicGameData } from './types';
import * as React from 'react';
import { Button } from '@/components/ui/button';

interface GameTileProps {
  game: PublicGameData;
  onJoinGame?: (gameId: string) => void;
}

export const GameTile: React.FC<GameTileProps> = ({ game, onJoinGame }) => {
  return (
    <div className="border-2 hover:border-primary/20 transition-colors">
      <div className="p-4 space-y-4">
        <div>
          <h3 className="font-semibold text-lg">{game.name}</h3>
          <p className="text-sm text-muted-foreground">
            Players: {game.playerCount}/{game.maxPlayers}
          </p>
          <p className="text-xs text-muted-foreground">
            Created: {new Date(game.createdAt).toLocaleString()}
          </p>
          <p className="text-xs text-muted-foreground">Game ID: {game.id}</p>
        </div>
        
        <Button
          variant="outline"
          size="sm"
          onClick={() => onJoinGame?.(game.id)}
          className="w-full"
        >
          Join Game
        </Button>
      </div>
    </div>
  );
};
