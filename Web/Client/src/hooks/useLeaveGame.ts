import { useMutation } from "@tanstack/react-query";
import { useAtomValue } from "jotai";
import { commandsAtom } from "@/atoms";

export const useLeaveGame = () => {
    const commands = useAtomValue(commandsAtom);

    return useMutation({
        mutationFn: async ({ gameId, gameToken }: { gameId: string; gameToken: string }) => {
            if (!commands) {
                throw new Error("Commands not available");
            }
            return commands.leaveGame(gameId, gameToken);
        },
    });
};