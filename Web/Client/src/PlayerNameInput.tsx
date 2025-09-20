import * as React from "react";
import { Button, TextInput, Group } from "@mantine/core";
import { useState, useEffect } from "react";
import { useUpdatePlayer } from "@/hooks/useUpdatePlayer";

interface PlayerNameInputProps {
    currentPlayerName: string;
    gameId: string;
    disabled?: boolean;
}

export const PlayerNameInput: React.FC<PlayerNameInputProps> = ({
    currentPlayerName,
    gameId,
    disabled = false,
}) => {
    // Player name state and localStorage persistence
    const [playerName, setPlayerName] = useState(() => localStorage.getItem("hoverrace-player-name") ?? "");
    const updatePlayerMutation = useUpdatePlayer(gameId);

    useEffect(() => {
        if (playerName) {
            localStorage.setItem("hoverrace-player-name", playerName);
        }
    }, [playerName]);

    // Update server when name changes and is different from current
    const handleUpdateName = async () => {
        if (!playerName.trim() || playerName === currentPlayerName || updatePlayerMutation.isPending || disabled) {
            return;
        }

        try {
            await updatePlayerMutation.mutateAsync(playerName.trim());
        } catch (error) {
            console.error("Failed to update player name:", error);
        }
    };

    return (
        <Group gap="xs">
            <TextInput
                placeholder="Enter your name..."
                value={playerName}
                onChange={(e) => setPlayerName(e.currentTarget.value)}
                onBlur={handleUpdateName}
                onKeyDown={(e) => {
                    if (e.key === "Enter") {
                        handleUpdateName();
                        e.currentTarget.blur();
                    }
                }}
                disabled={updatePlayerMutation.isPending || disabled}
                style={{ flex: 1 }}
            />
            <Button
                onClick={handleUpdateName}
                disabled={!playerName.trim() || playerName === currentPlayerName || updatePlayerMutation.isPending || disabled}
                variant="outline"
                size="sm"
            >
                {updatePlayerMutation.isPending ? "Updating..." : "Update"}
            </Button>
        </Group>
    );
};
