import * as React from "react";
import { Button, Box, Text, Title, Stack } from "@mantine/core";
import type { Game } from "@/types.ts";
import { useJoinGame } from "@/hooks/useJoinGame";
import { notifications } from "@mantine/notifications";
import styles from "./GameTile.module.css";

interface GameTileProps {
    game: Game;
    isJoined: boolean;
    disabled?: boolean;
}

export const GameTile: React.FC<GameTileProps> = ({ game, isJoined, disabled }) => {
    const isGameFull = game.playerCount >= game.maxPlayers;
    const isGameStarted = game.status === "playing";
    const joinGameMutation = useJoinGame();

    const handleJoinGame = () => {
        const savedName = localStorage.getItem("hoverrace-player-name");
        joinGameMutation.mutate(
            { gameId: game.id, name: savedName || undefined },
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
        <Box className={styles.gameTile}>
            <Stack className={styles.gameTileContent}>
                <Box>
                    <Title order={4} size="lg" fw={600} className={styles.gameTitle}>
                        {game.name}
                    </Title>
                    <Text size="sm" c="dimmed" className={styles.gameInfo}>
                        Players: {game.playerCount}/{game.maxPlayers}
                    </Text>
                    <Text size="xs" c="dimmed" className={styles.gameInfo}>
                        Created: {new Date(game.createdAt).toLocaleDateString()}
                    </Text>
                    <Text size="xs" c="dimmed" className={styles.gameIdShort}>
                        ID: {game.id.slice(0, 8)}...
                    </Text>
                    <Text size="xs" c="dimmed" className={styles.gameIdFull}>
                        Game ID: {game.id}
                    </Text>
                </Box>

                <Button
                    variant="outline"
                    size="md"
                    onClick={handleJoinGame}
                    disabled={isJoined || isGameFull || isGameStarted || disabled || joinGameMutation.isPending}
                    fullWidth={true}
                    className={styles.joinButton}
                >
                    {isGameStarted ? "Game Started" : isGameFull ? "Game Full" : "Join Game"}
                </Button>
            </Stack>
        </Box>
    );
};
