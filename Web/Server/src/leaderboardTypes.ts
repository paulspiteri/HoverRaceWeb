// Vehicle type enum based on MR_* constants
export enum VehicleType {
    ELECTRO = 0,
    HITECH = 1,
    BITURBO = 2
}

export type LeaderboardEntry = {
    id: number;
    playerName: string;
    trackName: string;
    lapTimeMs: number;
    isMobile: boolean;
    vehicleType: number;
    createdAt: Date;
};

export type SubmitLapTimeRequest = {
    playerName: string;
    trackName: string;
    lapTimeMs: number;
    isMobile: boolean;
    vehicleType: number;
    ghostReplay: string; // Base64 encoded ghost replay data
};

export type SubmitLapTimeResult = {
    success: boolean;
};

export type GetLeaderboardRequest = {
    trackName: string;
    isMobile: boolean;
    limit?: number;
};
