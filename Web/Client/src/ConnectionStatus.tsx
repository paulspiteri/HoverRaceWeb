import { Text, Box, Group } from "@mantine/core";
import { useAtomValue } from 'jotai';
import { connectionIdAtom } from '@/atoms.ts';
import styles from './ConnectionStatus.module.css';

interface ConnectionStatusProps {
    className?: string;
}

export const ConnectionStatus: React.FC<ConnectionStatusProps> = ({ className }) => {
    const connectionId = useAtomValue(connectionIdAtom);
    return (
        <Box className={`${styles.container} ${className || ""}`}>
            <Group gap="xs" align="center">
                <Box
                    bg={connectionId ? "green" : "yellow"}
                    className={styles.statusIcon}
                    title={connectionId ? "Connected" : "Connecting..."}
                />
                <Text
                    size="xs"
                    c="dimmed"
                    className={styles.statusText}
                >
                    {connectionId ?? "Connecting..."}
                </Text>
            </Group>
        </Box>
    );
};
