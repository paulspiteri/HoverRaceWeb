import * as React from 'react';
import type { JoinedGame } from './types';
import type { PeerConnectionStatusMessage } from '@/peerTypes.ts';
import { useMemo } from 'react';
import { Title, Stack, Group, Text, Badge, Box, Avatar } from '@mantine/core';

interface PlayerListProps {
  gamePlayers: JoinedGame['players'];
  creatorConnectionId: string;
  peerStatuses: ('connecting' | 'connected' | 'disconnected' | undefined)[] | undefined;
  currentConnectionId: string | undefined;
  peersActualStatuses?: (PeerConnectionStatusMessage | undefined)[];
  isHost?: boolean;
}

interface ConnectionStatusProps {
  player: { connectionId: string };
  index: number;
  peerStatuses: PlayerListProps['peerStatuses'];
  currentConnectionId: string | undefined;
}

const ConnectionStatus: React.FC<ConnectionStatusProps> = ({ player, index, peerStatuses, currentConnectionId }) => {
  if (player.connectionId === currentConnectionId) {
    return (
      <Group gap="xs">
        <Box w={8} h={8} bg="blue" style={{ borderRadius: '50%' }} />
        <Text size="sm" c="dimmed">You</Text>
      </Group>
    );
  }

  const status = peerStatuses?.[index] || 'disconnected';
  const statusColor = status === 'connected' ? 'green' : status === 'connecting' ? 'yellow' : 'red';
  const statusText = status === 'connected' ? 'Connected' : status === 'connecting' ? 'Connecting' : 'Disconnected';

  return (
    <Group gap="xs">
      <Box w={8} h={8} bg={statusColor} style={{ borderRadius: '50%' }} />
      <Text size="sm" c="dimmed">{statusText}</Text>
    </Group>
  );
};

interface MeshStatusProps {
  index: number;
  gamePlayers: JoinedGame['players'];
  peerStatus: PeerConnectionStatusMessage | undefined;
}

const MeshStatus: React.FC<MeshStatusProps> = ({ index, gamePlayers, peerStatus }) => {
  // Check if this peer is fully connected to all other peers
  const isFullyConnected = useMemo(() => {
    if (!peerStatus) return false;
    
    return gamePlayers.every((gamePlayer, j) => {
      if (!gamePlayer || j === index) return true; // Skip empty slots and self
      const peerReport = peerStatus.peers[gamePlayer.connectionId];
      return peerReport && peerReport.isConnected;
    });
  }, [gamePlayers, index, peerStatus]);

  const statusColor = !peerStatus 
    ? 'gray' 
    : isFullyConnected 
      ? 'green' 
      : 'red';
      
  const statusText = !peerStatus 
    ? 'No Report' 
    : isFullyConnected 
      ? 'Mesh Ready' 
      : 'Mesh Not Ready';

  return (
    <Group gap="xs">
      <Box w={8} h={8} bg={statusColor} style={{ borderRadius: '50%' }} />
      <Text size="sm" c="dimmed">{statusText}</Text>
    </Group>
  );
};

export const PlayerList: React.FC<PlayerListProps> = ({
  gamePlayers,
  creatorConnectionId,
  peerStatuses,
  currentConnectionId,
  peersActualStatuses,
  isHost = false,
}) => {
  return (
    <Box>
      <Title order={3} size="lg" mb="md">Players</Title>
      <Stack gap="sm">
        {gamePlayers.map((player, index) => {
          if (!player) {
            return (
              <Box
                key={`empty-${index}`}
                p="md"
                style={{ border: '1px solid var(--mantine-color-gray-3)', borderRadius: 8, opacity: 0.5 }}
              >
                <Group justify="space-between">
                  <Group gap="md">
                    <Avatar size="sm" bg="gray.1" c="gray.6">
                      {index + 1}
                    </Avatar>
                    <Text fw={500} c="dimmed">Empty Slot</Text>
                  </Group>
                  <Group gap="xs">
                    <Box w={8} h={8} bg="gray" style={{ borderRadius: '50%' }} />
                    <Text size="sm" c="dimmed">Empty</Text>
                  </Group>
                </Group>
              </Box>
            );
          }

          return (
            <Box
              key={player.connectionId}
              p="md"
              style={{ border: '1px solid var(--mantine-color-gray-3)', borderRadius: 8 }}
            >
              <Group justify="space-between">
                <Group gap="md">
                  <Avatar size="sm">
                    {index + 1}
                  </Avatar>
                  <Box>
                    <Group gap="xs">
                      <Text fw={500}>
                        {player.name || `Player ${index + 1}`}
                      </Text>
                      {player.connectionId === creatorConnectionId && (
                        <Badge size="xs" variant="light">Host</Badge>
                      )}
                    </Group>
                    <Text size="sm" c="dimmed">ID: {player.connectionId}</Text>
                  </Box>
                </Group>
                <Stack gap="xs" align="flex-end">
                  <ConnectionStatus
                    player={player}
                    index={index}
                    peerStatuses={peerStatuses}
                    currentConnectionId={currentConnectionId}
                  />

                  {/* Overall connection status (host only) */}
                  {isHost && player.connectionId !== currentConnectionId && (
                    <MeshStatus index={index} gamePlayers={gamePlayers} peerStatus={peersActualStatuses?.[index]} />
                  )}
                </Stack>
              </Group>
            </Box>
          );
        })}
      </Stack>
    </Box>
  );
};
