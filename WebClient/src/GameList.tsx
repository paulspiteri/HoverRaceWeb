import type { PublicGameData } from './types';

export const GameList: React.FC<{ games: PublicGameData[] }> = ({ games }) => {
  return (
    <div>
      <div>
        <div>Available Games ({games.length})</div>
        <div>Join an existing game or create a new one</div>
      </div>
      <div>
        {games.length === 0 ? (
          <p className="text-muted-foreground">No games available</p>
        ) : (
          <div className="grid gap-4">
            {games.map((game) => (
              <div
                key={game.id}
                className="border-2 hover:border-primary/20 transition-colors"
              >
                <div className="p-4">
                  <div className="flex justify-between items-start">
                    <div>
                      <h3 className="font-semibold text-lg">{game.name}</h3>
                      <p className="text-sm text-muted-foreground">
                        Players: {game.playerCount}/{game.maxPlayers}
                      </p>
                      <p className="text-xs text-muted-foreground">
                        Created: {new Date(game.createdAt).toLocaleString()}
                      </p>
                      <p className="text-xs text-muted-foreground">
                        Game ID: {game.id}
                      </p>
                    </div>
                    <button>Join Game</button>
                  </div>
                </div>
              </div>
            ))}
          </div>
        )}
      </div>
    </div>
  );
};
