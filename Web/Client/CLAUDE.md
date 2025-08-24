# HoverRace WebClient - Implementation Details

## Project Overview
Real-time multiplayer game using WebRTC peer-to-peer networking with custom signaling server.

## Architecture

### Frontend Stack
- **React 19** with TypeScript
- **Mantine UI Library**
- **Vite** build tool
- **Simple-peer** for WebRTC connections
- **Custom signaling** via game server

### Backend Integration
- **Game Server**: `/home/paul/develop/HoverRace/Server/src`
- **Real-time Communication**: Server-Sent Events (SSE) for signaling
- **API Endpoints**: REST API for game management

## Key Components

### Game Management (`src/App.tsx`)
- Main application component
- Manages active game state and peer connections
- Handles game creation, joining, leaving, and starting

### UI Components (All Mantine-based)
- **ActiveGame.tsx**: Game lobby interface with player list and controls
- **GameList.tsx**: Available games sidebar
- **GameTile.tsx**: Individual game cards with join/leave buttons
- **PlayerList.tsx**: Player roster with connection status indicators
- **PlayerNameInput.tsx**: Player name management
- **ConnectionStatus.tsx**: Connection indicator with visual status

### Networking Layer

#### WebRTC Peer Management (`src/usePeers.ts`)
- **Simple-peer integration** for WebRTC connections
- **Custom signaling** through game server (no PeerJS broker dependency)
- **Peer status tracking** with connection state management
- **Mesh network topology** for multiplayer connections
- **Data channels** ready for reliable/unreliable communication

#### Game Server Communication (`src/useGameData.ts`, `src/commands.ts`)
- **Server-Sent Events** for real-time updates
- **REST API commands**: create, join, leave, start, signal, updatePlayer
- **Automatic game state synchronization**

## Features Implemented

### Game Lifecycle
- ✅ **Create Game**: Host creates game lobby
- ✅ **Join Game**: Players join via game list
- ✅ **Leave Game**: Players can leave at any time
- ✅ **Start Game**: Host starts game when ready
- ✅ **Game State Management**: Proper waiting/playing status handling

### UI/UX Features
- ✅ **Responsive Design**: Mantine-based responsive layouts
- ✅ **Real-time Updates**: Live player count and status
- ✅ **Connection Indicators**: Visual peer connection status
- ✅ **Smart Button States**: Disabled states for started games
- ✅ **Player Management**: Name persistence via localStorage

### Network Architecture
- ✅ **P2P Mesh Network**: Direct peer-to-peer connections
- ✅ **Custom Signaling**: Own server handles WebRTC signaling
- ✅ **Connection Monitoring**: Peer status tracking and reporting
- ✅ **Reliable Communication**: Default data channel for status messages
- 🔄 **Game Data Channel**: UDP-like channel ready for implementation

## Technical Decisions

### WebRTC Library Choice
- **Evaluated**: PeerJS vs simple-peer
- **Chosen**: simple-peer
- **Reason**: Better support for custom signaling, lighter weight, more control

### Network Topology
- **Mesh Network**: All peers connect to each other
- **Host-based Control**: Game creator has special privileges
- **Custom Signaling**: Server handles WebRTC offer/answer/ICE exchange

## Current State

### Completed Features
- Full game lobby system with real-time updates
- Peer-to-peer connection establishment
- Game state management (waiting/playing)
- Player management with persistent names
- Connection status monitoring and reporting

## Development Notes

### Type Safety
- Full TypeScript coverage with shared types between client/server
- Proper error handling and validation

### Performance Considerations
- Efficient peer connection management
- Minimal re-renders with proper React optimization
- Local state persistence for better UX

### Code Organization
- Clean separation between UI, networking, and game logic
- Reusable hooks for complex state management
- Consistent error handling patterns
