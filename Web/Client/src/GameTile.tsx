import * as React from "react";
import { Button, Box, Text, Title, Stack } from "@mantine/core";
import type { Game } from "@/types.ts";
import { useJoinGame } from "@/hooks/useJoinGame";
import { notifications } from "@mantine/notifications";

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
        <Box
            p="md"
            className="game-tile-hover"
            style={{
                border: "2px solid var(--mantine-color-gray-3)",
                borderRadius: 8,
                transition: "border-color 0.2s ease",
                cursor: "pointer",
            }}
        >
            <Stack gap="md">
                <Box>
                    <Title order={4} size="lg" fw={600}>
                        {game.name}
                    </Title>
                    <Text size="sm" c="dimmed" style={{ whiteSpace: 'nowrap' }}>
                        Players: {game.playerCount}/{game.maxPlayers}
                    </Text>
                    <Text size="xs" c="dimmed" style={{ whiteSpace: 'nowrap' }}>
                        Created: {new Date(game.createdAt).toLocaleString()}
                    </Text>
                    <Text size="xs" c="dimmed" style={{ whiteSpace: 'nowrap', overflow: 'hidden', textOverflow: 'ellipsis' }}>
                        Game ID: {game.id}
                    </Text>
                </Box>

                <Button
                    variant="outline"
                    size="sm"
                    onClick={handleJoinGame}
                    disabled={isJoined || isGameFull || isGameStarted || disabled || joinGameMutation.isPending}
                    fullWidth={true}
                >
                    {isGameStarted ? "Game Started" : isGameFull ? "Game Full" : "Join Game"}
                </Button>
            </Stack>
        </Box>
    );
};
