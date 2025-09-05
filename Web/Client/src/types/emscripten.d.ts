declare global {
    function sendGameMessage(playerId: number, data: Uint8Array, reliable: boolean): boolean;
}

export {};
