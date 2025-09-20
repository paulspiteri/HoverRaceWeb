import * as React from "react";
import { GameTile } from "./GameTile";
import type { Game } from "./types";
import { Title, Stack, Text, Box, Flex } from "@mantine/core";
import styles from "./GameList.module.css";

interface GameListProps {
    games: Game[];
}

export const GameList: React.FC<GameListProps> = ({ games }) => {
    return (
        <Flex h="100%" direction="column" style={{ minHeight: 0 }}>
            <Title order={3} mb="md" visibleFrom="sm">
                Games ({games.length})
            </Title>
            <Box flex={1} style={{ overflowY: "auto" }}>
                {games.length === 0 ? (
                    <Flex h="100%" align="center" justify="center">
                        <Text c="dimmed" ta="center" className={styles.emptyText}>
                            No games available
                        </Text>
                    </Flex>
                ) : (
                    <Stack className={styles.gameStack}>
                        {games.map((game) => <GameTile key={game.id} game={game} isJoined={"players" in game} />)}
                    </Stack>
                )}
            </Box>
        </Flex>
    );
};
