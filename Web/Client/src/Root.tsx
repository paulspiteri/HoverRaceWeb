import { useCallback, useRef, useEffect } from "react";
import { Container, Stack, Title, Group, Flex, Box, ActionIcon } from "@mantine/core";
import { IconHome, IconMaximize } from "@tabler/icons-react";
import { useGameData } from "@/useGameData.ts";
import { GameList } from "@/GameList.tsx";
import { ConnectionStatus } from "@/ConnectionStatus.tsx";
import { useNavigate, Outlet, useMatch } from "react-router-dom";
import styles from "./Root.module.css";
import { useSetAtom, useAtom } from "jotai";
import {
    connectionIdAtom,
    gameTokenAtom,
    commandsAtom,
    gamesAtom,
    eventSourceAtom,
    canvasAtom,
    gameScreenModeAtom,
} from "@/atoms.ts";

export function Root() {
    const navigate = useNavigate();
    const canvasRef = useRef<HTMLCanvasElement | null>(null);
    const gameMatch = useMatch("/game/*");
    const [gameScreenMode, setGameScreenMode] = useAtom(gameScreenModeAtom);

    useEffect(() => {
        const handleFullscreenChange = () =>
            void setGameScreenMode(document.fullscreenElement ? "fullscreen" : "maximized");
        document.addEventListener("fullscreenchange", handleFullscreenChange);
        return () => document.removeEventListener("fullscreenchange", handleFullscreenChange);
    }, [setGameScreenMode]);

    // Sync atoms setup
    const setConnectionId = useSetAtom(connectionIdAtom);
    const setGameToken = useSetAtom(gameTokenAtom);
    const setCommands = useSetAtom(commandsAtom);
    const setGames = useSetAtom(gamesAtom);
    const setEventSource = useSetAtom(eventSourceAtom);
    const setCanvasAtom = useSetAtom(canvasAtom);

    const setActiveGame = useCallback(
        (id: string | undefined, token?: string) => {
            navigate(id !== undefined ? `/game/${id}` : "/");
            setGameToken(token);
        },
        [navigate, setGameToken],
    );

    const { connectionId, games, commands, eventSource } = useGameData(
        `${import.meta.env.VITE_SERVER_URL}/api`,
        setActiveGame,
    );

    useEffect(() => setConnectionId(connectionId), [connectionId, setConnectionId]);
    useEffect(() => setCommands(commands), [commands, setCommands]);
    useEffect(() => setGames(games), [games, setGames]);
    useEffect(() => setEventSource(eventSource), [eventSource, setEventSource]);
    useEffect(() => setCanvasAtom(canvasRef), [canvasRef, setCanvasAtom]);

    const handleJoinGame = (id: string) => {
        if (connectionId) {
            const savedName = localStorage.getItem("hoverrace-player-name");
            commands.joinGame(id, connectionId, savedName || undefined);
        }
    };

    const handleCanvasClick = () => {
        if (gameScreenMode === "fullscreen") {
            document.exitFullscreen();
            setGameScreenMode("maximized");
        } else if (gameScreenMode === "mini") {
            setGameScreenMode("maximized");
        } else {
            setGameScreenMode("mini");
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

                                <Outlet />
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
                    <ConnectionStatus />
                    <GameList games={games} onJoinGame={handleJoinGame} />
                </Flex>
            </Flex>
            <div
                className={`${styles["canvas-border"]} ${gameScreenMode === "maximized" ? styles["canvas-border-maximized"] : ""}`}
                style={{
                    display: !gameMatch ? "none" : undefined,
                }}
            >
                <canvas
                    ref={canvasRef}
                    id="canvas"
                    tabIndex={-1}
                    className={`${styles["canvas-emscripten"]} ${gameScreenMode === "mini" ? styles["canvas-interactive"] : ""}`}
                    onClick={handleCanvasClick}
                />

                {gameScreenMode === "maximized" && (
                    <ActionIcon
                        size="lg"
                        variant="filled"
                        color="blue"
                        onClick={() => canvasRef.current?.requestFullscreen()}
                        style={{
                            position: "absolute",
                            top: 10,
                            right: 10,
                        }}
                        title="Enter fullscreen"
                    >
                        <IconMaximize size={20} />
                    </ActionIcon>
                )}
            </div>
        </Container>
    );
}
