import * as React from "react";
import { Container, Stack, Title, Text, Paper, Anchor} from "@mantine/core";
import styles from "./About.module.css";

export const About: React.FC = () => {
    return (
        <Container size="md" className={styles.container}>
            <Stack gap="lg">
                <Paper shadow="sm" p="xl" withBorder>
                    <Stack gap="md">
                        <Title order={2} mt="md">About the Project</Title>
                        <Text>
                            HoverRace Web is a browser-based version of the classic HoverRace game, modernized with WebGL rendering,  WebRTC for peer-to-peer multiplayer, and a React user interface.

                        </Text>

                        <Text mt="sm">
                            This web version was forked from the{" "}
                            <Anchor href="https://github.com/HoverRace/HoverRace/commit/0f06d7a96fe31581de0242b00813925e32d72008" target="_blank">
                                original source code
                            </Anchor>
                            {" "}of the Windows 95 version, which was created by Richard Langlois (Grokksoft).
                        </Text>

                        <Text size="sm" c="dimmed" mt="md">
                            View the source code on{" "}
                            <Anchor href="https://github.com/paulspiteri/HoverRaceWeb" target="_blank">
                                GitHub
                            </Anchor>
                        </Text>
                    </Stack>
                </Paper>
            </Stack>
        </Container>
    );
};
