import { useState, useRef, useEffect } from "react";
import { TextInput, Stack, Text } from "@mantine/core";
import { useAtomValue } from "jotai";
import { canvasAtom, connectionIdAtom } from "./atoms";
import { getPlayerColor } from "./utils/chatColors";
import styles from "./FloatingChat.module.css";
import type { ChatMessage } from "./types";

interface FloatingChatProps {
    messages: ChatMessage[];
    onSendMessage: (message: string) => void;
}

export const FloatingChat: React.FC<FloatingChatProps> = ({ messages, onSendMessage }: FloatingChatProps) => {
    const [inputValue, setInputValue] = useState("");
    const inputRef = useRef<HTMLInputElement>(null);
    const scrollAreaRef = useRef<HTMLDivElement>(null);
    const canvasRef = useAtomValue(canvasAtom);
    const currentConnectionId = useAtomValue(connectionIdAtom);

    // Global keyboard handler for T/Enter to focus chat
    useEffect(() => {
        const handleGlobalKeyDown = (e: KeyboardEvent) => {
            if (document.activeElement === canvasRef?.current) {
                if (e.key === "t" || e.key === "T" || e.key === "Enter") {
                    e.preventDefault();
                    inputRef.current?.focus();
                }
            }
        };

        document.addEventListener("keydown", handleGlobalKeyDown);
        return () => document.removeEventListener("keydown", handleGlobalKeyDown);
    }, [canvasRef]);

    const returnFocusToCanvas = () => {
        if (canvasRef?.current) {
            inputRef.current?.blur();
            canvasRef.current.focus();
        }
    };

    useEffect(() => {
        if (scrollAreaRef.current) {
            scrollAreaRef.current.scrollTop = scrollAreaRef.current.scrollHeight;
        }
    }, [messages]);

    const handleSendMessage = () => {
        if (inputValue.trim()) {
            onSendMessage(inputValue.trim());
            setInputValue("");
            // Small delay to ensure the blur event completes before focusing canvas
            setTimeout(() => returnFocusToCanvas(), 0);
        }
    };

    const handleKeyDown = (e: React.KeyboardEvent) => {
        if (e.key === "Enter" && !e.shiftKey) {
            e.preventDefault();
            handleSendMessage();
        } else if (e.key === "Escape") {
            e.preventDefault();
            setInputValue("");
            returnFocusToCanvas();
        }
    };


    return (
        <div className={styles.chatContainer}>
            <div className={styles.chatWindow}>
                <div ref={scrollAreaRef} className={styles.messagesArea}>
                    <Stack gap="2px" p="sm">
                        {messages.map((msg) => {
                            const colors = getPlayerColor(msg.senderId, currentConnectionId);

                            return (
                                <div key={msg.id} className={styles.message}>
                                    <Text size="sm" className={styles.messageText}>
                                        <Text
                                            component="span"
                                            size="xs"
                                            c={colors.color}
                                            fw={700}
                                        >
                                            {msg.senderName || "Anonymous"}:
                                        </Text>{" "}
                                        {msg.message}
                                    </Text>
                                </div>
                            );
                        })}
                        {messages.length === 0 && (
                            <Text size="sm" c="dimmed" ta="center">
                                No messages
                            </Text>
                        )}
                    </Stack>
                </div>

                <div className={styles.inputForm}>
                    <TextInput
                        ref={inputRef}
                        value={inputValue}
                        onChange={(e) => setInputValue(e.currentTarget.value)}
                        onKeyDown={handleKeyDown}
                        onFocus={(e) => (e.currentTarget.placeholder = "Type a message...")}
                        onBlur={(e) => (e.currentTarget.placeholder = "Press T or Enter to chat...")}
                        placeholder="Press T or Enter to chat..."
                        className={styles.chatInput}
                        variant="filled"
                        size="sm"
                    />
                </div>
            </div>
        </div>
    );
};
