import { Text, Box, Group } from "@mantine/core";

export const ConnectionStatus: React.FC<{
    connectionId: string | undefined;
}> = ({ connectionId }) => {
    return (
        <Box
            p="sm"
            mb="md"
            style={{
                border: "1px solid var(--mantine-color-gray-3)",
                borderRadius: 8,
                backgroundColor: "var(--mantine-color-gray-0)",
            }}
        >
            <Group gap="xs" align="center">
                <Box
                    w={8}
                    h={8}
                    bg={connectionId ? "green" : "yellow"}
                    style={{ borderRadius: "50%", flexShrink: 0 }}
                />
                <Text
                    size="xs"
                    c="dimmed"
                    style={{
                        fontFamily: "monospace",
                        wordBreak: "break-all",
                    }}
                >
                    {connectionId ?? "Connecting..."}
                </Text>
            </Group>
        </Box>
    );
};
