import * as React from "react";
import { useState, useRef, useEffect } from "react";
import { TextInput, ActionIcon, ScrollArea, Text, Group, Stack, Paper, Box } from "@mantine/core";
import { IconSend } from "@tabler/icons-react";
import { useAtomValue } from "jotai";
import { connectionIdAtom } from "./atoms";
import { getPlayerColor, formatTimestamp } from "./utils/chatColors";
import type { ChatMessage } from "./types";
import styles from "./Chat.module.css";

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


    return (
        <Box className={styles.container}>
            {/* Messages area */}
            <ScrollArea
                className={styles.messagesArea}
                viewportRef={scrollAreaRef}
                type="auto"
                styles={{
                    viewport: {
                        paddingBottom: 0,
                        paddingRight: "8px",
                        ...(messages.length === 0 && {
                            display: "flex",
                            alignItems: "center",
                            justifyContent: "center",
                        }),
                    },
                }}
            >
                {messages.length === 0 ? (
                    <Text size="sm" c="dimmed" ta="center">
                        No messages
                    </Text>
                ) : (
                    <Stack gap="xs" className={styles.messagesStack}>
                        {messages.map((message) => {
                            const colors = getPlayerColor(message.senderId, currentConnectionId);
                            return (
                                <Paper
                                    key={message.id}
                                    p="xs"
                                    radius="sm"
                                    bg={colors.color}
                                    className={`${styles.message} ${
                                        message.senderId === currentConnectionId ? styles.messageOwn : styles.messageOther
                                    }`}
                                >
                                    <Group gap="xs" justify="space-between">
                                        <Text size="xs" c={colors.text} fw={500} opacity={0.8}>
                                            {message.senderId === currentConnectionId
                                                ? "You"
                                                : message.senderName || message.senderId}
                                        </Text>
                                        <Text size="xs" c={colors.text} opacity={0.6}>
                                            {formatTimestamp(message.timestamp)}
                                        </Text>
                                    </Group>
                                    <Text size="sm" mt="2px" c={colors.text}>
                                        {message.message}
                                    </Text>
                                </Paper>
                            );
                        })}
                    </Stack>
                )}
            </ScrollArea>

            {/* Message input - fixed at bottom */}
            <Group gap="xs" className={styles.inputGroup}>
                <TextInput
                    className={styles.textInput}
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
        </Box>
    );
};
