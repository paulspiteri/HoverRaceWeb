import * as React from 'react';
import { Button, Box, Text, Title, Stack } from '@mantine/core';
import type { Game } from '@/types.ts';

interface GameTileProps {
  game: Game;
  isJoined: boolean;
  disabled?: boolean;
  onJoinGame?: (gameId: string) => void;
  onLeaveGame?: (gameId: string) => void;
}

export const GameTile: React.FC<GameTileProps> = ({ game, isJoined, disabled, onJoinGame, onLeaveGame }) => {
  const isGameFull = game.playerCount >= game.maxPlayers;
  const isGameStarted = game.status === 'playing';
  return (
    <Box
      p="md"
      style={{ 
        border: '2px solid var(--mantine-color-gray-3)', 
        borderRadius: 8,
        transition: 'border-color 0.2s ease',
        cursor: 'pointer'
      }}
      __vars={{
        '--box-border-color': 'var(--mantine-color-gray-3)',
      }}
      onMouseEnter={(e) => {
        e.currentTarget.style.borderColor = 'var(--mantine-color-blue-3)';
      }}
      onMouseLeave={(e) => {
        e.currentTarget.style.borderColor = 'var(--mantine-color-gray-3)';
      }}
    >
      <Stack gap="md">
        <Box>
          <Title order={4} size="lg" fw={600}>{game.name}</Title>
          <Text size="sm" c="dimmed">
            Players: {game.playerCount}/{game.maxPlayers}
          </Text>
          <Text size="xs" c="dimmed">Created: {new Date(game.createdAt).toLocaleString()}</Text>
          <Text size="xs" c="dimmed">Game ID: {game.id}</Text>
        </Box>

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
          disabled={(!isJoined && (isGameFull || isGameStarted)) || disabled}
          fullWidth
        >
          {isJoined 
            ? 'Leave Game' 
            : isGameStarted 
              ? 'Game Started'
              : isGameFull 
                ? 'Game Full' 
                : 'Join Game'
          }
        </Button>
      </Stack>
    </Box>
  );
};
