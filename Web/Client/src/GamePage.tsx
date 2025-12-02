import * as React from "react";
import {useCallback, useEffect, useRef, useState} from "react";
import {useParams} from "react-router-dom";
import {ActiveGame} from "@/ActiveGame.tsx";
import {JoinGameOffer} from "@/JoinGameOffer.tsx";
import {GameNotFound} from "@/GameNotFound.tsx";
import type {JoinedGame, VehicleType} from "./types";
import {usePeers} from "@/usePeers.ts";
import {useGameInstance} from "@/interop/gameInterop.ts";
import {useGameWindowSize} from "@/interop/useGameWindowSize.ts";
import {useLeaderboard} from "@/hooks/useLeaderboard.ts";
import {useSubmitLapTime} from "@/hooks/useSubmitLapTime.ts";
import {useGhostReplay} from "@/hooks/useGhostReplay.ts";
import {isMobileDevice} from "@/utils/deviceDetection.ts";
import {useAtomValue, useSetAtom} from "jotai";
import {
    connectionIdAtom,
    gameTokenAtom,
    commandsAtom,
    gamesAtom,
    eventSourceAtom,
    canvasAtom,
    gameScreenModeAtom, gameApiAtom,
} from "@/atoms.ts";
import {notifications} from "@mantine/notifications";

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
    const playerName = playerIndex !== undefined ? joinedGame?.players[playerIndex]?.name : undefined;
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

    const trackName = joinedGame?.trackName.replace('.trk', '');
    const [currentVehicleType, setCurrentVehicleType] = useState<VehicleType>();
    const isMobile = isMobileDevice();
    const { data: leaderboard } = useLeaderboard(trackName, 10, undefined, isMobile);
    const { data: vehicleLeaderboard } = useLeaderboard(currentVehicleType !== undefined ? trackName : undefined, 1, currentVehicleType, isMobile); // get best lap for vehicle once selected
    const bestVehicleLap = vehicleLeaderboard?.[0];
    const { data: ghostReplayData } = useGhostReplay(bestVehicleLap?.id);

    useEffect(() => {
        if (ghostReplayData && gameInstanceApi) {
            console.log('Loading best lap ghost replay:', ghostReplayData.length, 'bytes');
            gameInstanceApi.loadBestLapGhost(ghostReplayData);
        }
    }, [ghostReplayData, gameInstanceApi]);

    useEffect(() => {
        global.sendGameMessage = sendData;
        return () => {
            delete global.sendGameMessage;
        };
    }, [sendData]);

    const submitLapTimeMutation = useSubmitLapTime();

    useEffect(() => {
        if (!trackName) {
            return;
        }
        global.onLapComplete = (newLap: number, lapTimeMs: number, vehicleType: VehicleType, ghostReplayData: Uint8Array | null) => {
            // Update current vehicle type for leaderboard filtering
            setCurrentVehicleType(vehicleType);

            if (newLap <= 1 || !ghostReplayData) return;
            if (bestVehicleLap !== undefined && lapTimeMs >= bestVehicleLap?.lapTimeMs) return;

            const ghostReplay = btoa(String.fromCharCode(...ghostReplayData));
            const isMobile = window.matchMedia("(pointer: coarse)").matches;

            submitLapTimeMutation.mutate({
                playerName,
                trackName,
                lapTimeMs,
                isMobile,
                vehicleType,
                ghostReplay,
            }, {
                onSuccess: () => {
                    notifications.show({
                        title: 'Lap Record',
                        message: `Lap time ${(lapTimeMs / 1000).toFixed(2)}s submitted to leaderboard!`,
                        color: 'green',
                    });
                },
            });
        };

        return () => {
            delete global.onLapComplete;
        };
    }, [bestVehicleLap, playerName, submitLapTimeMutation, trackName]);

    const isGameStarted = useRef(false) // to guarantee we only start once
    const isGamePlaying = joinedGame?.status === "playing" && playerIndex !== undefined;
    const hasWeapons = joinedGame?.hasWeapons;
    const laps = joinedGame?.laps;
    useEffect(() => {
        if (!isGameStarted.current && isGamePlaying && gameInstanceApi && trackName && hasWeapons !== undefined && laps) {
            isGameStarted.current = true;
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
            gameInstanceApi.startGame(
                playerIndex,
                trackName + '.trk',
                hasWeapons,
                laps
            );
        }
    }, [gameInstanceApi, hasWeapons, isGamePlaying, laps, peerLatencies, peerStatuses, playerIndex, setGameScreenMode, trackName]);

    // If we're already in the game, show the normal game interface
    if (isInGame && joinedGame) {
        return (
            <ActiveGame
                game={joinedGame}
                peerStatuses={peerStatuses}
                peersActualStatuses={peersActualStatuses}
                peerLatencies={peerLatencies}
                isLoadingGameData={isLoadingGameData}
                bestLapTime={leaderboard?.[0]?.lapTimeMs}
            />
        );
    }

    // If game exists but we're not in it, show join interface
    if (game && !isInGame) {
        return <JoinGameOffer gameInfo={game}/>;
    }

    // Game not found
    return <GameNotFound/>;
};
