import { ConnectionStatus } from "@/ConnectionStatus.tsx";
import { GameList } from "@/GameList.tsx";
import { Button, Container, Stack, Title, Group, Flex, Box } from "@mantine/core";
import { useGameData } from "@/useGameData.ts";
import { useCallback, useEffect, useRef } from "react";
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
} from "react-router-dom";
import type { Commands } from "@/commands.ts";

interface GameOutletContext {
    connectionId: string | undefined;
    games: Game[];
    commands: Commands;
    eventSource: EventSource | undefined;
    gameToken: string | undefined;
}

function Root() {
    const navigate = useNavigate();
    const gameToken = useRef<string>(undefined);

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

                                <Outlet
                                    context={
                                        {
                                            connectionId,
                                            games,
                                            commands,
                                            eventSource,
                                            gameToken: gameToken.current,
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

function GamePage() {
    const { gameId } = useParams();
    const { connectionId, games, commands, eventSource, gameToken } = useOutletContext<GameOutletContext>();

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

    const onGamePlayerDisconnected = useCallback((playerId: number) => void setPlayerStatus(playerId, false, 0, 0), []);

    const { peerStatuses, peersActualStatuses, peerLatencies, sendData } = usePeers(
        connectionId,
        currentGame,
        eventSource,
        commands.sendSignal,
        gameToken,
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
            if (isGamePlaying) {
                peerStatuses?.forEach((x, idx) => {
                    const latencies = peerLatencies?.[idx];
                    setPlayerStatus(
                        idx,
                        x === "connected",
                        latencies?.minimumLatency ?? 0,
                        latencies?.averageLatency ?? 0,
                    );
                });
            }
        }
    }, [peerStatuses, peerLatencies, isGamePlaying]);

    useEffect(() => {
        if (isGamePlaying) {
            startGame(playerIndex);
        }
    }, [isGamePlaying, playerIndex]);

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
}

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
