import * as React from "react";
import { useState, useRef, useEffect } from "react";
import { TextInput, ActionIcon, ScrollArea, Text, Group, Stack, Paper, Box } from "@mantine/core";
import { IconSend } from "@tabler/icons-react";
import { useAtomValue } from "jotai";
import { connectionIdAtom } from "./atoms";
import type { ChatMessage } from "./types";

interface ChatProps {
    messages: ChatMessage[];
    onSendMessage: (message: string) => void;
}

export const Chat: React.FC<ChatProps> = ({ messages, onSendMessage }) => {
    const [newMessage, setNewMessage] = useState("");
    const scrollAreaRef = useRef<HTMLDivElement>(null);
    const currentConnectionId = useAtomValue(connectionIdAtom);

    // Auto-scroll to bottom when new messages arrive
    useEffect(() => {
        if (scrollAreaRef.current) {
            // Use setTimeout to ensure DOM has updated
            setTimeout(() => {
                if (scrollAreaRef.current) {
                    scrollAreaRef.current.scrollTo({
                        top: scrollAreaRef.current.scrollHeight,
                        behavior: "smooth",
                    });
                }
            }, 0);
        }
    }, [messages]);

    const handleSendMessage = () => {
        const trimmedMessage = newMessage.trim();
        if (trimmedMessage) {
            onSendMessage(trimmedMessage);
            setNewMessage("");
        }
    };

    const handleKeyPress = (event: React.KeyboardEvent) => {
        if (event.key === "Enter" && !event.shiftKey) {
            event.preventDefault();
            handleSendMessage();
        }
    };

    const formatTimestamp = (timestamp: Date) => {
        return new Date(timestamp).toLocaleTimeString([], {
            hour: "2-digit",
            minute: "2-digit",
        });
    };

    const getPlayerColor = (senderId: string) => {
        // If it's the current user, use blue
        if (senderId === currentConnectionId) {
            return "blue.0";
        }

        // Generate a consistent color based on the senderId
        const colors = ["green.0", "red.0", "yellow.0", "purple.0", "orange.0", "pink.0", "indigo.0", "teal.0"];

        // Use a simple hash to get consistent color for same senderId
        let hash = 0;
        for (let i = 0; i < senderId.length; i++) {
            hash = ((hash << 5) - hash + senderId.charCodeAt(i)) & 0xffffffff;
        }
        return colors[Math.abs(hash) % colors.length];
    };

    return (
        <Box
            h="100%"
            p="sm"
            style={{
                border: "1px solid var(--mantine-color-gray-3)",
                borderRadius: "var(--mantine-radius-md)",
                minHeight: 0,
            }}
        >
            <Stack gap="xs" h="100%" style={{ minHeight: 0 }}>
                {/* Messages area */}
                <ScrollArea
                    flex={1}
                    viewportRef={scrollAreaRef}
                    style={{ minHeight: 0, maxHeight: "100%" }}
                    type="auto"
                    styles={{
                        viewport: {
                            paddingBottom: 0,
                            paddingRight: "8px",
                            display: messages.length === 0 ? "flex" : "block",
                            alignItems: messages.length === 0 ? "center" : "unset",
                            justifyContent: messages.length === 0 ? "center" : "unset",
                        },
                    }}
                >
                    {messages.length === 0 ? (
                        <Text size="sm" c="dimmed" ta="center">
                            No messages
                        </Text>
                    ) : (
                        <Stack gap="xs" p="xs">
                            {messages.map((message) => (
                                <Paper
                                    key={message.id}
                                    p="xs"
                                    radius="sm"
                                    bg={getPlayerColor(message.senderId)}
                                    styles={{
                                        root: {
                                            alignSelf:
                                                message.senderId === currentConnectionId ? "flex-end" : "flex-start",
                                            maxWidth: "80%",
                                        },
                                    }}
                                >
                                    <Group gap="xs" justify="space-between">
                                        <Text size="xs" c="dimmed" fw={500}>
                                            {message.senderId === currentConnectionId
                                                ? "You"
                                                : message.senderName || message.senderId}
                                        </Text>
                                        <Text size="xs" c="dimmed">
                                            {formatTimestamp(message.timestamp)}
                                        </Text>
                                    </Group>
                                    <Text size="sm" mt="2px">
                                        {message.message}
                                    </Text>
                                </Paper>
                            ))}
                        </Stack>
                    )}
                </ScrollArea>

                {/* Message input */}
                <Group gap="xs">
                    <TextInput
                        flex={1}
                        placeholder="Type a message..."
                        value={newMessage}
                        onChange={(e) => setNewMessage(e.target.value)}
                        onKeyDown={handleKeyPress}
                        size="sm"
                    />
                    <ActionIcon onClick={handleSendMessage} disabled={!newMessage.trim()} size="lg" variant="filled">
                        <IconSend size={16} />
                    </ActionIcon>
                </Group>
            </Stack>
        </Box>
    );
};
