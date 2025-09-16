import { useCallback, useEffect, useRef, useState } from "react";
import type { JoinedGame, ServerMessage, SignalMessage } from "@/types.ts";
import type { PeerConnectionStatusMessage, PeerMessage } from "@/peerTypes.ts";
import { MESSAGE_TYPES, createEnvelope, parseEnvelope, createJsonEnvelope } from "@/peerMessage.ts";
import SimplePeer from "simple-peer";

export interface GamePeer {
    peer: SimplePeer.Instance;
    connectionId: string;
    unreliableChannel?: RTCDataChannel;
    reliableChannel?: RTCDataChannel;
}

export interface PeerConnectionLatency {
    connectionId: string;
    latencies: number[];
    averageLatency: number;
    minimumLatency: number;
}

// Utility function to access the internal RTCPeerConnection
const LATENCY_MEASUREMENT_COUNT = 10;

export const getPeerConnection = (peer: SimplePeer.Instance): RTCPeerConnection => {
    return (peer as object as { _pc: RTCPeerConnection })._pc;
};

export const usePeers = (
    connectionId: string | undefined,
    game: JoinedGame | undefined,
    eventSource: EventSource | undefined,
    sendSignal:
        | ((gameId: string, targetConnectionId: string, gameToken: string, signalData: string) => Promise<void>)
        | undefined,
    gameToken: string | undefined,
    isLoadingGameData: boolean,
    onGameData: (playerIndex: number, data: Uint8Array, reliable: boolean) => void,
    onGamePlayerPeerDisconnect: (playerIndex: number) => void,
) => {
    const peers = useRef<(GamePeer | undefined)[] | undefined>(undefined);
    const earlySignals = useRef<Record<string, SignalMessage>>(undefined);
    const [peerStatuses, setPeerStatuses] = useState<("connecting" | "connected" | "disconnected" | undefined)[]>();
    const [peersActualStatuses, setPeersActualStatuses] = useState<PeerConnectionStatusMessage[]>();
    const [peerLatencies, setPeerLatencies] = useState<PeerConnectionLatency[]>();

    // these refs are required because the underlying GameAPI can be created AFTER the peers are created
    const onGameDataRef = useRef(onGameData);
    const onGamePlayerPeerDisconnectRef = useRef(onGamePlayerPeerDisconnect);
    useEffect(() => void (onGameDataRef.current = onGameData), [onGameData]);
    useEffect(
        () => void (onGamePlayerPeerDisconnectRef.current = onGamePlayerPeerDisconnect),
        [onGamePlayerPeerDisconnect],
    );

    const sendJsonMessage = useCallback((peer: GamePeer, message: PeerMessage, reliable: boolean = true): boolean => {
        if (!peer.peer.connected) {
            console.warn(`❌ Cannot send ${message.type}: peer not connected (${peer.connectionId})`);
            return false;
        }

        if (reliable) {
            if (peer.reliableChannel?.readyState !== "open") {
                console.warn(
                    `❌ Cannot send ${message.type} to ${peer.connectionId}: reliable channel not open (state: ${peer.reliableChannel?.readyState})`,
                );
                return false;
            }
        } else {
            if (peer.unreliableChannel?.readyState !== "open") {
                console.warn(
                    `❌ Cannot send ${message.type} to ${peer.connectionId}: unreliable channel not open (state: ${peer.unreliableChannel?.readyState})`,
                );
                return false;
            }
        }

        try {
            const envelope = createJsonEnvelope(message);

            if (reliable) {
                peer.reliableChannel!.send(new Uint8Array(envelope));
            } else {
                peer.unreliableChannel!.send(new Uint8Array(envelope));
            }
            return true;
        } catch (error) {
            console.error(`❌ Failed to send ${message.type} to ${peer.connectionId}:`, error);
            return false;
        }
    }, []);

    // Send status update to HOST peer
    const sendStatusUpdateToHost = useCallback(() => {
        if (!game || !peers.current || !peerStatuses) {
            return;
        }

        const hostPeer = peers.current[0];
        if (!hostPeer || !hostPeer.peer.connected) {
            return;
        }

        const statusMessage: PeerConnectionStatusMessage = {
            type: "peerConnectionStatus",
            isLoadingGameData,
            peers: game.players.reduce(
                (acc, player, index) => {
                    if (player && player.connectionId !== connectionId) {
                        acc[player.connectionId] = {
                            isConnected: peerStatuses[index] === "connected",
                        };
                    }
                    return acc;
                },
                {} as Record<string, { isConnected: boolean }>,
            ),
        };

        sendJsonMessage(hostPeer, statusMessage);
    }, [connectionId, game, isLoadingGameData, peerStatuses, sendJsonMessage]);

    useEffect(() => sendStatusUpdateToHost(), [peerStatuses, sendStatusUpdateToHost, isLoadingGameData]);

    const gameStatus = game?.status;
    useEffect(() => {
        if (!peerStatuses || gameStatus !== "waiting") return;

        const tmr = setInterval(() => {
            if (!peers.current) return;
            let anyNeedsPing = false;
            for (let i = 0; i < peers.current.length; i++) {
                const peer = peers.current[i];
                if (!peer || !peer.peer.connected || peerStatuses[i] !== "connected") continue;

                const latencyData = peerLatencies?.[i];
                const needsPing = !latencyData || latencyData.latencies.length < LATENCY_MEASUREMENT_COUNT;
                if (needsPing) {
                    anyNeedsPing = true;
                    console.log(`🏓 Sending scheduled ping to player ${i} (${peer.connectionId})`);
                    sendJsonMessage(
                        peer,
                        {
                            type: "peerConnectionPing",
                            pingType: "ping",
                            timestamp: Date.now(),
                        },
                        false,
                    );
                }
            }
            if (!anyNeedsPing) {
                clearInterval(tmr);
            }
        }, 1000);

        return () => clearInterval(tmr);
    }, [peerStatuses, peerLatencies, sendJsonMessage, gameStatus]);

    const onSignalReceived = useCallback((signal: SignalMessage) => {
        console.log(`📡 Received signal from ${signal.fromConnectionId}`);
        const gamePeer = peers.current?.find((x) => x && x.connectionId === signal.fromConnectionId);
        if (gamePeer) {
            gamePeer.peer.signal(signal.signalData);
        } else if (earlySignals.current) {
            console.warn(`❌ Signal received from unknown peer - saving in buffer.`);
            earlySignals.current[signal.fromConnectionId] = signal;
        } else {
            console.warn(`❌ Signal received with no active game.`);
        }
    }, []);

    const handleChannelMessage = useCallback(
        (data: Uint8Array, playerConnectionId: string, playerIndex: number, reliable: boolean) => {
            if (data.length >= 4) {
                try {
                    const { messageType, data: payload } = parseEnvelope(data);

                    if (messageType === MESSAGE_TYPES.GAME_BINARY) {
                        onGameDataRef.current(playerIndex, payload, reliable);
                    } else if (messageType === MESSAGE_TYPES.JSON) {
                        const message = JSON.parse(new TextDecoder().decode(payload)) as PeerMessage;
                        if (message.type === "peerConnectionStatus") {
                            console.log(`📊 Received status update from ${playerConnectionId}:`, message);
                            setPeersActualStatuses((prev) => {
                                if (prev) {
                                    const updated = [...prev];
                                    updated[playerIndex] = message as PeerConnectionStatusMessage;
                                    return updated;
                                }
                            });
                        } else if (message.type === "peerConnectionPing") {
                            if (message.pingType === "ping") {
                                console.log(`🏓 Received ping from ${playerConnectionId}:`, message);
                                sendJsonMessage(
                                    peers.current![playerIndex]!,
                                    {
                                        type: "peerConnectionPing",
                                        pingType: "pong",
                                        timestamp: message.timestamp,
                                    },
                                    false,
                                );
                            }
                            if (message.pingType === "pong") {
                                const latency = Date.now() - message.timestamp;
                                console.log(`📶 Received pong from ${playerConnectionId}, latency: ${latency}ms`);
                                setPeerLatencies((prev) => {
                                    if (prev) {
                                        const updated = [...prev];
                                        const existing = updated[playerIndex];
                                        const newLatencies = existing ? [...existing.latencies, latency] : [latency];
                                        const averageLatency = Math.round(
                                            newLatencies.reduce((a, b) => a + b, 0) / newLatencies.length,
                                        );
                                        const minimumLatency = Math.min(...newLatencies);

                                        updated[playerIndex] = {
                                            connectionId: playerConnectionId,
                                            latencies: newLatencies,
                                            averageLatency: averageLatency,
                                            minimumLatency: minimumLatency,
                                        };
                                        return updated;
                                    }
                                });
                            }
                        } else {
                            console.error(
                                `❌ Unknown JSON message type received from ${playerConnectionId}:`,
                                (message as PeerMessage).type,
                            );
                        }
                    } else {
                        console.error(`❌ Unknown envelope type ${messageType} from ${playerConnectionId}`);
                    }
                } catch (error) {
                    console.error(`❌ Failed to parse data from ${playerConnectionId}:`, error);
                }
            } else {
                console.error(`❌ Unexpected short message from ${playerConnectionId}`);
            }
        },
        [sendJsonMessage],
    );

    useEffect(() => {
        const onMessage = (event: MessageEvent) => {
            try {
                const data = JSON.parse(event.data) as ServerMessage;
                if (data.type === "signal") {
                    onSignalReceived(data);
                }
            } catch (error) {
                console.error("Error parsing SSE message in usePeers:", error);
            }
        };
        eventSource?.addEventListener("message", onMessage);
        return () => eventSource?.removeEventListener("message", onMessage);
    }, [eventSource, onSignalReceived]);

    useEffect(() => {
        if (game && gameToken) {
            const myPlayerIndex = game.players.findIndex((x) => x?.connectionId === connectionId);
            if (myPlayerIndex === -1) {
                throw new Error("Current connection not found in game players");
            }

            if (!peers.current) {
                peers.current = new Array<GamePeer | undefined>(game.maxPlayers);
            }
            if (!earlySignals.current) {
                earlySignals.current = {};
            }
            setPeerStatuses((prev) => (!prev ? new Array(game.maxPlayers).fill(undefined) : prev));
            setPeersActualStatuses((prev) => (!prev ? new Array(game.maxPlayers).fill(undefined) : prev));
            setPeerLatencies((prev) => (!prev ? new Array(game.maxPlayers).fill(undefined) : prev));

            for (let i = 0; i < game.maxPlayers; i++) {
                if (i === myPlayerIndex) {
                    continue;
                }
                const currentPeer = peers.current[i];
                const gamePlayer = game.players[i];

                // Only destroy peer if player actually changed
                if (currentPeer && (!gamePlayer || currentPeer.connectionId !== gamePlayer.connectionId)) {
                    console.log(`🔄 Player ${i} changed, destroying old peer`);
                    currentPeer.peer.destroy();
                    peers.current[i] = undefined;
                    setPeerStatuses((prev) => {
                        const updated = [...(prev || [])];
                        updated[i] = undefined;
                        return updated;
                    });
                }

                // Only create peer if slot is empty and player exists and it's not us
                if (!peers.current[i] && gamePlayer && gamePlayer.connectionId !== connectionId) {
                    const isInitiator = myPlayerIndex > i;
                    const playerConnectionId = gamePlayer.connectionId;
                    console.log(`🤝 Creating peer for player ${i} (${playerConnectionId}) - initiator: ${isInitiator}`);

                    const simplePeer = new SimplePeer({ initiator: isInitiator, trickle: true });
                    const newPeer: GamePeer = {
                        connectionId: playerConnectionId,
                        peer: simplePeer,
                    };
                    peers.current[i] = newPeer;

                    // If we have a signal for this player, use it immediately
                    if (earlySignals.current[playerConnectionId]) {
                        simplePeer.signal(earlySignals.current[playerConnectionId].signalData);
                        delete earlySignals.current[playerConnectionId];
                    }

                    simplePeer.on("signal", (data: SimplePeer.SignalData) => {
                        console.log(`📤 Sending signal to ${playerConnectionId}`);
                        if (game && gameToken && sendSignal) {
                            sendSignal(game.id, playerConnectionId, gameToken, JSON.stringify(data));
                        } else {
                            console.error(`❌ Cannot send signal, not in a ready game state.`);
                        }
                    });

                    const onChannelOpen = () => {
                        {
                            if (
                                newPeer.unreliableChannel?.readyState === "open" &&
                                newPeer.reliableChannel?.readyState === "open"
                            ) {
                                updatePeerStatus("connected");
                            }
                        }
                    };

                    if (!isInitiator) {
                        const pc = getPeerConnection(simplePeer);

                        pc.addEventListener("datachannel", (event) => {
                            const channel = event.channel;
                            console.log(`📡 Received data channel: ${channel.label} from ${playerConnectionId}`);

                            if (channel.label === "unreliable") {
                                newPeer.unreliableChannel = channel;

                                channel.onmessage = (event) =>
                                    handleDataChannelMessage(event.data, playerConnectionId, i, false);
                                channel.onopen = () => {
                                    console.log(`📡 Unreliable channel open to ${playerConnectionId}`);
                                    onChannelOpen();
                                };
                            } else if (channel.label === "reliable") {
                                newPeer.reliableChannel = channel;

                                channel.onmessage = (event) =>
                                    handleDataChannelMessage(event.data, playerConnectionId, i, true);
                                channel.onopen = () => {
                                    console.log(`📡 Reliable channel open to ${playerConnectionId}`);
                                    onChannelOpen();
                                };
                            }
                        });
                    }

                    setPeerStatuses((prev) => {
                        const updated = [...(prev || [])];
                        updated[i] = "connecting";
                        return updated;
                    });

                    const updatePeerStatus = (status: "connected" | "disconnected") => {
                        setPeerStatuses((prev) => {
                            const updated = [...(prev || [])];
                            updated[i] = status;
                            return updated;
                        });

                        if (status === "disconnected" && game.status === "playing") {
                            onGamePlayerPeerDisconnectRef.current(i);
                        }
                    };

                    const handleDataChannelMessage = (
                        data: unknown,
                        playerConnectionId: string,
                        playerIndex: number,
                        reliable: boolean,
                    ) => {
                        const uint8Data = new Uint8Array(data as ArrayBuffer);
                        handleChannelMessage(uint8Data, playerConnectionId, playerIndex, reliable);
                    };

                    simplePeer.on("connect", () => {
                        console.log(`🔗 Connected to peer ${playerConnectionId}`);

                        if (isInitiator) {
                            const pc = getPeerConnection(simplePeer);
                            newPeer.unreliableChannel = pc.createDataChannel("unreliable", {
                                ordered: false,
                                maxRetransmits: 0,
                            });
                            console.log(`📡 Created unreliable channel for ${playerConnectionId}`);
                            newPeer.unreliableChannel.onmessage = (event) =>
                                handleDataChannelMessage(event.data, playerConnectionId, i, false);
                            newPeer.unreliableChannel.onopen = () => {
                                console.log(`📡 Unreliable channel open to ${playerConnectionId}`);
                                onChannelOpen();
                            };

                            newPeer.reliableChannel = pc.createDataChannel("reliable", {
                                ordered: true,
                            });
                            console.log(`📡 Created reliable channel for ${playerConnectionId}`);
                            newPeer.reliableChannel.onmessage = (event) =>
                                handleDataChannelMessage(event.data, playerConnectionId, i, true);
                            newPeer.reliableChannel.onopen = () => {
                                console.log(`📡 Reliable channel open to ${playerConnectionId}`);
                                onChannelOpen();
                            };
                        }
                    });

                    simplePeer.on("close", () => {
                        console.log(`🔌 Peer disconnected: ${playerConnectionId}`);
                        updatePeerStatus("disconnected");
                    });

                    simplePeer.on("error", (err: Error) => {
                        console.error(`❌ Peer error with ${playerConnectionId}:`, err);
                        updatePeerStatus("disconnected");
                    });
                }
            }
        } else {
            if (peers.current) {
                console.log(`🧹 Cleaning up all peers (no active game)`);
                for (const gamePeer of peers.current) {
                    if (gamePeer) {
                        console.log(`🗑️ Destroying peer ${gamePeer.connectionId}`);
                        gamePeer.peer.destroy();
                    }
                }
                peers.current = undefined;
                earlySignals.current = undefined;
                setPeerStatuses(undefined);
                setPeersActualStatuses(undefined);
                setPeerLatencies(undefined);
            }
        }
    }, [game, connectionId, sendSignal, gameToken, handleChannelMessage, sendJsonMessage]);

    const sendData = useCallback((playerIndex: number, data: Uint8Array, reliable: boolean = true): boolean => {
        if (!peers.current) {
            console.warn(`❌ Cannot send data: peers not initialized`);
            return false;
        }

        const peer = peers.current[playerIndex];
        if (!peer) {
            console.warn(`❌ Cannot send data: no peer at index ${playerIndex}`);
            return false;
        }

        if (!peer.peer.connected) {
            console.warn(`❌ Cannot send data to player ${playerIndex}: peer not connected (${peer.connectionId})`);
            return false;
        }

        if (reliable) {
            if (peer.reliableChannel?.readyState !== "open") {
                console.warn(
                    `❌ Cannot send data to player ${playerIndex}: reliable channel not open (state: ${peer.reliableChannel?.readyState}) (${peer.connectionId})`,
                );
                return false;
            }
        } else {
            if (peer.unreliableChannel?.readyState !== "open") {
                console.warn(
                    `❌ Cannot send data to player ${playerIndex}: unreliable channel not open (state: ${peer.unreliableChannel?.readyState}) (${peer.connectionId})`,
                );
                return false;
            }
        }

        try {
            const envelope = createEnvelope(MESSAGE_TYPES.GAME_BINARY, data);
            if (reliable) {
                peer.reliableChannel!.send(new Uint8Array(envelope));
            } else {
                peer.unreliableChannel!.send(new Uint8Array(envelope));
            }
            return true;
        } catch (error) {
            console.error(`❌ Failed to send data to player ${playerIndex} (${peer.connectionId}):`, error);
            return false;
        }
    }, []);

    return {
        peerStatuses,
        peersActualStatuses,
        peerLatencies,
        sendData,
    };
};
