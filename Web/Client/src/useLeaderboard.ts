import { useQuery } from '@tanstack/react-query';
import type { LeaderboardEntry } from './types';

interface LeaderboardResponse {
    entries: LeaderboardEntry[];
}

async function fetchLeaderboard(trackName: string, isMobile: boolean, limit: number = 10): Promise<LeaderboardEntry[]> {
    const url = `${import.meta.env.VITE_SERVER_URL}/api/leaderboard/${encodeURIComponent(trackName)}?isMobile=${isMobile}&limit=${limit}`;
    const response = await fetch(url);
    if (!response.ok) {
        throw new Error(`Failed to fetch leaderboard: ${response.statusText}`);
    }
    const data: LeaderboardResponse = await response.json();
    return data.entries;
}

export function useLeaderboard(trackName: string | undefined, limit: number = 10) {
    // Detect if device is mobile
    const isMobile = window.matchMedia("(pointer: coarse)").matches;

    return useQuery({
        queryKey: ['useLeaderboard', trackName, isMobile, limit],
        queryFn: () => fetchLeaderboard(trackName!, isMobile, limit),
        enabled: Boolean(trackName),
    });
}
