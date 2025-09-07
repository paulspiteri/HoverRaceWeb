import * as React from "react";
import { GameTile } from "./GameTile";
import type { Game } from "./types";
import { Title, Stack, Text, Box, Flex } from "@mantine/core";

interface GameListProps {
    games: Game[];
    onJoinGame: (gameId: string) => void;
}

export const GameList: React.FC<GameListProps> = ({ games, onJoinGame }) => {
    return (
        <Flex h="100%" direction="column" style={{ minHeight: 0 }}>
            <Title order={3} mb="md">
                Games ({games.length})
            </Title>
            <Box flex={1} style={{ overflowY: "auto" }}>
                <Stack gap="md">
                    {games.length === 0 ? (
                        <Text c="dimmed" ta="center" py="xl">
                            No games available
                        </Text>
                    ) : (
                        games.map((game) => (
                            <GameTile key={game.id} game={game} isJoined={"players" in game} onJoinGame={onJoinGame} />
                        ))
                    )}
                </Stack>
            </Box>
        </Flex>
    );
};
