import * as React from "react";
import type { JoinedGame } from "./types";
import type { PeerConnectionStatusMessage } from "@/peerTypes.ts";
import type { PeerConnectionLatency } from "@/usePeers.ts";
import { useMemo } from "react";
import { Title, Stack, Group, Text, Badge, Box, Avatar, Flex } from "@mantine/core";
import { ElectroCar } from "@/ElectroCar.tsx";

interface ConnectionStatusProps {
    player: { connectionId: string };
    index: number;
    peerStatuses: PlayerListProps["peerStatuses"];
    currentConnectionId: string | undefined;
    peerLatencies?: (PeerConnectionLatency | undefined)[];
}

const ConnectionStatus: React.FC<ConnectionStatusProps> = ({
    player,
    index,
    peerStatuses,
    currentConnectionId,
    peerLatencies,
}) => {
    if (player.connectionId === currentConnectionId) {
        return (
            <Group gap="xs" wrap="nowrap">
                <Box w={8} h={8} bg="blue" style={{ borderRadius: "50%" }} />
                <Text size="sm" c="dimmed">
                    You
                </Text>
            </Group>
        );
    }

    const status = peerStatuses?.[index] || "disconnected";
    const statusColor = status === "connected" ? "green" : status === "connecting" ? "yellow" : "red";
    const statusText = status === "connected" ? "Connected" : status === "connecting" ? "Connecting" : "Disconnected";
    const latency = peerLatencies?.[index]?.averageLatency;

    return (
        <Group gap="xs" wrap="nowrap">
            <Box w={8} h={8} bg={statusColor} style={{ borderRadius: "50%" }} />
            <Text size="sm" c="dimmed" style={{ whiteSpace: "nowrap" }}>
                {statusText}
                {status === "connected" && latency && ` • ${latency}ms`}
            </Text>
        </Group>
    );
};

interface MeshStatusProps {
    index: number;
    gamePlayers: JoinedGame["players"];
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

    const statusColor = !peerStatus ? "gray" : isFullyConnected ? "green" : "red";

    const statusText = !peerStatus ? "No Report" : isFullyConnected ? "Mesh Ready" : "Mesh Not Ready";

    return (
        <Group gap="xs" wrap="nowrap">
            <Box w={8} h={8} bg={statusColor} style={{ borderRadius: "50%" }} />
            <Text size="sm" c="dimmed" style={{ whiteSpace: "nowrap" }}>
                {statusText}
            </Text>
        </Group>
    );
};

interface PlayerListProps {
    gamePlayers: JoinedGame["players"];
    peerStatuses: ("connecting" | "connected" | "disconnected" | undefined)[] | undefined;
    currentConnectionId: string | undefined;
    peersActualStatuses?: (PeerConnectionStatusMessage | undefined)[];
    peerLatencies?: (PeerConnectionLatency | undefined)[];
    isHost?: boolean;
    isLoadingGameData: boolean;
}

export const PlayerList: React.FC<PlayerListProps> = ({
    gamePlayers,
    peerStatuses,
    currentConnectionId,
    peersActualStatuses,
    peerLatencies,
    isHost = false,
    isLoadingGameData,
}) => {
    return (
        <Box
            h="100%"
            p="sm"
            style={{
                border: "1px solid var(--mantine-color-gray-3)",
                borderRadius: "var(--mantine-radius-md)",
                minHeight: 0,
            }}
        >
            <Flex h="100%" direction="column" style={{ minHeight: 0 }}>
                <Title order={3} size="lg" mb="md">
                    Players
                </Title>
                <Box flex={1} style={{ overflowY: "auto", minHeight: 0 }}>
                    <Stack gap="sm">
                    {gamePlayers.map((player, index) => {
                        if (!player) {
                            return (
                                <Box
                                    key={`empty-${index}`}
                                    p="md"
                                    style={{
                                        border: "1px solid var(--mantine-color-gray-3)",
                                        borderRadius: 8,
                                        opacity: 0.5,
                                    }}
                                >
                                    <Group justify="space-between" wrap="nowrap">
                                        <Group gap="md">
                                            <Avatar size="sm" bg="gray.1" c="gray.6" style={{ width: 36, height: 20 }}>
                                                {index + 1}
                                            </Avatar>
                                            <Text fw={500} c="dimmed">
                                                Empty Slot
                                            </Text>
                                        </Group>
                                        <Group gap="xs">
                                            <Box w={8} h={8} bg="gray" style={{ borderRadius: "50%" }} />
                                            <Text size="sm" c="dimmed">
                                                Empty
                                            </Text>
                                        </Group>
                                    </Group>
                                </Box>
                            );
                        }

                        return (
                            <Box
                                key={player.connectionId}
                                p="md"
                                style={{ border: "1px solid var(--mantine-color-gray-3)", borderRadius: 8 }}
                            >
                                <Group justify="space-between" wrap="nowrap">
                                    <Group gap="md" wrap="nowrap" style={{ minWidth: 0, flex: 1, overflow: "hidden" }}>
                                        <ElectroCar width={36} height={20} />
                                        <Box style={{ minWidth: 0, flex: 1, overflow: "hidden" }}>
                                            <Group gap="xs" wrap="nowrap">
                                                <Text fw={500}>{player.name || `Player ${index + 1}`}</Text>
                                                {index === 0 && (
                                                    <Badge size="xs" variant="light" style={{ flexShrink: 0 }}>
                                                        Host
                                                    </Badge>
                                                )}
                                            </Group>
                                            <Text
                                                size="sm"
                                                c="dimmed"
                                                style={{
                                                    overflow: "hidden",
                                                    textOverflow: "ellipsis",
                                                    whiteSpace: "nowrap",
                                                    width: "0",
                                                    minWidth: "100%",
                                                }}
                                            >
                                                ID: {player.connectionId}
                                            </Text>
                                        </Box>
                                    </Group>
                                    <Stack gap="xs" align="flex-end">
                                        <ConnectionStatus
                                            player={player}
                                            index={index}
                                            peerStatuses={peerStatuses}
                                            currentConnectionId={currentConnectionId}
                                            peerLatencies={peerLatencies}
                                        />

                                        {/* Overall connection status (host only) */}
                                        {isHost && player.connectionId !== currentConnectionId && (
                                            <MeshStatus
                                                index={index}
                                                gamePlayers={gamePlayers}
                                                peerStatus={peersActualStatuses?.[index]}
                                            />
                                        )}

                                        {/* Game data loading status */}
                                        {((isHost &&
                                            player.connectionId !== currentConnectionId &&
                                            peersActualStatuses?.[index]) ||
                                            player.connectionId === currentConnectionId) && (
                                            <Group gap="xs" wrap="nowrap">
                                                <Box
                                                    w={8}
                                                    h={8}
                                                    bg={
                                                        player.connectionId === currentConnectionId
                                                            ? isLoadingGameData
                                                                ? "red"
                                                                : "green"
                                                            : peersActualStatuses?.[index]?.isLoadingGameData
                                                              ? "red"
                                                              : "green"
                                                    }
                                                    style={{ borderRadius: "50%" }}
                                                />
                                                <Text size="sm" c="dimmed" style={{ whiteSpace: "nowrap" }}>
                                                    {player.connectionId === currentConnectionId
                                                        ? isLoadingGameData
                                                            ? "Loading Game"
                                                            : "Game Ready"
                                                        : peersActualStatuses?.[index]?.isLoadingGameData
                                                          ? "Loading Game"
                                                          : "Game Ready"}
                                                </Text>
                                            </Group>
                                        )}
                                    </Stack>
                                </Group>
                            </Box>
                        );
                    })}
                    </Stack>
                </Box>
            </Flex>
        </Box>
    );
};
