import * as React from "react";
import { Button, Container, Stack, Title, Group, Card, Text } from "@mantine/core";
import { useNavigate, useParams } from "react-router-dom";
import type { Game } from "./types";
import { useJoinGame } from "@/hooks/useJoinGame";

interface JoinGameInterfaceProps {
    gameInfo: Game;
}

export const JoinGameOffer: React.FC<JoinGameInterfaceProps> = ({ gameInfo }) => {
    const navigate = useNavigate();
    const { gameId } = useParams();
    const joinGameMutation = useJoinGame();

    const handleJoinGame = () => {
        if (!gameId) return;
        const savedName = localStorage.getItem("hoverrace-player-name");
        joinGameMutation.mutate({ gameId, name: savedName || undefined });
    };

    return (
        <Container size="sm" mt="xl">
            <Card withBorder shadow="sm" radius="md" p="lg">
                <Stack gap="md" align="center">
                    <Title order={2}>Join Game</Title>
                    <Text size="lg" fw={500}>
                        {gameInfo.name}
                    </Text>
                    <Text size="sm" c="dimmed">
                        Players: {gameInfo.playerCount}/{gameInfo.maxPlayers} • Status: {gameInfo.status} • ID:{" "}
                        {gameInfo.id}
                    </Text>

                    <Group gap="md">
                        <Button variant="outline" onClick={() => navigate("/")}>
                            Back to Home
                        </Button>
                        <Button
                            onClick={handleJoinGame}
                            disabled={gameInfo.status === "playing" || gameInfo.playerCount >= gameInfo.maxPlayers || joinGameMutation.isPending}
                        >
                            Join Game
                        </Button>
                    </Group>

                    {gameInfo.status === "playing" && (
                        <Text size="sm" c="dimmed">
                            This game has already started and cannot be joined.
                        </Text>
                    )}

                    {gameInfo.playerCount >= gameInfo.maxPlayers && gameInfo.status !== "playing" && (
                        <Text size="sm" c="dimmed">
                            This game is full and cannot be joined.
                        </Text>
                    )}
                </Stack>
            </Card>
        </Container>
    );
};
