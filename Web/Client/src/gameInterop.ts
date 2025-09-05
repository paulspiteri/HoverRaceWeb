import { useEffect, useState } from "react";

interface InteropInterface {
    _main: () => void;
    _Quit: () => void;
    _SetPlayerId: (playerId: number) => void;
    _SetPeerStatus: (playerId: number, isConnected: boolean, minLatency: number, avgLatency: number) => void;
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

export const startGame = async (gameInstance: InteropInterface, playerIndex: number) => {
    gameInstance._SetPlayerId(playerIndex);
    gameInstance._main();
};

export const useGameInstance = (canvas: HTMLCanvasElement | null) => {
    const [gameInstance, setGameInstance] = useState<InteropInterface>();
    const [isLoading, setIsLoading] = useState(false);

    useEffect(() => {
        if (!canvas) return;

        canvas.addEventListener("webglcontextlost", webglContextLostEventHandler, false);

        console.log("Creating game instance...");
        const abortController = new AbortController();
        setIsLoading(true);

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
                setIsLoading(false);
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
                instanceForDispose._Quit();
            }
            setGameInstance(undefined);
            setIsLoading(false);

            if (canvas) {
                canvas.removeEventListener("webglcontextlost", webglContextLostEventHandler);
            }
        };
    }, [canvas]);

    return { gameInstance, isLoading };
};
