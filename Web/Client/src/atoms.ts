import { atom } from "jotai";
import type { Commands } from "./commands";
import type { Game } from "./types";

export const connectionIdAtom = atom<string | undefined>(undefined);
export const gameTokenAtom = atom<string | undefined>(undefined);
export const commandsAtom = atom<Commands | undefined>(undefined);
export const gamesAtom = atom<Game[]>([]);
