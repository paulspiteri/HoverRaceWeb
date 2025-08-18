import express from 'express';
import cors from 'cors';
import { v4 as uuidv4 } from 'uuid';
import { gameManager } from './gameManager';
import type {
  CreateGameRequest,
  JoinGameRequest,
  PublicGameData,
  Game,
} from './types';

const app = express();
const port = 3001;

app.use(cors());
app.use(express.json());

// Store SSE connections with associated creator IDs
const sseClients = new Map<express.Response, string>();

// SSE endpoint for real-time game list updates
app.get('/api/games/stream', (req, res) => {
  res.writeHead(200, {
    'Content-Type': 'text/event-stream',
    'Cache-Control': 'no-cache',
    Connection: 'keep-alive',
    'Access-Control-Allow-Origin': '*',
  });

  // Generate unique connection ID for this client
  const connectionId = uuidv4();

  // Send connection ID and current games immediately (public data only)
  const games = gameManager.getAllGames();
  const publicGames = games.map(toPublicGameData);
  res.write(
    `data: ${JSON.stringify({ type: 'connectionId', connectionId })}\n\n`
  );
  res.write(
    `data: ${JSON.stringify({ type: 'gameList', games: publicGames })}\n\n`
  );

  sseClients.set(res, connectionId);

  req.on('close', () => {
    const disconnectedConnectionId = sseClients.get(res);
    sseClients.delete(res);

    // If this connection created any games, delete them
    if (disconnectedConnectionId) {
      const games = gameManager.getAllGames();
      games.forEach((game) => {
        if (game.creatorConnectionId === disconnectedConnectionId) {
          gameManager.deleteGame(game.id);
        }
      });
    }
  });
});

// REST endpoint to create a game
app.post('/api/games', (req, res) => {
  try {
    const gameData: CreateGameRequest = req.body;

    if (!gameData.name || !gameData.maxPlayers || !gameData.creatorConnectionId) {
      return res.status(400).json({ error: 'Missing required fields' });
    }

    const game = gameManager.createGame(gameData);
    res.status(201).json(game);
  } catch {
    res.status(500).json({ error: 'Failed to create game' });
  }
});

// REST endpoint to join a game
app.post('/api/games/:id/join', (req, res) => {
  try {
    const { id } = req.params;
    const { connectionId }: JoinGameRequest = req.body;

    if (!connectionId) {
      return res.status(400).json({ error: 'Missing connectionId' });
    }

    // Validate connectionId exists in active SSE connections
    const isValidConnectionId = Array.from(sseClients.values()).includes(connectionId);
    if (!isValidConnectionId) {
      return res.status(400).json({ error: 'Invalid or inactive connectionId' });
    }

    const joined = gameManager.joinGame(id, connectionId);

    if (joined) {
      res.status(200).json({ message: 'Joined game successfully' });
    } else {
      res.status(400).json({ error: 'Cannot join game' });
    }
  } catch {
    res.status(500).json({ error: 'Failed to join game' });
  }
});

// REST endpoint to leave a game
app.post('/api/games/:id/leave', (req, res) => {
  try {
    const { id } = req.params;
    const { connectionId }: JoinGameRequest = req.body;

    if (!connectionId) {
      return res.status(400).json({ error: 'Missing connectionId' });
    }

    const left = gameManager.leaveGame(id, connectionId);

    if (left) {
      res.status(200).json({ message: 'Left game successfully' });
    } else {
      res.status(400).json({ error: 'Cannot leave game' });
    }
  } catch {
    res.status(500).json({ error: 'Failed to leave game' });
  }
});

// REST endpoint to delete a game
app.delete('/api/games/:id', (req, res) => {
  const { id } = req.params;
  const deleted = gameManager.deleteGame(id);

  if (deleted) {
    res.status(204).send();
  } else {
    res.status(404).json({ error: 'Game not found' });
  }
});

// Convert Game to PublicGameData
function toPublicGameData(game: Game): PublicGameData {
  return {
    id: game.id,
    name: game.name,
    playerCount: game.players.length,
    maxPlayers: game.maxPlayers,
    createdAt: game.createdAt,
    creatorConnectionId: game.creatorConnectionId,
  };
}

// Broadcast public game updates to all SSE clients
function broadcastPublicUpdate(data: unknown) {
  const message = `data: ${JSON.stringify(data)}\n\n`;
  sseClients.forEach((creatorId, client) => {
    try {
      client.write(message);
    } catch {
      sseClients.delete(client);
    }
  });
}

// Broadcast private game updates only to participants
function broadcastPrivateUpdate(game: Game, data: unknown) {
  const message = `data: ${JSON.stringify(data)}\n\n`;
  const participantConnectionIds = game.players.map((p) => p.connectionId);

  sseClients.forEach((connectionId, client) => {
    if (participantConnectionIds.includes(connectionId)) {
      try {
        client.write(message);
      } catch {
        sseClients.delete(client);
      }
    }
  });
}

// Listen for game events and broadcast
gameManager.on('gameCreated', (game: Game) => {
  // Send public data to all clients
  broadcastPublicUpdate({ type: 'gameCreated', game: toPublicGameData(game) });
  // Send full data to participants only
  broadcastPrivateUpdate(game, { type: 'gameCreatedFull', game });
});

gameManager.on('gameDeleted', (gameId: string) => {
  // Game deletion is public information
  broadcastPublicUpdate({ type: 'gameDeleted', gameId });
});

gameManager.on('gameUpdated', (game: Game) => {
  // Send public data to all clients
  broadcastPublicUpdate({ type: 'gameUpdated', game: toPublicGameData(game) });
  // Send full data to participants only
  broadcastPrivateUpdate(game, { type: 'gameUpdatedFull', game });
});

app.listen(port, () => {
  console.log(`Server running on port ${port}`);
});
