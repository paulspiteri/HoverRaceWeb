import * as React from "react";
import { Button, Stack, Group } from "@mantine/core";
import { useAtomValue } from "jotai";
import { connectionIdAtom, commandsAtom } from "@/atoms.ts";

export const NoGame: React.FC = () => {
    const connectionId = useAtomValue(connectionIdAtom);
    const commands = useAtomValue(commandsAtom);
    return (
        <Stack gap="lg">
            <Group justify="center">
                <Button
                    onClick={() => {
                        const savedName = localStorage.getItem("hoverrace-player-name");
                        commands?.createGame(connectionId!, savedName || undefined);
                    }}
                    disabled={!connectionId || !commands}
                    size="lg"
                    px="xl"
                >
                    New Game
                </Button>
            </Group>
        </Stack>
    );
};
