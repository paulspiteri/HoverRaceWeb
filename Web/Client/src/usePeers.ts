import { useCallback, useEffect, useRef, useState } from "react";
import type { JoinedGame, ServerMessage, SignalMessage } from "@/types.ts";
import type { PeerConnectionStatusMessage, PeerMessage } from "@/peerTypes.ts";
import { MESSAGE_TYPES, createEnvelope, parseEnvelope, createJsonEnvelope } from "@/peerMessage.ts";
import SimplePeer from "simple-peer";

export interface GamePeer {
    peer: SimplePeer.Instance;
    connectionId: string;
    unreliableChannel?: RTCDataChannel;
}

// Utility function to access the internal RTCPeerConnection
export const getPeerConnection = (peer: SimplePeer.Instance): RTCPeerConnection => {
    return (peer as object as { _pc: RTCPeerConnection })._pc;
};

export const usePeers = (
    connectionId: string | undefined,
    game: JoinedGame | undefined,
    eventSource: EventSource | undefined,
    sendSignal: (gameId: string, targetConnectionId: string, gameToken: string, signalData: string) => Promise<void>,
    gameToken: string | undefined,
    onGameData: (playerIndex: number, data: unknown) => void,
) => {
    const peers = useRef<(GamePeer | undefined)[] | undefined>(undefined);
    const [peerStatuses, setPeerStatuses] = useState<("connecting" | "connected" | "disconnected" | undefined)[]>();
    const [peersActualStatuses, setPeersActualStatuses] = useState<PeerConnectionStatusMessage[]>(); // only used at the host

    // Send status update to HOST peer
    const sendStatusUpdateToHost = useCallback(() => {
        if (!game || !peers.current || !peerStatuses) {
            return;
        }

        const hostPeer = peers.current?.find((p) => p?.connectionId === game.creatorConnectionId);
        if (!hostPeer) {
            return;
        }

        const statusMessage: PeerConnectionStatusMessage = {
            type: "peerConnectionStatus",
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

        try {
            if (hostPeer.peer.connected) {
                const envelope = createJsonEnvelope(statusMessage);
                hostPeer.peer.send(envelope);
                console.log("📊 Sent status update to host (JSON envelope):", statusMessage);
            }
        } catch (error) {
            console.error("❌ Failed to send status update to host:", error);
        }
    }, [connectionId, game, peerStatuses]);

    useEffect(() => sendStatusUpdateToHost(), [peerStatuses, sendStatusUpdateToHost]);

    const onSignalReceived = useCallback((signal: SignalMessage) => {
        console.log(`📡 Received signal from ${signal.fromConnectionId}`);
        const gamePeer = peers.current?.find((x) => x && x.connectionId === signal.fromConnectionId);
        if (gamePeer) {
            gamePeer.peer.signal(signal.signalData);
        } else {
            console.warn(`❌ No peer found for signal from ${signal.fromConnectionId}`);
        }
    }, []);

    const handleChannelMessage = useCallback(
        (data: Uint8Array, playerConnectionId: string, playerIndex: number) => {
            if (data.length >= 4) {
                try {
                    const { messageType, data: payload } = parseEnvelope(data);

                    if (messageType === MESSAGE_TYPES.BINARY) {
                        onGameData(playerIndex, payload);
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
        [onGameData],
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
        if (game) {
            const myPlayerIndex = game.players.findIndex((x) => x?.connectionId === connectionId);
            if (myPlayerIndex === -1) {
                throw new Error("Current connection not found in game players");
            }

            if (!peers.current) {
                peers.current = new Array<GamePeer | undefined>(game.maxPlayers);
            }

            setPeerStatuses((prev) => {
                if (!prev || prev.length !== game.maxPlayers) {
                    return new Array(game.maxPlayers).fill(undefined);
                }
                return prev;
            });

            setPeersActualStatuses((prev) => {
                if (!prev || prev.length !== game.maxPlayers) {
                    return new Array(game.maxPlayers).fill(undefined);
                }
                return prev;
            });

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

                    const newPeer: GamePeer = {
                        connectionId: playerConnectionId,
                        peer: new SimplePeer({ initiator: isInitiator, trickle: false }),
                    };
                    peers.current[i] = newPeer;

                    setPeerStatuses((prev) => {
                        const updated = [...(prev || [])];
                        updated[i] = "connecting";
                        return updated;
                    });

                    newPeer.peer.on("signal", (data: SimplePeer.SignalData) => {
                        console.log(`📤 Sending signal to ${playerConnectionId}`);
                        if (game && gameToken) {
                            sendSignal(game.id, playerConnectionId, gameToken, JSON.stringify(data));
                        }
                    });

                    const updatePeerStatus = (status: "connected" | "disconnected") => {
                        setPeerStatuses((prev) => {
                            const updated = [...(prev || [])];
                            // Find the current index of this peer (don't rely on closure)
                            const currentIndex = peers.current?.findIndex(
                                (p) => p?.connectionId === playerConnectionId,
                            );
                            if (currentIndex !== undefined && currentIndex >= 0) {
                                updated[currentIndex] = status;
                            }
                            return updated;
                        });
                    };

                    const handleDataChannelMessage = (
                        data: unknown,
                        playerConnectionId: string,
                        playerIndex: number,
                    ) => {
                        const uint8Data = new Uint8Array(data as ArrayBuffer);
                        handleChannelMessage(uint8Data, playerConnectionId, playerIndex);
                    };

                    newPeer.peer.on("connect", () => {
                        console.log(`🔗 Connected to peer ${playerConnectionId}`);

                        const pc = getPeerConnection(newPeer.peer);
                        if (isInitiator) {
                            newPeer.unreliableChannel = pc.createDataChannel("unreliable", {
                                ordered: false,
                                maxRetransmits: 0,
                            });
                            console.log(`📡 Created unreliable channel for ${playerConnectionId}`);

                            newPeer.unreliableChannel.onmessage = (event) =>
                                handleDataChannelMessage(event.data, playerConnectionId, i);

                            updatePeerStatus("connected");
                        } else {
                            pc.ondatachannel = (event) => {
                                const channel = event.channel;
                                console.log(`📡 Received data channel: ${channel.label} from ${playerConnectionId}`);

                                if (channel.label === "unreliable") {
                                    newPeer.unreliableChannel = channel;
                                }

                                channel.onmessage = (event) =>
                                    handleDataChannelMessage(event.data, playerConnectionId, i);

                                updatePeerStatus("connected");
                            };
                        }
                    });

                    newPeer.peer.on("close", () => {
                        console.log(`🔌 Peer disconnected: ${playerConnectionId}`);
                        updatePeerStatus("disconnected");
                    });

                    newPeer.peer.on("error", (err: Error) => {
                        console.error(`❌ Peer error with ${playerConnectionId}:`, err);
                        updatePeerStatus("disconnected");
                    });

                    newPeer.peer.on("data", (data: ArrayBuffer) => {
                        handleDataChannelMessage(data, playerConnectionId, i);
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
                setPeerStatuses(undefined);
            }
        }
    }, [game, connectionId, sendSignal, gameToken, onGameData, handleChannelMessage]);

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
            // Reliable channel is always available via SimplePeer default
        } else {
            if (peer.unreliableChannel?.readyState !== "open") {
                console.warn(
                    `❌ Cannot send data to player ${playerIndex}: unreliable channel not open (state: ${peer.unreliableChannel?.readyState}) (${peer.connectionId})`,
                );
                return false;
            }
        }

        try {
            const envelope = createEnvelope(MESSAGE_TYPES.BINARY, data);

            if (reliable) {
                peer.peer.send(envelope);
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
        sendData,
    };
};
