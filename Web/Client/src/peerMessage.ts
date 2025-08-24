// Envelope types
export const MESSAGE_TYPES = {
    BINARY: 1,
    JSON: 2,
} as const;

export const createEnvelope = (messageType: number, data: Uint8Array): Uint8Array => {
    const envelope = new Uint8Array(4 + data.length);
    const view = new DataView(envelope.buffer);
    view.setUint32(0, messageType, true); // little-endian 4-byte header
    envelope.set(data, 4); // Copy data after header
    return envelope;
};

export const parseEnvelope = (envelope: Uint8Array): { messageType: number; data: Uint8Array } => {
    if (envelope.length < 4) {
        throw new Error("Invalid envelope: too short");
    }
    const view = new DataView(envelope.buffer, envelope.byteOffset);
    const messageType = view.getUint32(0, true); // little-endian
    const data = envelope.slice(4);
    return { messageType, data };
};

export const createJsonEnvelope = (data: object): Uint8Array => {
    const jsonData = new TextEncoder().encode(JSON.stringify(data));
    return createEnvelope(MESSAGE_TYPES.JSON, jsonData);
};
