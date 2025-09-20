import * as React from "react";
import { Button, Stack, Group } from "@mantine/core";
import { useCreateGame } from "@/hooks/useCreateGame";
import { notifications } from "@mantine/notifications";

export const NoGame: React.FC = () => {
    const createGameMutation = useCreateGame();
    return (
        <Stack gap="lg">
            <Group justify="center">
                <Button
                    onClick={() => {
                        const savedName = localStorage.getItem("hoverrace-player-name");
                        createGameMutation.mutate(savedName || undefined, {
                            onError: (error) => {
                                console.error("Failed to create game:", error);
                                notifications.show({
                                    title: "Failed to create game",
                                    message: "Unable to create a new game. Please try again.",
                                    color: "red",
                                });
                            },
                        });
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
