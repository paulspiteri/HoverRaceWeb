// Vehicle type enum based on MR_* constants
export const VehicleType = {
    ELECTRO: 0,
    HITECH: 1,
    BITURBO: 2
} as const;

export type VehicleType = typeof VehicleType[keyof typeof VehicleType];

export type LeaderboardEntry = {
    id: number;
    playerName: string | null;
    trackName: string;
    lapTimeMs: number;
    isMobile: boolean;
    vehicleType: VehicleType;
    createdAt: Date;
};

export type SubmitLapTimeRequest = {
    playerName?: string;
    trackName: string;
    lapTimeMs: number;
    isMobile: boolean;
    vehicleType: VehicleType;
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
