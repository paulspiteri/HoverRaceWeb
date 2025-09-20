import { useMutation } from "@tanstack/react-query";
import { useAtomValue } from "jotai";
import { commandsAtom, gameTokenAtom } from "@/atoms";

export const useStartGame = (gameId: string) => {
    const commands = useAtomValue(commandsAtom);
    const gameToken = useAtomValue(gameTokenAtom);

    return useMutation({
        mutationFn: async () => {
            if (!commands) {
                throw new Error("Commands not available");
            }
            if (!gameToken) {
                throw new Error("Game token not available");
            }
            return commands.startGame(gameId, gameToken);
        },
    });
};