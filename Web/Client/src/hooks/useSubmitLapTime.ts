import { useMutation } from '@tanstack/react-query';
import type { SubmitLapTimeRequest, SubmitLapTimeResult } from '../types';

export function useSubmitLapTime() {
    return useMutation({
        mutationFn: async (request: SubmitLapTimeRequest): Promise<SubmitLapTimeResult> => {
            const url = `${import.meta.env.VITE_SERVER_URL}/api/leaderboard/submit`;
            const response = await fetch(url, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify(request),
            });

            if (!response.ok) {
                throw new Error(`Failed to submit lap time: ${response.statusText}`);
            }

            return response.json();
        }
    });
}
