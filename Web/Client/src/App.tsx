import { ConnectionStatus } from "@/ConnectionStatus.tsx";
import { GameList } from "@/GameList.tsx";
import { Button, Container, Stack, Title, Group, Flex, Box, ActionIcon } from "@mantine/core";
import { IconHome } from "@tabler/icons-react";
import { useGameData } from "@/useGameData.ts";
import { type RefObject, useCallback, useEffect, useRef } from "react";
import { ActiveGame } from "@/ActiveGame.tsx";
import type { Game, JoinedGame } from "./types";
import { usePeers } from "@/usePeers.ts";
import {
    createBrowserRouter,
    RouterProvider,
    useNavigate,
    useParams,
    Outlet,
    useOutletContext,
    useMatch,
} from "react-router-dom";
import type { Commands } from "@/commands.ts";
import styles from "./App.module.css";
import { useGameInstance } from "@/gameInterop.ts";

interface GameOutletContext {
    connectionId: string | undefined;
    games: Game[];
    commands: Commands;
    eventSource: EventSource | undefined;
    gameToken: string | undefined;
    canvasRef: RefObject<HTMLCanvasElement | null>;
}

function Root() {
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

const NoGame: React.FC = () => {
    const { connectionId, commands } = useOutletContext<GameOutletContext>();

    return (
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
    );
};

const GamePage: React.FC = () => {
    const { gameId } = useParams();
    const { connectionId, games, commands, eventSource, gameToken, canvasRef } = useOutletContext<GameOutletContext>();
    const { gameInstanceApi, isLoading } = useGameInstance(canvasRef.current);

    const currentGame =
        gameId !== undefined
            ? (games.find((x) => x.id === gameId && "players" in x) as JoinedGame | undefined)
            : undefined;
    const playerIndex = currentGame?.players.findIndex((p) => p?.connectionId === connectionId);

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

    const onGamePlayerDisconnected = useCallback(
        (playerId: number) => void gameInstanceApi?.setPlayerStatus(playerId, false, 0, 0),
        [gameInstanceApi],
    );

    const { peerStatuses, peersActualStatuses, peerLatencies, sendData } = usePeers(
        connectionId,
        currentGame,
        eventSource,
        commands.sendSignal,
        gameToken,
        isLoading,
        onGameData,
        onGamePlayerDisconnected,
    );

    useEffect(() => void (global.sendGameMessage = sendData), [sendData]);

    const handleLeaveGame = () => {
        if (currentGame && gameToken) {
            commands.leaveGame(currentGame.id, gameToken);
        }
    };

    const handleStartGame = async () => {
        if (currentGame && gameToken) {
            await commands.startGame(currentGame.id, gameToken);
        }
    };

    const handleUpdatePlayer = async (name: string) => {
        if (currentGame && gameToken) {
            await commands.updatePlayer(currentGame.id, gameToken, name);
        }
    };

    const isGamePlaying = currentGame?.status === "playing" && playerIndex !== undefined;
    useEffect(() => {
        {
            if (isGamePlaying && gameInstanceApi) {
                peerStatuses?.forEach((x, idx) => {
                    const latencies = peerLatencies?.[idx];
                    gameInstanceApi.setPlayerStatus(
                        idx,
                        x === "connected",
                        latencies?.minimumLatency ?? 0,
                        latencies?.averageLatency ?? 0,
                    );
                });
            }
        }
    }, [peerStatuses, peerLatencies, isGamePlaying, gameInstanceApi]);

    useEffect(() => {
        if (isGamePlaying && gameInstanceApi) {
            gameInstanceApi.startGame(playerIndex);
        }
    }, [gameInstanceApi, isGamePlaying, playerIndex]);

    if (!currentGame) {
        return <div>Game not found</div>;
    }

    return (
        <ActiveGame
            game={currentGame}
            onClose={handleLeaveGame}
            onStartGame={handleStartGame}
            peerStatuses={peerStatuses}
            currentConnectionId={connectionId}
            onUpdatePlayer={handleUpdatePlayer}
            peersActualStatuses={peersActualStatuses}
            peerLatencies={peerLatencies}
        />
    );
};

const router = createBrowserRouter([
    {
        path: "/",
        element: <Root />,
        children: [
            {
                index: true,
                element: <NoGame />,
            },
            {
                path: "game/:gameId",
                element: <GamePage />,
            },
        ],
    },
]);

function App() {
    return <RouterProvider router={router} />;
}

export default App;
