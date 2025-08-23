import * as React from 'react';
import { GameTile } from './GameTile';
import type { Game } from './types';
import { Title, Stack, Text, Box } from '@mantine/core';

interface GameListProps {
  games: Game[];
  disabled?: boolean;
  onJoinGame?: (gameId: string) => void;
  onLeaveGame?: (gameId: string) => void;
}

export const GameList: React.FC<GameListProps> = ({ games, disabled, onJoinGame, onLeaveGame }) => {
  return (
    <Box>
      <Title order={3} mb="md">Available Games ({games.length})</Title>
      <Stack gap="md">
        {games.length === 0 ? (
          <Text c="dimmed" ta="center" py="xl">No games available</Text>
        ) : (
          games.map((game) => (
            <GameTile
              key={game.id}
              game={game}
              isJoined={'players' in game}
              disabled={disabled}
              onJoinGame={onJoinGame}
              onLeaveGame={onLeaveGame}
            />
          ))
        )}
      </Stack>
    </Box>
  );
};
