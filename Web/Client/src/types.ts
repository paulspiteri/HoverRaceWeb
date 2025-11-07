import type { AvailableGame, JoinedGame } from '../../Server/src/types.ts';

export * from '../../Server/src/types.ts';
export * from '../../Server/src/leaderboardTypes.ts';

export type Game = AvailableGame | JoinedGame;
