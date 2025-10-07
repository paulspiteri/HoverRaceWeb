import * as React from "react";
import {GameTile} from "./GameTile";
import {Title, Stack, Text, Box, Flex} from "@mantine/core";
import styles from "./GameList.module.css";
import {useAtomValue} from 'jotai';
import {gamesAtom} from '@/atoms.ts';
import {useMemo} from 'react';

interface GameListProps {
}

export const GameList: React.FC<GameListProps> = () => {
    const allGames = useAtomValue(gamesAtom);
    const games = useMemo(() => allGames.filter(game => game.status !== "playing"), [allGames]);

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
                        {games.map((game) => <GameTile key={game.id} game={game} isJoined={"players" in game}/>)}
                    </Stack>
                )}
            </Box>
        </Flex>
    );
};
