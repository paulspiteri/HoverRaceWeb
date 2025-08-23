import { Text, Box, Group } from '@mantine/core';

export const ConnectionStatus: React.FC<{
  connectionId: string | undefined;
}> = ({ connectionId }) => {
  return (
    <Box>
      <Group gap="xs" justify="center">
        <Box 
          w={8} 
          h={8} 
          bg={connectionId ? 'green' : 'yellow'} 
          style={{ borderRadius: '50%' }} 
        />
        <Text size="sm" c="dimmed">
          Connection ID: {connectionId ?? 'Connecting...'}
        </Text>
      </Group>
    </Box>
  );
};
