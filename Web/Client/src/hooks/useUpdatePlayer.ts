import { useMutation } from "@tanstack/react-query";
import { useAtomValue } from "jotai";
import { commandsAtom, gameTokenAtom } from "@/atoms";

export const useUpdatePlayer = (gameId: string) => {
    const commands = useAtomValue(commandsAtom);
    const gameToken = useAtomValue(gameTokenAtom);

    return useMutation({
        mutationFn: async (name: string) => {
            if (!commands) {
                throw new Error("Commands not available");
            }
            if (!gameToken) {
                throw new Error("Game token not available");
            }
            return commands.updatePlayer(gameId, gameToken, name);
        },
    });
};