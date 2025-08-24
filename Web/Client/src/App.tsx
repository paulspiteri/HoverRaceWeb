import { ConnectionStatus } from "@/ConnectionStatus.tsx";
import { GameList } from "@/GameList.tsx";
import { Button, Container, Stack, Title, Group, Flex, Box } from "@mantine/core";
import { useGameData } from "@/useGameData.ts";
import { useCallback, useEffect, useState } from "react";
import { ActiveGame } from "@/ActiveGame.tsx";
import type { JoinedGame } from "./types";
import { usePeers } from "@/usePeers.ts";

export interface ActiveGame {
    gameId: string;
    token: string;
}

function App() {
    const [activeGame, setActiveGame] = useState<ActiveGame>();

    const { connectionId, games, commands, eventSource } = useGameData("http://localhost:3001/api", setActiveGame);

    const currentGame =
        activeGame && (games.find((g) => g.id === activeGame.gameId && "players" in g) as JoinedGame | undefined);

    const onGameData = useCallback((playerId: number, data: unknown) => {
        let binaryData = null;
        if (data instanceof Uint8Array) {
            binaryData = data;
        } else if (data instanceof ArrayBuffer) {
            binaryData = new Uint8Array(data);
        } else {
            console.error(`[JS] Received unsupported data format:`, typeof data);
            return;
        }
        receiveGameData(playerId, binaryData);
    }, []);

    const { peerStatuses, peersActualStatuses, sendData } = usePeers(
        connectionId,
        currentGame,
        eventSource,
        commands.sendSignal,
        activeGame?.token,
        onGameData,
    );

    useEffect(() => void (global.sendGameMessage = sendData), [sendData]);

    const handleJoinGame = (gameId: string) => {
        if (connectionId) {
            const savedName = localStorage.getItem("hoverrace-player-name");
            commands.joinGame(gameId, connectionId, savedName || undefined);
        }
    };

    const handleLeaveGame = () => {
        if (activeGame) {
            commands.leaveGame(activeGame.gameId, activeGame.token);
        }
    };

    const handleStartGame = async () => {
        if (activeGame) {
            await commands.startGame(activeGame.gameId, activeGame.token);
        }
    };

    const handleUpdatePlayer = async (name: string) => {
        if (activeGame) {
            await commands.updatePlayer(activeGame.gameId, activeGame.token, name);
        }
    };

    useEffect(() => {
        {
            if (currentGame?.status === "playing") {
                startGame(0);
            }
        }
    }, [currentGame?.status]);

    return (
        <Container fluid h="100vh" style={{ overflow: "hidden" }} p={0}>
            <Flex h="100%">
                {/* Main content area */}
                <Box flex={1} p="xl">
                    <Flex direction="column" align="center" h="100%">
                        <Container size="xl" w="100%" h="100%">
                            <Stack gap="xl" h="100%">
                                <Group justify="center">
                                    <Title
                                        order={1}
                                        size="h1"
                                        style={{
                                            background: "linear-gradient(45deg, #1976d2, #9c27b0)",
                                            WebkitBackgroundClip: "text",
                                            WebkitTextFillColor: "transparent",
                                        }}
                                    >
                                        HoverRace
                                    </Title>
                                </Group>

                                {!activeGame && (
                                    <Stack gap="lg">
                                        <ConnectionStatus connectionId={connectionId} />
                                        <Group justify="center">
                                            <Button
                                                onClick={() => {
                                                    const savedName = localStorage.getItem("hoverrace-player-name");
                                                    commands.createGame(connectionId!, savedName || undefined);
                                                }}
                                                disabled={!connectionId}
                                                size="lg"
                                                px="xl"
                                            >
                                                Create New Game
                                            </Button>
                                        </Group>
                                    </Stack>
                                )}
                                {activeGame && currentGame && (
                                    <ActiveGame
                                        game={currentGame}
                                        onClose={handleLeaveGame}
                                        onStartGame={handleStartGame}
                                        peerStatuses={peerStatuses}
                                        currentConnectionId={connectionId}
                                        onUpdatePlayer={handleUpdatePlayer}
                                        peersActualStatuses={peersActualStatuses}
                                    />
                                )}
                            </Stack>
                        </Container>
                    </Flex>
                </Box>
                {/* Right sidebar for game list */}
                <Flex
                    w={384}
                    p="xl"
                    direction="column"
                    style={{
                        borderLeft: "1px solid var(--mantine-color-gray-3)",
                        overflow: "hidden",
                    }}
                >
                    <GameList
                        games={games}
                        disabled={activeGame !== undefined}
                        onJoinGame={handleJoinGame}
                        onLeaveGame={handleLeaveGame}
                    />
                </Flex>
            </Flex>
        </Container>
    );
}

export default App;
