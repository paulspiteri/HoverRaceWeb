const getQueryParam = (name) => {
    const urlParams = new URLSearchParams(window.location.search);
    return urlParams.get(name);
};

const myPeerId = getQueryParam('id');
const peer = new Peer(myPeerId);

// Store the active peer connection for sending messages
let activePeerConnection = null;

// Shared data handling function
function handleReceivedData(data) {
    // Convert data to Uint8Array for C++ processing
    let binaryData = null;
    
    if (data instanceof Uint8Array) {
        binaryData = data;
    } else if (data instanceof ArrayBuffer) {
        binaryData = new Uint8Array(data);
    } else {
        console.error(`[JS] Received unsupported data format:`, typeof data);
        return;
    }
    
    // Call C++ function to handle received message
    // Allocate memory in Emscripten heap
    const dataPtr = Module._malloc(binaryData.length);
    Module.HEAPU8.set(binaryData, dataPtr);
    
    // Call C++ function
    Module._ReceivePeerMessage(dataPtr, binaryData.length);
    
    // Free the allocated memory
    Module._free(dataPtr);
}

peer.on('open', function(id) {
    console.log(`Peer initialized with ID: ${id}`);
    const peersToConnect = getQueryParam('peers');
    if (peersToConnect) {
        connectToPeers(peersToConnect);
    }
});

peer.on('connection', function(conn) {
    console.log(`Incoming connection from: ${conn.peer}`);
    activePeerConnection = conn;

    conn.on('data', handleReceivedData);

    startGame(0); // Host player ID
});

peer.on('error', function(err) {
    console.log(`Peer error: ${err}`);
});


const connectToPeers = (peerIds) => {
    if (!peerIds) return;
    const peers = peerIds.split(';').filter(id => id.trim());
    peers.forEach(peerId => {
        const conn = peer.connect(peerId.trim());

        conn.on('open', function() {
            console.log(`Connected to peer: ${peerId}`);
            activePeerConnection = conn;
            
            startGame(1);  // Client player ID - only makes sense for 1 peer
        });

        conn.on('error', function(err) {
            console.log(`Failed to connect to ${peerId}: ${err}`);
        });

        conn.on('data', handleReceivedData);
    });
};

// Function called from C++ to send game messages to peer
function sendGameMessage(dataArray) {
    if (!activePeerConnection) {
        console.error('No active peer connection for sending message');
        return false;
    }
    
    try {
        // dataArray is already a Uint8Array from Emscripten
        // Send the binary data to peer
        activePeerConnection.send(dataArray);
        return true;
    } catch (error) {
        console.error('Failed to send message to peer:', error);
        return false;
    }
}

