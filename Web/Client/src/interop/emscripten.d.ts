import type {VehicleType} from '../../../Server/src/leaderboardTypes.ts';

declare global {
    var sendGameMessage: ((playerId: number, data: Uint8Array, reliable: boolean) => boolean) | undefined;
    var onLapComplete: ((newLap: number, lapTimeMs: number, vehicleType: VehicleType, ghostReplayData: Uint8Array) => void) | undefined;
}

export {};
