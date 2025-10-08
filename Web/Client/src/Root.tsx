import { useCallback, useRef, useEffect } from "react";
import { Container, Box, ActionIcon } from "@mantine/core";
import { IconMaximize } from "@tabler/icons-react";
import { useGameData } from "@/useGameData.ts";
import { GameList } from "@/GameList.tsx";
import { ConnectionStatus } from "@/ConnectionStatus.tsx";
import { Header } from "@/Header.tsx";
import { useNavigate, Outlet, useMatch } from "react-router-dom";
import { MobileInput } from "@/components/MobileInput.tsx";
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
import { useCanvasResize } from "@/hooks/useCanvasResize.ts";
import { useGameScreenModePinchGesture } from "@/hooks/useGameScreenModePinchGesture.ts";

export function Root() {
    const navigate = useNavigate();
    const fullscreenContainerRef = useRef<HTMLDivElement | null>(null);
    const canvasRef = useRef<HTMLCanvasElement | null>(null);
    const gameMatch = useMatch("/game/*");
    const [gameScreenMode, setGameScreenMode] = useAtom(gameScreenModeAtom);
    const { canvasSize, handleResizeStart } = useCanvasResize();
    const { handleTouchStart, handleTouchMove, handleTouchEnd } = useGameScreenModePinchGesture(gameScreenMode, setGameScreenMode);
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
        <Container
            fluid
            className={`${styles.root} ${gameMatch ? styles.rootInGame : styles.rootHome}`}
            style={{ overflow: "hidden" }}
            p={0}
        >
            {/* Header */}
            <Box className={styles.header}>
                {!gameMatch &&<Header />}
            </Box>

            {/* Main Content Area */}
            <Box className={styles.content}>
                <Outlet />
            </Box>

            {/* Games List - always render, CSS handles visibility */}
            <Box className={`${styles.gamesList} ${gameMatch ? styles.gamesListHidden : ""}`}>
                <ConnectionStatus className={styles.desktopConnectionStatus} />
                <GameList/>
            </Box>
            <div
                ref={fullscreenContainerRef}
                className={`${styles["canvas-border"]} ${gameScreenMode === "maximized" ? styles["canvas-border-maximized"] : ""} ${gameScreenMode === "hidden" ? styles["canvas-border-hidden"] : ""}`}
                style={gameScreenMode === "mini" ? {
                    width: `${canvasSize.width}px`,
                    height: `${canvasSize.height}px`,
                } : undefined}
            >
                {gameScreenMode === "mini" && (
                    <div
                        onMouseDown={handleResizeStart}
                        className={styles["resize-handle"]}
                        title="Resize canvas"
                    />
                )}

                <canvas
                    ref={canvasRef}
                    id="canvas"
                    tabIndex={-1}
                    className={`${styles["canvas-emscripten"]} ${gameScreenMode === "mini" ? styles["canvas-interactive"] : ""}`}
                    onClick={handleCanvasClick}
                    onTouchStart={handleTouchStart}
                    onTouchMove={handleTouchMove}
                    onTouchEnd={handleTouchEnd}
                />

                {gameScreenMode === "maximized" && document.fullscreenEnabled && (
                    <ActionIcon
                        size="lg"
                        variant="filled"
                        color="blue"
                        onClick={() => fullscreenContainerRef.current?.requestFullscreen()}
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

                <MobileInput
                    canvasElement={canvasRef.current}
                    enabled={gameScreenMode === "maximized" || gameScreenMode === "fullscreen"}
                />
            </div>
        </Container>
    );
}
