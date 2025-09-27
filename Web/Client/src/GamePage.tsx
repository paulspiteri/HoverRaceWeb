import * as React from "react";
import { useCallback, useEffect } from "react";
import { useParams } from "react-router-dom";
import { ActiveGame } from "@/ActiveGame.tsx";
import { JoinGameOffer } from "@/JoinGameOffer.tsx";
import { GameNotFound } from "@/GameNotFound.tsx";
import type { JoinedGame } from "./types";
import { usePeers } from "@/usePeers.ts";
import { useGameInstance } from "@/interop/gameInterop.ts";
import { useGameWindowSize } from "@/interop/useGameWindowSize.ts";
import { useAtomValue, useSetAtom } from "jotai";
import {
    connectionIdAtom,
    gameTokenAtom,
    commandsAtom,
    gamesAtom,
    eventSourceAtom,
    canvasAtom,
    gameScreenModeAtom, gameApiAtom,
} from "@/atoms.ts";

export const GamePage: React.FC = () => {
    const { gameId } = useParams();
    const connectionId = useAtomValue(connectionIdAtom);
    const gameToken = useAtomValue(gameTokenAtom);
    const commands = useAtomValue(commandsAtom);
    const eventSource = useAtomValue(eventSourceAtom);
    const canvasRef = useAtomValue(canvasAtom);
    const games = useAtomValue(gamesAtom);
    const setGameScreenMode = useSetAtom(gameScreenModeAtom);
    const setGameAPI = useSetAtom(gameApiAtom);
    const { gameInstanceApi, isLoadingGameData } = useGameInstance(canvasRef?.current ?? null);
    useEffect(() => setGameAPI(gameInstanceApi), [setGameAPI, gameInstanceApi]);
    useGameWindowSize(gameInstanceApi, canvasRef?.current ?? null);

    const game = games.find((x) => x.id === gameId);
    const joinedGame = game && "players" in game ? (game as JoinedGame) : undefined;
    const playerIndex = joinedGame?.players.findIndex((p) => p?.connectionId === connectionId);

    const isInGame = joinedGame && playerIndex !== undefined && playerIndex >= 0;

    const onGameData = useCallback(
        (playerId: number, data: Uint8Array, reliable: boolean) =>
            void gameInstanceApi?.receiveGameData(playerId, data, reliable),
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

    const isGamePlaying = joinedGame?.status === "playing" && playerIndex !== undefined;
    useEffect(() => {
        {
            if (isGamePlaying && gameInstanceApi) {
                const isMobile = window.matchMedia("(pointer: coarse)").matches;
                setGameScreenMode(isMobile ? "maximized" : "mini");
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
    }, [peerStatuses, peerLatencies, isGamePlaying, gameInstanceApi, setGameScreenMode]);

    useEffect(() => {
        if (isGamePlaying && gameInstanceApi) {
            gameInstanceApi.startGame(playerIndex);
        }
    }, [gameInstanceApi, isGamePlaying, playerIndex]);

    // If we're already in the game, show the normal game interface
    if (isInGame && joinedGame) {
        return (
            <ActiveGame
                game={joinedGame}
                peerStatuses={peerStatuses}
                peersActualStatuses={peersActualStatuses}
                peerLatencies={peerLatencies}
                isLoadingGameData={isLoadingGameData}
            />
        );
    }

    // If game exists but we're not in it, show join interface
    if (game && !isInGame) {
        return <JoinGameOffer gameInfo={game} />;
    }

    // Game not found
    return <GameNotFound />;
};
