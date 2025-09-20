import { atom } from "jotai";
import type { Commands } from "./commands";
import type { Game } from "./types";
import type { RefObject } from "react";

export const connectionIdAtom = atom<string | undefined>(undefined);
export const gameTokenAtom = atom<string | undefined>(undefined);
export const commandsAtom = atom<Commands | undefined>(undefined);
export const gamesAtom = atom<Game[]>([]);
export const eventSourceAtom = atom<EventSource | undefined>(undefined);
export const canvasAtom = atom<RefObject<HTMLCanvasElement | null> | undefined>(undefined);
export const gameScreenModeAtom = atom<"hidden" | "mini" | "maximized" | "fullscreen">("hidden");
