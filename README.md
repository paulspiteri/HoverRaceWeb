# HoverRaceWeb

A web port of [HoverRace](https://github.com/HoverRace/HoverRace), the classic multiplayer hovercraft racing game originally by Richard Langlois / GrokkSoft Inc.

## What's Changed from the Original

The original HoverRace was a Windows-only game built on MFC and DirectX. This fork replaces that platform-specific stack with cross-platform and web-friendly alternatives:

- **MFC / DirectX / Win32 removed** — replaced with [SDL3](https://github.com/libsdl-org/SDL) for window management and input
- **[Sokol](https://github.com/floooh/sokol)** for OpenGL (native) / WebGL2 (browser) rendering
- **[Emscripten](https://emscripten.org/)** to compile the C++ game engine to WebAssembly, running natively in the browser
- **WebRTC** (via [simple-peer](https://github.com/feross/simple-peer)) for peer-to-peer multiplayer networking
- **React** frontend for the game lobby and user interface

## Architecture

The project has three main parts:

### C++ Game Engine
The original game logic — physics, collisions, track loading, object simulation, rendering — is preserved in C++ and built with CMake. It compiles to both a native binary (SDL3 + OpenGL) and a WebAssembly module (Emscripten + WebGL2) from the same codebase.

Game assets (tracks, sprites, fonts) are embedded into the WASM binary so the game works without additional downloads.

### React Frontend (`Web/Client`)
A React 19 / TypeScript / Vite app that hosts the WASM game module and provides the game lobby UI. Players can create or join games, see who's in the lobby, and track lap times. The frontend talks to the WASM engine via Emscripten's exported C function interface.

Mobile is supported via touch joystick controls.

### Node.js Backend (`Web/Server`)
A lightweight Express server that handles game lobby signaling — creating and listing games, coordinating WebRTC peer connections. Actual gameplay data flows directly peer-to-peer; the server is not in the game data path. Lap times and leaderboards are persisted in SQLite.

## Building

### Prerequisites

- CMake 3.20+
- Emscripten SDK (for web build)
- Node.js 20+

A [Nix flake](flake.nix) is provided that sets up the full development environment including Emscripten, CMake, Node.js, and graphics libraries.

### Web Build

```sh
./build-web.sh
```

This compiles the C++ engine to `hoverrace.js` / `hoverrace.wasm` via Emscripten and places the output in the frontend's `public/` directory.

Then build and run the frontend:

```sh
cd Web/Client
npm install
npm run dev
```

And the backend:

```sh
cd Web/Server
npm install
npm run dev
```

### Native Build

```sh
cmake -B build
cmake --build build
```

Produces a native SDL3 + OpenGL binary for local testing without a browser.

## Tracks

Four tracks are included from the original game:

- Classic H
- Steeplechase
- The Alley 2
- The River

## License

GrokkSoft HoverRace SourceCode License v0.1 — see original license terms from the upstream repository.
