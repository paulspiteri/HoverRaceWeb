import { useEffect, useMemo, useState } from "react";
import { notifications } from "@mantine/notifications";

export const WeaponType = {
    Missile: 0,
    Mine: 1,
    PowerUp: 2,
} as const;

export type WeaponType = typeof WeaponType[keyof typeof WeaponType];

interface InteropInterface {
    _malloc: (size: number) => number;
    _free: (dataPtr: number) => number;
    HEAPU8: Uint8Array;
    _main: () => void;
    _Quit: () => void;
    _ChangeWindowSize: (width: number, height: number) => void;
    _ConfigureGame: (playerId: number, trackNamePtr: number, hasWeapons: boolean, laps: number) => void;
    _SetPeerStatus: (playerId: number, isConnected: boolean, minLatency: number, avgLatency: number) => void;
    _ReceivePeerMessage: (playerId: number, dataPtr: number, size: number, reliable: boolean) => void;
    _SetCurrentWeapon: (weaponType: number) => void;
}

export interface GameInstanceAPI {
    setWindowSize: (width: number, height: number) => void;
    startGame: (playerIndex: number, trackName: string, hasWeapons: boolean, laps: number) => void;
    setPlayerStatus: (playerId: number, isConnected: boolean, minLatency: number, avgLatency: number) => void;
    receiveGameData: (playerId: number, binaryData: Uint8Array, reliable: boolean) => void;
    setCurrentWeapon: (weaponType: number) => void;
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
    notifications.show({
        title: "WebGL Context Lost",
        message: "The WebGL context was lost. You will need to reload the page to continue playing.",
        color: "red",
        autoClose: false,
    });
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
            locateFile: (path: string) => import.meta.env.VITE_GAME_URL + "/" + path,
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

    const gameInstanceApi = useMemo<GameInstanceAPI | undefined>(() => {
        if (gameInstance) {
            return {
                setWindowSize: (width: number, height: number) => {
                    gameInstance._ChangeWindowSize(width, height);
                },
                startGame: (playerIndex: number, trackName: string, hasWeapons: boolean, laps: number) => {
                    // Allocate memory for track name string in C++ heap
                    const trackNamePtr = gameInstance._malloc(trackName.length + 1);
                    gameInstance.HEAPU8.set(new TextEncoder().encode(trackName + '\0'), trackNamePtr);

                    // Configure the game with all parameters
                    gameInstance._ConfigureGame(playerIndex, trackNamePtr, hasWeapons, laps);

                    // Free the allocated memory
                    gameInstance._free(trackNamePtr);

                    // Actually start the game
                    gameInstance._main();
                },
                setPlayerStatus: (playerId: number, isConnected: boolean, minLatency: number, avgLatency: number) => {
                    gameInstance._SetPeerStatus(playerId, isConnected, minLatency, avgLatency);
                },
                receiveGameData: (playerId: number, binaryData: Uint8Array, reliable: boolean) => {
                    // Allocate memory in Emscripten heap
                    const dataPtr = gameInstance._malloc(binaryData.length);
                    gameInstance.HEAPU8.set(binaryData, dataPtr);

                    // Call C++ function
                    gameInstance._ReceivePeerMessage(playerId, dataPtr, binaryData.length, reliable);

                    // Free the allocated memory
                    gameInstance._free(dataPtr);
                },
                setCurrentWeapon: (weaponType: number) => {
                    gameInstance._SetCurrentWeapon(weaponType);
                },
            } satisfies GameInstanceAPI;
        }
    }, [gameInstance]);

    return { gameInstanceApi, isLoadingGameData };
};
