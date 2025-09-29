import { useMutation } from "@tanstack/react-query";
import { useAtomValue } from "jotai";
import { commandsAtom, gameTokenAtom } from "@/atoms";
import type { UpdateGameRequest } from "@/types";

export const useUpdateGame = (gameId: string) => {
    const commands = useAtomValue(commandsAtom);
    const gameToken = useAtomValue(gameTokenAtom);

    return useMutation({
        mutationFn: async (updates: UpdateGameRequest) => {
            if (!commands) {
                throw new Error("Commands not available");
            }
            if (!gameToken) {
                throw new Error("Game token not available");
            }
            return commands.updateGame(gameId, gameToken, updates);
        },
    });
};