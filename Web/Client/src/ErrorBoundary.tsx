import React from "react";
import { Container, Title, Text, Button, Stack, Box } from "@mantine/core";
import { useRouteError, isRouteErrorResponse } from "react-router-dom";

export const ErrorBoundary: React.FC = () => {
    const error = useRouteError();

    let errorMessage: string;
    let errorDetails: string | null = null;

    if (isRouteErrorResponse(error)) {
        errorMessage = `${error.status} ${error.statusText}`;
        errorDetails = error.data?.message || null;
    } else if (error instanceof Error) {
        errorMessage = error.message;
        errorDetails = error.stack || null;
    } else if (typeof error === 'string') {
        errorMessage = error;
    } else {
        errorMessage = "An unexpected error occurred";
    }

    const handleReload = () => {
        window.location.reload();
    };

    const handleGoHome = () => {
        window.location.href = "/";
    };

    return (
        <Container h="100vh" style={{ display: "flex", alignItems: "center", justifyContent: "center" }}>
            <Box maw={500} w="100%">
                <Stack gap="lg" ta="center">
                    <Title order={1} c="red">
                        Something went wrong
                    </Title>

                    <Text size="lg" c="dimmed">
                        {errorMessage}
                    </Text>

                    {errorDetails && (
                        <Text size="sm" c="dimmed" ta="left" style={{
                            fontFamily: "monospace",
                            backgroundColor: "var(--mantine-color-gray-1)",
                            padding: "var(--mantine-spacing-sm)",
                            borderRadius: "var(--mantine-radius-sm)",
                            whiteSpace: "pre-wrap",
                            overflow: "auto",
                            maxHeight: "200px"
                        }}>
                            {errorDetails}
                        </Text>
                    )}

                    <Stack gap="sm">
                        <Button onClick={handleReload} variant="filled">
                            Reload Page
                        </Button>
                        <Button onClick={handleGoHome} variant="outline">
                            Go to Home
                        </Button>
                    </Stack>
                </Stack>
            </Box>
        </Container>
    );
};