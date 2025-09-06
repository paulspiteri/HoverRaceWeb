import * as React from "react";
import { useCallback, useEffect, useState } from "react";
import { useParams, useOutletContext } from "react-router-dom";
import { ActiveGame } from "@/ActiveGame.tsx";
import { JoinGameOffer } from "@/JoinGameOffer.tsx";
import { GameNotFound } from "@/GameNotFound.tsx";
import type { JoinedGame } from "./types";
import { usePeers } from "@/usePeers.ts";
import { useGameInstance } from "@/interop/gameInterop.ts";
import type { GameOutletContext } from "./App";
import { useGameWindowSize } from "@/interop/useGameWindowSize.ts";
import { useAtomValue } from 'jotai';
import { connectionIdAtom, gameTokenAtom, commandsAtom, gamesAtom } from '@/atoms.ts';

export const GamePage: React.FC = () => {
    const { gameId } = useParams();
    const { eventSource, canvasRef } = useOutletContext<GameOutletContext>();
    const connectionId = useAtomValue(connectionIdAtom);
    const gameToken = useAtomValue(gameTokenAtom);
    const commands = useAtomValue(commandsAtom);
    const games = useAtomValue(gamesAtom);
    const { gameInstanceApi, isLoadingGameData } = useGameInstance(canvasRef.current);
    useGameWindowSize(gameInstanceApi, canvasRef.current);

    // Track if we're currently leaving the game to prevent flicker
    const [isLeaving, setIsLeaving] = useState(false);

    const game = games.find((x) => x.id === gameId);
    const joinedGame = game && "players" in game ? (game as JoinedGame) : undefined;
    const playerIndex = joinedGame?.players.findIndex((p) => p?.connectionId === connectionId);

    const isInGame = joinedGame && playerIndex !== undefined && playerIndex >= 0;

    const onGameData = useCallback(
        (playerId: number, data: Uint8Array) => void gameInstanceApi?.receiveGameData(playerId, data),
        [gameInstanceApi],
    );

    const onGamePlayerDisconnected = useCallback(
        (playerId: number) => void gameInstanceApi?.setPlayerStatus(playerId, false, 0, 0),
        [gameInstanceApi],
    );

    const { peerStatuses, peersActualStatuses, peerLatencies, sendData } = usePeers(
        connectionId,
        joinedGame,
        eventSource,
        commands?.sendSignal,
        gameToken,
        isLoadingGameData,
        onGameData,
        onGamePlayerDisconnected,
    );

    useEffect(() => void (global.sendGameMessage = sendData), [sendData]);

    const handleJoinGame = () => {
        if (!gameId || !connectionId || !commands) return;
        const savedName = localStorage.getItem("hoverrace-player-name");
        commands.joinGame(gameId, connectionId, savedName || undefined);
    };

    const handleLeaveGame = async () => {
        if (joinedGame && gameToken && commands) {
            setIsLeaving(true);
            try {
                await commands.leaveGame(joinedGame.id, gameToken);
            } catch (error) {
                console.error("Failed to leave game:", error);
                setIsLeaving(false);
            }
        }
    };

    const handleStartGame = async () => {
        if (joinedGame && gameToken && commands) {
            await commands.startGame(joinedGame.id, gameToken);
        }
    };

    const handleUpdatePlayer = async (name: string) => {
        if (joinedGame && gameToken && commands) {
            await commands.updatePlayer(joinedGame.id, gameToken, name);
        }
    };

    const isGamePlaying = joinedGame?.status === "playing" && playerIndex !== undefined;
    useEffect(() => {
        {
            if (isGamePlaying && gameInstanceApi) {
                peerStatuses?.forEach((x, idx) => {
                    const latencies = peerLatencies?.[idx];
                    gameInstanceApi.setPlayerStatus(
                        idx,
                        x === "connected",
                        latencies?.minimumLatency ?? 0,
                        latencies?.averageLatency ?? 0,
                    );
                });
            }
        }
    }, [peerStatuses, peerLatencies, isGamePlaying, gameInstanceApi]);

    useEffect(() => {
        if (isGamePlaying && gameInstanceApi) {
            gameInstanceApi.startGame(playerIndex);
        }
    }, [gameInstanceApi, isGamePlaying, playerIndex]);

    // If we're currently leaving, don't show anything (prevents flicker)
    if (isLeaving) {
        return null;
    }

    // If we're already in the game, show the normal game interface
    if (isInGame && joinedGame) {
        return (
            <ActiveGame
                game={joinedGame}
                onClose={handleLeaveGame}
                onStartGame={handleStartGame}
                peerStatuses={peerStatuses}
                onUpdatePlayer={handleUpdatePlayer}
                peersActualStatuses={peersActualStatuses}
                peerLatencies={peerLatencies}
                isLoadingGameData={isLoadingGameData}
            />
        );
    }

    // If game exists but we're not in it, show join interface
    if (game && !isInGame) {
        return <JoinGameOffer gameInfo={game} onJoinGame={handleJoinGame} />;
    }

    // Game not found
    return <GameNotFound />;
};
