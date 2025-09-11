import * as React from "react";
import { Button, Container, Stack, Card, Alert } from "@mantine/core";
import { useAtomValue } from "jotai";
import { connectionIdAtom, commandsAtom } from "@/atoms.ts";

export const GameNotFound: React.FC = () => {
    const connectionId = useAtomValue(connectionIdAtom);
    const commands = useAtomValue(commandsAtom);

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
                            commands?.createGame(connectionId!, savedName || undefined);
                        }}
                        disabled={!connectionId || !commands}
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
