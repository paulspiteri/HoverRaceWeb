# HoverRace Server

A TypeScript WebSocket server for managing online network games using Server-Sent Events (SSE) for real-time updates.

## Requirements

- **Node.js 23+** - Required to run TypeScript files directly 
- **Important**: Never use ts-node or tsx - they are not necessary, Node can run .ts files directly
- **Important**: Never change imports to use `.js` extensions - keep them as `.ts` imports

## Architecture

- **Express.js** server on port 3001
- **Server-Sent Events** for real-time game list updates with creator connection tracking
- **In-memory storage** for game data and player lists
- **REST API** for game creation, deletion, joining, and leaving
- **Automatic cleanup** when game creators disconnect from SSE

## Commands

```bash
# Install dependencies
npm install

# Start server (development with auto-restart)
npm run dev

# Start server (production)
npm start

# Build TypeScript to JavaScript
npm run build

# Lint code
npm run lint

# Lint and fix issues
npm run lint:fix

# Format code with prettier
npm run format

# Check code formatting
npm run format:check
```

## API Endpoints

### REST Endpoints
- `POST /api/games` - Create a new game (requires creatorConnectionId)
- `POST /api/games/:id/join` - Join an existing game (requires connectionId only)
- `POST /api/games/:id/leave` - Leave a game (requires connectionId)
- `DELETE /api/games/:id` - Delete a game

### Real-time Endpoint
- `GET /api/games/stream` - SSE stream for real-time game updates (provides connectionId for game creation)

## Usage Examples

### Create a game
```bash
curl -X POST http://localhost:3001/api/games \
  -H "Content-Type: application/json" \
  -d '{"name": "My Game", "maxPlayers": 4, "creatorConnectionId": "connection-id-from-sse"}'
```

### Join a game
```bash
curl -X POST http://localhost:3001/api/games/1/join \
  -H "Content-Type: application/json" \
  -d '{"connectionId": "your-connection-id-from-sse"}'
```

### Leave a game
```bash
curl -X POST http://localhost:3001/api/games/1/leave \
  -H "Content-Type: application/json" \
  -d '{"connectionId": "your-connection-id-from-sse"}'
```

### Delete a game
```bash
curl -X DELETE http://localhost:3001/api/games/GAME_ID
```

### Listen to real-time updates and get connection ID
```bash
curl -N http://localhost:3001/api/games/stream
# First message: {"type":"connectionId","connectionId":"uuid-here"}
# Second message: {"type":"gameList","games":[...]} (public data with playerCount only)
# Participants also receive: {"type":"gameUpdatedFull","game":{...}} (with full players array)
```

## Project Structure

```
src/
├── server.ts      # Main Express server with SSE and selective broadcasting
├── gameManager.ts # Game and player management logic
└── types.ts       # TypeScript interfaces (Game, Player, PublicGameData, requests)
```

## Game Management Features

- **Player Tracking**: Games maintain a list of connected players with unique IDs and connectionIds
- **Creator Protection**: Game creators cannot join their own games (automatically included)
- **Max Players**: Respects maximum player limits when joining games
- **Auto Cleanup**: Games are automatically deleted when creators disconnect from SSE
- **Connection-based Creation**: Each SSE connection gets a unique UUID for creating games
- **Selective Broadcasting**: Public game data (playerCount) to all, full details only to participants
- **ConnectionId Validation**: Only active SSE connections can join games
- **Duplicate Prevention**: Players cannot join the same game twice
- **Privacy Protection**: Player details only visible to game participants