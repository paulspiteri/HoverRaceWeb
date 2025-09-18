import * as React from "react";
import { useState, useEffect } from "react";
import { useAtomValue } from "jotai";
import { useParams } from "react-router-dom";
import { eventSourceAtom, gameTokenAtom, commandsAtom, gameScreenModeAtom } from "./atoms";
import { Chat } from "./Chat";
import { FloatingChat } from "./FloatingChat";
import type { ServerMessage, ChatMessage } from "./types";

export const GameChat: React.FC = () => {
    const { gameId } = useParams();
    const eventSource = useAtomValue(eventSourceAtom);
    const gameToken = useAtomValue(gameTokenAtom);
    const commands = useAtomValue(commandsAtom);
    const gameScreenMode = useAtomValue(gameScreenModeAtom);
    const [chatMessages, setChatMessages] = useState<ChatMessage[]>([]);

    useEffect(() => {
        const loadChatHistory = async () => {
            if (gameId && gameToken && commands) {
                const messages = await commands.getChatHistory(gameId, gameToken);
                setChatMessages((prev) => {
                    // Merge history with any new messages that arrived while loading
                    const historyIds = new Set(messages.map((m) => m.id));
                    const newMessages = prev.filter((m) => !historyIds.has(m.id));
                    return [...messages, ...newMessages];
                });
            }
        };

        loadChatHistory();
    }, [gameId, gameToken, commands]);

    useEffect(() => {
        if (!eventSource) return;

        const handleMessage = (event: MessageEvent) => {
            try {
                const data = JSON.parse(event.data) as ServerMessage;
                if (data.type === "chatMessage" && data.chatMessage.gameId === gameId) {
                    setChatMessages((prev) => [...prev, data.chatMessage]);
                }
            } catch (error) {
                console.error("Error parsing SSE message in GameChat:", error);
            }
        };

        eventSource.addEventListener("message", handleMessage);
        return () => eventSource.removeEventListener("message", handleMessage);
    }, [eventSource, gameId]);

    const handleSendChatMessage = async (message: string) => {
        if (gameId && gameToken && commands) {
            await commands.sendChatMessage(gameId, gameToken, message);
        }
    };

    // Render floating chat if maximized, otherwise normal chat
    if (gameScreenMode === "maximized") {
        return <FloatingChat messages={chatMessages} onSendMessage={handleSendChatMessage} />;
    }

    return <Chat messages={chatMessages} onSendMessage={handleSendChatMessage} />;
};
