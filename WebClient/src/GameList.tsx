import type { PublicGameData } from './types';
import * as React from 'react';
import { GameTile } from './GameTile';

interface GameListProps {
  games: PublicGameData[];
  onJoinGame?: (gameId: string) => void;
}

export const GameList: React.FC<GameListProps> = ({ games, onJoinGame }) => {
  return (
    <div>
      <div>
        <div>Available Games ({games.length})</div>
      </div>
      <div>
        {games.length === 0 ? (
          <p className="text-muted-foreground">No games available</p>
        ) : (
          <div className="grid gap-4">
            {games.map((game) => (
              <GameTile 
                key={game.id} 
                game={game} 
                onJoinGame={onJoinGame}
              />
            ))}
          </div>
        )}
      </div>
    </div>
  );
};
