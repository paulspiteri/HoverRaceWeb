import * as React from "react";
import { Button, Modal, Select, NumberInput, Checkbox, Stack, Group } from "@mantine/core";
import { notifications } from "@mantine/notifications";
import { useUpdateGame } from "@/hooks/useUpdateGame";
import type { JoinedGame } from "./types";

const trackOptions = [
    { value: "ClassicH.trk", label: "ClassicH" },
    { value: "Steeplechase.trk", label: "Steeplechase" },
    { value: "The Alley2.trk", label: "The Alley2" },
    { value: "The River.trk", label: "The River" },
];

interface GameSettingsModalProps {
    game: JoinedGame;
    opened: boolean;
    onClose: () => void;
}

export const GameSettingsModal: React.FC<GameSettingsModalProps> = ({
    game,
    opened,
    onClose,
}) => {
    const updateGameMutation = useUpdateGame(game.id);

    const [settingsForm, setSettingsForm] = React.useState({
        trackName: game.trackName || "Steeplechase.trk",
        hasWeapons: game.hasWeapons ?? true,
        laps: game.laps || 5,
    });

    const handleSettingsSubmit = async (e: React.FormEvent) => {
        e.preventDefault();
        try {
            const updates: Partial<{ trackName: string; hasWeapons: boolean; laps: number }> = {};
            if (settingsForm.trackName !== game.trackName) {
                updates.trackName = settingsForm.trackName;
            }
            if (settingsForm.hasWeapons !== game.hasWeapons) {
                updates.hasWeapons = settingsForm.hasWeapons;
            }
            if (settingsForm.laps !== game.laps) {
                updates.laps = settingsForm.laps;
            }

            if (Object.keys(updates).length > 0) {
                await updateGameMutation.mutateAsync(updates);
                notifications.show({
                    title: "Settings updated",
                    message: "Game settings have been updated successfully.",
                    color: "green",
                });
            }
            onClose();
        } catch (error) {
            console.error("Failed to update game settings:", error);
            notifications.show({
                title: "Failed to update settings",
                message: "Unable to update game settings. Please try again.",
                color: "red",
            });
        }
    };

    return (
        <Modal
            opened={opened}
            onClose={onClose}
            title="Game Settings"
            size="md"
        >
            <form onSubmit={handleSettingsSubmit}>
                <Stack gap="md">
                    <Select
                        label="Track"
                        placeholder="Select a track"
                        value={settingsForm.trackName}
                        onChange={(value) => setSettingsForm(prev => ({ ...prev, trackName: value || "Steeplechase.trk" }))}
                        data={trackOptions}
                        required
                    />

                    <Checkbox
                        label="Enable Weapons"
                        checked={settingsForm.hasWeapons}
                        onChange={(e) => setSettingsForm(prev => ({ ...prev, hasWeapons: e.target.checked }))}
                    />

                    <NumberInput
                        label="Number of Laps"
                        placeholder="5"
                        value={settingsForm.laps}
                        onChange={(value) => setSettingsForm(prev => ({ ...prev, laps: Number(value) || 1 }))}
                        min={1}
                        max={99}
                        required
                    />

                    <Group justify="flex-end" gap="sm">
                        <Button
                            variant="subtle"
                            onClick={onClose}
                            disabled={updateGameMutation.isPending}
                        >
                            Cancel
                        </Button>
                        <Button
                            type="submit"
                            loading={updateGameMutation.isPending}
                        >
                            Save Settings
                        </Button>
                    </Group>
                </Stack>
            </form>
        </Modal>
    );
};