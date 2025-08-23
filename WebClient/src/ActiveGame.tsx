import * as React from 'react';

import { Button, Card, Text, Title, Stack, Group } from '@mantine/core';
import type { JoinedGame } from './types';
import { useMemo } from 'react';
import { PlayerList } from './PlayerList';
import { PlayerNameInput } from './PlayerNameInput';
import type { PeerConnectionStatusMessage } from '@/peerTypes.ts';

interface ActiveGameProps {
  game: JoinedGame;
  onClose: () => void;
  onStartGame: () => void;
  peerStatuses: ('connecting' | 'connected' | 'disconnected' | undefined)[] | undefined;
  currentConnectionId: string | undefined;
  onUpdatePlayer: (name: string) => Promise<void>;
  peersActualStatuses?: (PeerConnectionStatusMessage | undefined)[];
}

export const ActiveGame: React.FC<ActiveGameProps> = ({
  game,
  onClose,
  onStartGame,
  peerStatuses,
  currentConnectionId,
  onUpdatePlayer,
  peersActualStatuses,
}) => {
  // Get current player info and determine if creator (index 0)
  const currentPlayerIndex = game.players.findIndex((p) => p?.connectionId === currentConnectionId);
  const currentPlayer = currentPlayerIndex >= 0 ? game.players[currentPlayerIndex] : undefined;
  const currentPlayerName = currentPlayer?.name || '';
  const isCreator = currentPlayerIndex === 0;

  // Check if all peers are connected (excluding self)
  const allPeersConnected = useMemo(() => {
    if (!peerStatuses || !isCreator) return false;

    const totalPlayers = game.players.filter((p) => p !== undefined).length;
    if (totalPlayers < 2) return false; // Need at least 2 players total

    if (peersActualStatuses) {
      for (let i = 1; i < game.players.length; i++) {
        const player = game.players[i];
        if (!player) continue; // Skip empty slots

        const peerStatus = peersActualStatuses[i];
        if (!peerStatus) return false;

        // Check that this peer reports being connected to all other peers (except self)
        for (let j = 0; j < game.players.length; j++) {
          const gamePlayer = game.players[j];
          if (!gamePlayer || j === i) continue; // Skip empty slots and self

          const peerReport = peerStatus.peers[gamePlayer.connectionId];
          if (!peerReport || !peerReport.isConnected) return false;
        }
      }
    }

    // Also check our own direct peer connections
    return game.players.every((player, index) => {
      if (!player || player.connectionId === currentConnectionId) return true; // Skip empty slots and self
      return peerStatuses[index] === 'connected';
    });
  }, [game.players, peerStatuses, currentConnectionId, isCreator, peersActualStatuses]);
  return (
    <Card withBorder shadow="sm" radius="md" mt="xl">
      <Card.Section withBorder inheritPadding py="xs">
        <Title order={2}>{game.name}</Title>
        <Text size="sm" c="dimmed">
          Game Lobby • {game.players.filter((p) => p !== undefined).length}/{game.maxPlayers} Players
        </Text>
      </Card.Section>

      <Stack gap="lg" mt="md">
        <PlayerNameInput currentPlayerName={currentPlayerName} onUpdatePlayer={onUpdatePlayer} />

        <PlayerList
          gamePlayers={game.players}
          creatorConnectionId={game.creatorConnectionId}
          peerStatuses={peerStatuses}
          currentConnectionId={currentConnectionId}
          peersActualStatuses={peersActualStatuses}
          isHost={isCreator}
        />

        {/* Action Buttons */}
        <Group justify="space-between" pt="md">
          <Button variant="outline" onClick={onClose}>
            {isCreator ? 'Cancel Game' : 'Leave Game'}
          </Button>

          {isCreator && (
            <Button
              onClick={onStartGame}
              disabled={game.players.filter((p) => p !== undefined).length < 2 || !allPeersConnected}
            >
              Start Game
            </Button>
          )}
        </Group>
      </Stack>
    </Card>
  );
};
