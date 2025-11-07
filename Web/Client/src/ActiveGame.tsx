import * as React from "react";
import { Button, Card, Text, Title, Group, Box, Tabs, ActionIcon } from "@mantine/core";
import type { JoinedGame } from "./types";
import { useMemo } from "react";
import { PlayerList } from "./PlayerList";
import { PlayerNameInput } from "./PlayerNameInput";
import { GameChat } from "./GameChat";
import type { PeerConnectionStatusMessage } from "@/peerTypes.ts";
import type { PeerConnectionLatency } from "@/usePeers.ts";
import { useAtomValue, useSetAtom } from "jotai";
import { connectionIdAtom, gameScreenModeAtom, gameTokenAtom } from "@/atoms.ts";
import { useLeaveGame } from "@/hooks/useLeaveGame";
import { useStartGame } from "@/hooks/useStartGame";
import { GameSettingsModal } from "./GameSettingsModal";
import { notifications } from "@mantine/notifications";
import { IconArrowLeft, IconSettings } from "@tabler/icons-react";
import styles from "./ActiveGame.module.css";

function formatLapTime(milliseconds: number): string {
    const minutes = Math.floor(milliseconds / 60000);
    const seconds = Math.floor((milliseconds % 60000) / 1000);
    const ms = milliseconds % 1000;
    return `${minutes}:${seconds.toString().padStart(2, '0')}.${ms.toString().padStart(3, '0')}`;
}

interface ActiveGameProps {
    game: JoinedGame;
    peerStatuses: ("connecting" | "connected" | "disconnected" | undefined)[] | undefined;
    peersActualStatuses?: (PeerConnectionStatusMessage | undefined)[];
    peerLatencies?: (PeerConnectionLatency | undefined)[];
    isLoadingGameData: boolean;
    bestLapTime?: number;
}

export const ActiveGame: React.FC<ActiveGameProps> = ({
    game,
    peerStatuses,
    peersActualStatuses,
    peerLatencies,
    isLoadingGameData,
    bestLapTime,
}) => {
    const currentConnectionId = useAtomValue(connectionIdAtom);
    const gameToken = useAtomValue(gameTokenAtom);
    const setGameScreenMode = useSetAtom(gameScreenModeAtom);
    const leaveGameMutation = useLeaveGame();
    const startGameMutation = useStartGame(game.id);

    const [settingsModalOpen, setSettingsModalOpen] = React.useState(false);

    const handleLeaveGame = async () => {
        if (gameToken) {
            try {
                await leaveGameMutation.mutateAsync({ gameId: game.id, gameToken });
                setGameScreenMode("hidden");
            } catch (error) {
                console.error("Failed to leave game:", error);
                notifications.show({
                    title: "Failed to leave game",
                    message: "Unable to leave the game. Please try again.",
                    color: "red",
                });
            }
        }
    };

    const handleStartGame = async () => {
        try {
            await startGameMutation.mutateAsync();
        } catch (error) {
            console.error("Failed to start game:", error);
            notifications.show({
                title: "Failed to start game",
                message: "Unable to start the game. Please try again.",
                color: "red",
            });
        }
    };

    // Get current player info and determine if creator (index 0)
    const currentPlayerIndex = game.players.findIndex((p) => p?.connectionId === currentConnectionId);
    const currentPlayer = currentPlayerIndex >= 0 ? game.players[currentPlayerIndex] : undefined;
    const currentPlayerName = currentPlayer?.name || "";
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
            return peerStatuses[index] === "connected";
        });
    }, [game.players, peerStatuses, currentConnectionId, isCreator, peersActualStatuses]);

    // Check if all peers are done loading game data
    const allPeersGameReady = useMemo(() => {
        if (!peersActualStatuses || !isCreator) return true;

        return game.players.every((player, index) => {
            if (!player || player.connectionId === currentConnectionId) return true; // Skip empty slots and self
            const peerStatus = peersActualStatuses[index];
            return peerStatus ? !peerStatus.isLoadingGameData : false;
        });
    }, [game.players, currentConnectionId, isCreator, peersActualStatuses]);
    return (
        <Card
            className={styles.card}
        >
            <Card.Section withBorder inheritPadding py="xs">
                <Group justify="space-between" align="center">
                    <Group gap="sm" align="center">
                        <ActionIcon
                            variant="subtle"
                            onClick={handleLeaveGame}
                            disabled={leaveGameMutation.isPending || !currentConnectionId}
                            size="lg"
                            title={isCreator ? "Cancel Game" : "Leave Game"}
                        >
                            <IconArrowLeft size={20} />
                        </ActionIcon>
                        <Box>
                            <Title order={2}>{game.name}</Title>
                            <Text size="sm" c="dimmed">
                                Game • {game.playerCount}/{game.maxPlayers} Players
                                {bestLapTime && ` • Best: ${formatLapTime(bestLapTime)}`}
                            </Text>
                        </Box>
                    </Group>

                </Group>
            </Card.Section>

            <Box className={styles.content}>
                <Group gap="xs" align="end">
                    <Box flex={1}>
                        <PlayerNameInput currentPlayerName={currentPlayerName} gameId={game.id} />
                    </Box>
                    {isCreator && (
                        <Group gap="xs">
                            {game.status === "waiting" && (
                                <ActionIcon
                                    variant="subtle"
                                    onClick={() => setSettingsModalOpen(true)}
                                    size="lg"
                                    title="Game Settings"
                                >
                                    <IconSettings size={20} />
                                </ActionIcon>
                            )}
                            <Button
                                onClick={handleStartGame}
                                disabled={
                                    game.players.filter((p) => p !== undefined).length < 2 ||
                                    !allPeersConnected ||
                                    !allPeersGameReady ||
                                    game.status === "playing" ||
                                    startGameMutation.isPending ||
                                    !currentConnectionId
                                }
                                size="sm"
                            >
                                {game.status === "playing" ? "Started" : "Start"}
                            </Button>
                        </Group>
                    )}
                </Group>

                <Box className={styles.gameArea}>
                    {/* Mobile: Tabbed Layout */}
                    <Box className={styles.tabsContainer}>
                        <Tabs defaultValue="chat" className={styles.tabs}>
                            <Tabs.List>
                                <Tabs.Tab value="chat">Chat</Tabs.Tab>
                                <Tabs.Tab value="players">Players ({game.players.filter(p => p).length})</Tabs.Tab>
                            </Tabs.List>

                            <Tabs.Panel value="chat" className={styles.tabPanel}>
                                <GameChat />
                            </Tabs.Panel>

                            <Tabs.Panel value="players" className={styles.tabPanel}>
                                <PlayerList
                                    gamePlayers={game.players}
                                    peerStatuses={peerStatuses}
                                    currentConnectionId={currentConnectionId}
                                    peersActualStatuses={peersActualStatuses}
                                    peerLatencies={peerLatencies}
                                    isHost={isCreator}
                                    isLoadingGameData={isLoadingGameData}
                                />
                            </Tabs.Panel>
                        </Tabs>
                    </Box>

                    {/* Desktop: Grid Layout */}
                    <Box className={styles.gridContainer}>
                        <GameChat />
                        <PlayerList
                            gamePlayers={game.players}
                            peerStatuses={peerStatuses}
                            currentConnectionId={currentConnectionId}
                            peersActualStatuses={peersActualStatuses}
                            peerLatencies={peerLatencies}
                            isHost={isCreator}
                            isLoadingGameData={isLoadingGameData}
                        />
                    </Box>
                </Box>

            </Box>

            <GameSettingsModal
                game={game}
                opened={settingsModalOpen}
                onClose={() => setSettingsModalOpen(false)}
            />
        </Card>
    );
};
