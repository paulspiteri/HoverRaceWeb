import * as React from "react";
import { Button, Container, Stack, Card, Alert } from "@mantine/core";
import { useNavigate } from "react-router-dom";

export const GameNotFound: React.FC = () => {
    const navigate = useNavigate();
    
    return (
        <Container size="sm" mt="xl">
            <Card withBorder shadow="sm" radius="md" p="lg">
                <Stack gap="md" align="center">
                    <Alert color="yellow" title="Game not found">
                        The game you&apos;re looking for doesn&apos;t exist or is no longer available.
                    </Alert>
                    <Button onClick={() => navigate("/")}>
                        Back to Home
                    </Button>
                </Stack>
            </Card>
        </Container>
    );
};