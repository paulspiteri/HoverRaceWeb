import { useMutation } from "@tanstack/react-query";
import { useAtomValue } from "jotai";
import { commandsAtom, connectionIdAtom } from "@/atoms";

export const useJoinGame = () => {
    const commands = useAtomValue(commandsAtom);
    const connectionId = useAtomValue(connectionIdAtom);

    return useMutation({
        mutationFn: async ({ gameId, name }: { gameId: string; name?: string }) => {
            if (!connectionId) {
                throw new Error("No connection ID available");
            }
            if (!commands) {
                throw new Error("Commands not available");
            }
            return commands.joinGame(gameId, connectionId, name);
        },
    });
};