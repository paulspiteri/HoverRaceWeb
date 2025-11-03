import * as React from "react";
import { Button, Box } from "@mantine/core";
import { useCreateGame } from "@/hooks/useCreateGame";
import { notifications } from "@mantine/notifications";
import { useAtomValue } from "jotai";
import { connectionIdAtom } from "@/atoms";
import styles from "./NoGame.module.css";

export const NoGame: React.FC = () => {
    const createGameMutation = useCreateGame();
    const connectionId = useAtomValue(connectionIdAtom);

    const handleCreateGame = () => {
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
    };

    return (
        <Box className={styles.container}>
            <Button
                onClick={handleCreateGame}
                disabled={createGameMutation.isPending || !connectionId}
                size="lg"
                px="xl"
                className={styles.newGameButton}
            >
                New Game
            </Button>
        </Box>
    );
};
