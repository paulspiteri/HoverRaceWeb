/**
 * TypeScript declarations for emscripten.js functions
 */

declare global {
  /**
   * Starts the game with the specified player ID.
   * Shows the game canvas and hides the React UI.
   * 
   * @param playerId - The ID of the player starting the game
   */
  function startGame(playerId: number): void;
}

export {};