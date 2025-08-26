/**
 * TypeScript declarations for emscripten.js functions
 */

declare global {
    function startGame(playerId: number): void;
    function receiveGameData(playerId: number, data: unknown): void;
    function sendGameMessage(playerId: number, data: Uint8Array, reliable: boolean): boolean;
}

export {};
