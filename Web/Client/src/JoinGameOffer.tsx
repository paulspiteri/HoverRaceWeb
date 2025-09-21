import * as React from "react";
import { Button, Stack, Title, Text, Card } from "@mantine/core";
import { useParams } from "react-router-dom";
import type { Game } from "./types";
import { useJoinGame } from "@/hooks/useJoinGame";
import { notifications } from "@mantine/notifications";
import styles from "./JoinGameOffer.module.css";

interface JoinGameInterfaceProps {
    gameInfo: Game;
}

export const JoinGameOffer: React.FC<JoinGameInterfaceProps> = ({ gameInfo }) => {
    const { gameId } = useParams();
    const joinGameMutation = useJoinGame();

    const handleJoinGame = () => {
        if (!gameId) return;
        const savedName = localStorage.getItem("hoverrace-player-name");
        joinGameMutation.mutate(
            { gameId, name: savedName || undefined },
            {
                onError: (error) => {
                    console.error("Failed to join game:", error);
                    notifications.show({
                        title: "Failed to join game",
                        message: "Unable to join the game. Please try again.",
                        color: "red",
                    });
                },
            }
        );
    };

    return (
        <div className={styles.container}>
            <div className={styles.content}>
                <Card withBorder shadow="sm" radius="md" p="lg" className={styles.gameInfo}>
                    <Stack gap="md" align="center">
                        <Title order={2}>Join Game</Title>
                        <Text size="lg" fw={500}>
                            {gameInfo.name}
                        </Text>
                        <Text size="sm" c="dimmed" ta="center">
                            ID: {gameInfo.id}
                        </Text>
                        <Text size="sm" c="dimmed" ta="center">
                            Players: {gameInfo.playerCount}/{gameInfo.maxPlayers} • Status: {gameInfo.status}
                        </Text>

                        {gameInfo.status === "playing" && (
                            <Text size="sm" c="dimmed" ta="center">
                                This game has already started and cannot be joined.
                            </Text>
                        )}

                        {gameInfo.playerCount >= gameInfo.maxPlayers && gameInfo.status !== "playing" && (
                            <Text size="sm" c="dimmed" ta="center">
                                This game is full and cannot be joined.
                            </Text>
                        )}
                    </Stack>
                </Card>

                <Button
                    className={styles.joinButton}
                    onClick={handleJoinGame}
                    disabled={gameInfo.status === "playing" || gameInfo.playerCount >= gameInfo.maxPlayers || joinGameMutation.isPending}
                >
                    Join Game
                </Button>
            </div>
        </div>
    );
};
