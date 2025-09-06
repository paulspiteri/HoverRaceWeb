import * as React from "react";
import { Button, TextInput, Group, Box } from "@mantine/core";
import { useState, useEffect } from "react";

interface PlayerNameInputProps {
    currentPlayerName: string;
    onUpdatePlayer: (name: string) => Promise<void>;
    disabled?: boolean;
}

export const PlayerNameInput: React.FC<PlayerNameInputProps> = ({
    currentPlayerName,
    onUpdatePlayer,
    disabled = false,
}) => {
    // Player name state and localStorage persistence
    const [playerName, setPlayerName] = useState(() => localStorage.getItem("hoverrace-player-name") ?? "");
    const [isUpdatingName, setIsUpdatingName] = useState(false);

    useEffect(() => {
        if (playerName) {
            localStorage.setItem("hoverrace-player-name", playerName);
        }
    }, [playerName]);

    // Update server when name changes and is different from current
    const handleUpdateName = async () => {
        if (!playerName.trim() || playerName === currentPlayerName || isUpdatingName || disabled) {
            return;
        }

        setIsUpdatingName(true);
        try {
            await onUpdatePlayer(playerName.trim());
        } catch (error) {
            console.error("Failed to update player name:", error);
        } finally {
            setIsUpdatingName(false);
        }
    };

    return (
        <Box>
            <Group gap="xs">
                <TextInput
                    label="Your Name"
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
                    disabled={isUpdatingName || disabled}
                    style={{ flex: 1 }}
                />
                <Button
                    onClick={handleUpdateName}
                    disabled={!playerName.trim() || playerName === currentPlayerName || isUpdatingName || disabled}
                    variant="outline"
                    size="sm"
                    mt="xl"
                >
                    {isUpdatingName ? "Updating..." : "Update"}
                </Button>
            </Group>
        </Box>
    );
};
