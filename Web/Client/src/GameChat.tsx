import * as React from "react";
import { useState, useEffect } from "react";
import { useAtomValue } from "jotai";
import { useParams } from "react-router-dom";
import { eventSourceAtom, gameTokenAtom, commandsAtom } from "./atoms";
import { Chat } from "./Chat";
import type { ServerMessage, ChatMessage } from "./types";

export const GameChat: React.FC = () => {
    const { gameId } = useParams();
    const eventSource = useAtomValue(eventSourceAtom);
    const gameToken = useAtomValue(gameTokenAtom);
    const commands = useAtomValue(commandsAtom);
    const [chatMessages, setChatMessages] = useState<ChatMessage[]>([]);

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

    return <Chat messages={chatMessages} onSendMessage={handleSendChatMessage} />;
};
