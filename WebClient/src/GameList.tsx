import * as React from 'react';
import { GameTile } from './GameTile';
import type { Game } from './types';

interface GameListProps {
  games: Game[];
  connectionId?: string;
  disabled?: boolean;
  onJoinGame?: (gameId: string) => void;
  onLeaveGame?: (gameId: string) => void;
}

export const GameList: React.FC<GameListProps> = ({
  games,
  connectionId,
  disabled,
  onJoinGame,
  onLeaveGame,
}) => {
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
                isJoined={'players' in game}
                isCreator={connectionId === game.creatorConnectionId}
                disabled={disabled}
                onJoinGame={onJoinGame}
                onLeaveGame={onLeaveGame}
              />
            ))}
          </div>
        )}
      </div>
    </div>
  );
};
