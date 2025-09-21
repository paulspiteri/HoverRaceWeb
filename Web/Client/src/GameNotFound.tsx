import * as React from "react";
import { Button, Alert, Card } from "@mantine/core";
import { useCreateGame } from "@/hooks/useCreateGame";
import { notifications } from "@mantine/notifications";
import styles from "./GameNotFound.module.css";

export const GameNotFound: React.FC = () => {
    const createGameMutation = useCreateGame();

    return (
        <div className={styles.container}>
            <div className={styles.content}>
                <Card withBorder shadow="sm" radius="md" className={styles.card}>
                    <Alert color="yellow" title="Game not found" className={styles.alert}>
                        The game you&apos;re looking for doesn&apos;t exist or is no longer available.
                    </Alert>
                </Card>
                <Button
                    className={styles.newGameButton}
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
                    variant="filled"
                    color="blue"
                >
                    New Game
                </Button>
            </div>
        </div>
    );
};
