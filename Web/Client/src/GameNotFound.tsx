import * as React from "react";
import { Button, Container, Stack, Card, Alert } from "@mantine/core";
import { useCreateGame } from "@/hooks/useCreateGame";

export const GameNotFound: React.FC = () => {
    const createGameMutation = useCreateGame();

    return (
        <Container size="sm" mt="xl">
            <Card withBorder shadow="sm" radius="md" p="lg">
                <Stack gap="md" align="center">
                    <Alert color="yellow" title="Game not found">
                        The game you&apos;re looking for doesn&apos;t exist or is no longer available.
                    </Alert>
                    <Button
                        onClick={() => {
                            const savedName = localStorage.getItem("hoverrace-player-name");
                            createGameMutation.mutate(savedName || undefined);
                        }}
                        disabled={createGameMutation.isPending}
                        variant="filled"
                        color="blue"
                    >
                        New Game
                    </Button>
                </Stack>
            </Card>
        </Container>
    );
};
