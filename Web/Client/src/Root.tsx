import { useCallback, useRef } from "react";
import { Container, Stack, Title, Group, Flex, Box, ActionIcon } from "@mantine/core";
import { IconHome } from "@tabler/icons-react";
import { useGameData } from "@/useGameData.ts";
import { GameList } from "@/GameList.tsx";
import { ConnectionStatus } from "@/ConnectionStatus.tsx";
import { useNavigate, Outlet, useMatch } from "react-router-dom";
import type { GameOutletContext } from "./App";
import styles from "./App.module.css";

export function Root() {
    const navigate = useNavigate();
    const canvasRef = useRef<HTMLCanvasElement | null>(null);
    const gameToken = useRef<string>(undefined);
    const gameMatch = useMatch("/game/*");

    const setActiveGame = useCallback(
        (id: string | undefined, token?: string) => {
            navigate(id !== undefined ? `/game/${id}` : "/");
            gameToken.current = token;
        },
        [navigate],
    );
    const { connectionId, games, commands, eventSource } = useGameData(
        `${window.location.protocol}//${window.location.hostname}:3001/api`,
        setActiveGame,
    );

    const handleJoinGame = (id: string) => {
        if (connectionId) {
            const savedName = localStorage.getItem("hoverrace-player-name");
            commands.joinGame(id, connectionId, savedName || undefined);
        }
    };

    return (
        <Container fluid h="100vh" style={{ overflow: "hidden" }} p={0}>
            <Flex h="100%">
                {/* Main content area */}
                <Box flex={1} p="xl">
                    <Flex direction="column" align="center" h="100%">
                        <Container size="xl" w="100%" h="100%">
                            <Stack gap="xl" h="100%">
                                <Group justify="center">
                                    <Group gap="sm" align="center">
                                        <ActionIcon
                                            variant="subtle"
                                            onClick={() => navigate("/")}
                                            size="xl"
                                            title="Go to Home"
                                        >
                                            <IconHome size={24} />
                                        </ActionIcon>
                                        <Title
                                            order={1}
                                            size="h1"
                                            style={{
                                                background: "linear-gradient(45deg, #1976d2, #9c27b0)",
                                                WebkitBackgroundClip: "text",
                                                WebkitTextFillColor: "transparent",
                                                cursor: "pointer",
                                            }}
                                            onClick={() => navigate("/")}
                                        >
                                            HoverRace
                                        </Title>
                                    </Group>
                                </Group>

                                <Outlet
                                    context={
                                        {
                                            connectionId,
                                            games,
                                            commands,
                                            eventSource,
                                            gameToken: gameToken.current,
                                            canvasRef,
                                        } satisfies GameOutletContext
                                    }
                                />
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
                    <ConnectionStatus connectionId={connectionId} />
                    <GameList games={games} onJoinGame={handleJoinGame} />
                </Flex>
            </Flex>
            <div className={styles["canvas-border"]}>
                <canvas
                    ref={canvasRef}
                    id="canvas"
                    tabIndex={-1}
                    className={styles["canvas-emscripten"]}
                    style={{
                        width: "350px",
                        height: "262px",
                        display: !gameMatch ? "none" : undefined,
                    }}
                />
            </div>
        </Container>
    );
}
