interface InteropInterface {
    _main: () => void;
    _SetPlayerId: (playerId: number) => void;
    _SetPeerStatus: (playerId: number, isConnected: boolean, minLatency: number, avgLatency: number) => void;
}

declare global {
    var HoverRace: (config: {
        noInitialRun: boolean;
        print: (...args: unknown[]) => void;
        canvas: HTMLCanvasElement;
        locateFile: (path: string) => string;
        totalDependencies: number;
        monitorRunDependencies: (left: number) => void;
        onRuntimeInitialized: () => void;
    }) => Promise<InteropInterface>;
}

export const startGame = async (canvas: HTMLCanvasElement, playerIndex: number) => {
    const Module = await HoverRace({
        noInitialRun: true,
        print(...args) {
            console.log(...args);
        },
        canvas,
        locateFile: (path: string) => "/" + path,
        totalDependencies: 0,
        monitorRunDependencies(left: number) {
            console.log("Dependencies remaining:", left);
        },
        onRuntimeInitialized() {
            console.log("Runtime ready, but files might still be loading...");
        },
    });

    console.log("Files definitely loaded now");
    Module._SetPlayerId(playerIndex);
    Module._main();
};
