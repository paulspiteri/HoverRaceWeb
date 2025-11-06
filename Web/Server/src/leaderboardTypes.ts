export type LeaderboardEntry = {
    id: number;
    playerName: string;
    trackName: string;
    lapTimeMs: number;
    isMobile: boolean;
    createdAt: Date;
};

export type SubmitLapTimeRequest = {
    playerName: string;
    trackName: string;
    lapTimeMs: number;
    isMobile: boolean;
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
