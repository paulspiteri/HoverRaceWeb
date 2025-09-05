import { useEffect, useMemo, useState } from "react";

interface InteropInterface {
    _main: () => void;
    _Quit: () => void;
    _SetPlayerId: (playerId: number) => void;
    _SetPeerStatus: (playerId: number, isConnected: boolean, minLatency: number, avgLatency: number) => void;
}

interface GameInstanceAPI {
    startGame: (playerIndex: number) => void;
    setPlayerStatus: (playerId: number, isConnected: boolean, minLatency: number, avgLatency: number) => void;
}

declare global {
    var HoverRace: (config: {
        noInitialRun: boolean;
        print: (...args: unknown[]) => void;
        canvas: HTMLCanvasElement;
        locateFile: (path: string) => string;
        totalDependencies?: number;
        monitorRunDependencies?: (left: number) => void;
        onRuntimeInitialized?: () => void;
    }) => Promise<InteropInterface>;
}

const webglContextLostEventHandler = (e: Event) => {
    alert("WebGL context lost. You will need to reload the page.");
    e.preventDefault();
};

export const useGameInstance = (canvas: HTMLCanvasElement | null) => {
    const [gameInstance, setGameInstance] = useState<InteropInterface>();
    const [isLoadingGameData, setIsLoadingGameData] = useState(false);

    useEffect(() => {
        if (!canvas) return;

        canvas.addEventListener("webglcontextlost", webglContextLostEventHandler, false);

        console.log("Creating game instance...");
        const abortController = new AbortController();
        setIsLoadingGameData(true);

        let instanceForDispose: InteropInterface | undefined;
        HoverRace({
            noInitialRun: true,
            print(...args) {
                console.log(...args);
            },
            canvas,
            locateFile: (path: string) => "/" + path,
            onRuntimeInitialized() {
                console.log("Runtime initialized");
                setIsLoadingGameData(false);
            },
        }).then((instance) => {
            if (!abortController.signal.aborted) {
                console.log("Setting game instance...");
                setGameInstance(instance);
                instanceForDispose = instance;
            }
        });

        return () => {
            console.log("Disposing game instance...");
            abortController.abort();
            if (instanceForDispose) {
                console.log("Quitting game instance...");
                try {
                    instanceForDispose._Quit();
                } catch (e: unknown) {
                    if (typeof e === "object" && (e as { name: string }).name === "ExitStatus") {
                        console.log("Program exited.");
                    } else {
                        console.error("Unexpected error:", e);
                        throw e;
                    }
                }
            }
            setGameInstance(undefined);
            setIsLoadingGameData(false);

            if (canvas) {
                canvas.removeEventListener("webglcontextlost", webglContextLostEventHandler);
            }
        };
    }, [canvas]);

    const gameInstanceApi = useMemo(() => {
        if (gameInstance) {
            return {
                startGame: (playerIndex: number) => {
                    gameInstance._SetPlayerId(playerIndex);
                    gameInstance._main();
                },
                setPlayerStatus: (playerId: number, isConnected: boolean, minLatency: number, avgLatency: number) => {
                    gameInstance._SetPeerStatus(playerId, isConnected, minLatency, avgLatency);
                },
            } satisfies GameInstanceAPI;
        }
    }, [gameInstance]);

    return { gameInstanceApi, isLoadingGameData };
};
