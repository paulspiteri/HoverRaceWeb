declare global {
    var sendGameMessage: ((playerId: number, data: Uint8Array, reliable: boolean) => boolean) | undefined;
    var onLapComplete: ((newLap: number, lapTimeMs: number, vehicleType: number, ghostReplayData: Uint8Array) => void) | undefined;
}

export {};
