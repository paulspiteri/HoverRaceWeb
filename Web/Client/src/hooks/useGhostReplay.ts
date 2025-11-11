import { useQuery } from '@tanstack/react-query';

interface GhostReplayResponse {
    ghostReplay: string; // Base64 encoded ghost replay data
}

async function fetchGhostReplay(id: number): Promise<Uint8Array> {
    const url = `${import.meta.env.VITE_SERVER_URL}/api/leaderboard/ghost/${id}`;
    const response = await fetch(url);
    if (!response.ok) {
        throw new Error(`Failed to fetch ghost replay: ${response.statusText}`);
    }
    const data: GhostReplayResponse = await response.json();

    // Decode base64 to Uint8Array
    const binaryString = atob(data.ghostReplay);
    const bytes = new Uint8Array(binaryString.length);
    for (let i = 0; i < binaryString.length; i++) {
        bytes[i] = binaryString.charCodeAt(i);
    }
    return bytes;
}

export function useGhostReplay(id: number | undefined) {
    return useQuery({
        queryKey: ['ghostReplay', id],
        queryFn: () => fetchGhostReplay(id!),
        enabled: Boolean(id),
    });
}
