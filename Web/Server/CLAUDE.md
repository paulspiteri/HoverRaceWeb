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
- **REST API** for game creation, deletion, joining, leaving, starting, and player updates
- **Automatic cleanup** when game creators disconnect from SSE
- **Fixed-size player arrays** with undefined slots to preserve indices

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
- `POST /api/games` - Create a new game (requires creatorConnectionId, optional creatorName)
- `POST /api/games/:id/join` - Join an existing game (requires connectionId, optional name)
- `POST /api/games/:id/leave` - Leave a game (requires gameToken)
- `POST /api/games/:id/start` - Start a game (requires creatorToken, creator only)
- `PUT /api/games/:id/player` - Update player info (requires gameToken, name)
- `POST /api/games/:id/signal` - Send WebRTC signaling data (requires gameToken, targetConnectionId, signalData)
- `DELETE /api/games/:id` - Delete a game (requires creatorToken)

### Real-time Endpoint
- `GET /api/games/stream` - SSE stream for real-time game updates (provides connectionId for game creation)

## Usage Examples

### Create a game
```bash
curl -X POST http://localhost:3001/api/games \
  -H "Content-Type: application/json" \
  -d '{"name": "My Game", "maxPlayers": 4, "creatorConnectionId": "connection-id-from-sse", "creatorName": "Player1"}'
```

### Join a game
```bash
curl -X POST http://localhost:3001/api/games/1/join \
  -H "Content-Type: application/json" \
  -d '{"connectionId": "your-connection-id-from-sse", "name": "Player2"}'
```

### Leave a game
```bash
curl -X POST http://localhost:3001/api/games/1/leave \
  -H "Content-Type: application/json" \
  -d '{"gameToken": "your-game-token"}'
```

### Start a game
```bash
curl -X POST http://localhost:3001/api/games/1/start \
  -H "Content-Type: application/json" \
  -d '{"creatorToken": "your-creator-token"}'
```

### Update player info
```bash
curl -X PUT http://localhost:3001/api/games/1/player \
  -H "Content-Type: application/json" \
  -d '{"gameToken": "your-game-token", "name": "NewName"}'
```

### Send WebRTC signal
```bash
curl -X POST http://localhost:3001/api/games/1/signal \
  -H "Content-Type: application/json" \
  -d '{"gameToken": "your-token", "targetConnectionId": "target-id", "signalData": "signal-data"}'
```

### Delete a game
```bash
curl -X DELETE http://localhost:3001/api/games/GAME_ID \
  -H "Content-Type: application/json" \
  -d '{"creatorToken": "your-creator-token"}'
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

- **Game Status**: Games have 'waiting' or 'playing' status - only waiting games can be joined
- **Fixed Player Arrays**: Player arrays are fixed-size (maxPlayers) with undefined slots preserving indices
- **Player Names**: Optional player names for creators and joiners, updatable via API
- **Game Starting**: Only creators can start their games, changes status from 'waiting' to 'playing'
- **Creator Persistence**: Started games persist even if creator leaves (unlike waiting games)
- **Token-based Auth**: Game operations use gameToken/creatorToken for security
- **WebRTC Signaling**: Direct peer-to-peer signaling between game participants
- **Auto Cleanup**: Waiting games deleted when creators disconnect, started games persist
- **Connection-based Creation**: Each SSE connection gets a unique UUID for creating games
- **Selective Broadcasting**: Public game data (playerCount, status) to all, full details only to participants
- **ConnectionId Validation**: Only active SSE connections can join games
- **Duplicate Prevention**: Players cannot join the same game twice
- **Privacy Protection**: Player details and tokens only visible to game participants