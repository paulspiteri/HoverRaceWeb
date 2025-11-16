import { useQuery } from '@tanstack/react-query';
import type { LeaderboardEntry } from '../types.ts';

interface LeaderboardResponse {
    entries: LeaderboardEntry[];
}

async function fetchLeaderboard(trackName: string, isMobile: boolean | undefined, limit: number = 10, vehicleType?: number): Promise<LeaderboardEntry[]> {
    let url = `${import.meta.env.VITE_SERVER_URL}/api/leaderboard/${encodeURIComponent(trackName)}?limit=${limit}`;

    // Only add isMobile parameter if it's defined (undefined means fetch all platforms)
    if (isMobile !== undefined) {
        url += `&isMobile=${isMobile}`;
    }

    if (vehicleType !== undefined) {
        url += `&vehicleType=${vehicleType}`;
    }
    const response = await fetch(url);
    if (!response.ok) {
        throw new Error(`Failed to fetch leaderboard: ${response.statusText}`);
    }
    const data: LeaderboardResponse = await response.json();
    return data.entries;
}

export function useLeaderboard(
    trackName: string | undefined,
    limit: number = 10,
    vehicleType?: number,
    isMobile?: boolean
) {
    return useQuery({
        queryKey: ['useLeaderboard', trackName, isMobile, limit, vehicleType],
        queryFn: () => fetchLeaderboard(trackName!, isMobile, limit, vehicleType),
        enabled: Boolean(trackName),
    });
}
