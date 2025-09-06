import * as React from "react";
import { Button, Stack, Group } from "@mantine/core";
import { useOutletContext } from "react-router-dom";
import type { GameOutletContext } from "./App";

export const NoGame: React.FC = () => {
    const { connectionId, commands } = useOutletContext<GameOutletContext>();
    return (
        <Stack gap="lg">
            <Group justify="center">
                <Button
                    onClick={() => {
                        const savedName = localStorage.getItem("hoverrace-player-name");
                        commands.createGame(connectionId!, savedName || undefined);
                    }}
                    disabled={!connectionId}
                    size="lg"
                    px="xl"
                >
                    New Game
                </Button>
            </Group>
        </Stack>
    );
};
