import * as React from "react";
import { Button, Stack, Group } from "@mantine/core";
import { useCreateGame } from "@/hooks/useCreateGame";

export const NoGame: React.FC = () => {
    const createGameMutation = useCreateGame();
    return (
        <Stack gap="lg">
            <Group justify="center">
                <Button
                    onClick={() => {
                        const savedName = localStorage.getItem("hoverrace-player-name");
                        createGameMutation.mutate(savedName || undefined);
                    }}
                    disabled={createGameMutation.isPending}
                    size="lg"
                    px="xl"
                >
                    New Game
                </Button>
            </Group>
        </Stack>
    );
};
