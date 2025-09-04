/**
 * TypeScript declarations for emscripten.js functions
 */

declare global {
    function receiveGameData(playerId: number, data: unknown): void;
    function sendGameMessage(playerId: number, data: Uint8Array, reliable: boolean): boolean;
    function setPlayerStatus(playerId: number, isConnected: boolean, minLatency: number, avgLatency: number): void;
}

export {};
