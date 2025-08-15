const getQueryParam = (name) => {
    const urlParams = new URLSearchParams(window.location.search);
    return urlParams.get(name);
};

const myPeerId = getQueryParam('id');
const peer = new Peer(myPeerId);

// Store the active peer connection for sending messages
let activePeerConnection = null;

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

    conn.on('data', function(data) {
        console.log(`Received data from ${conn.peer}: ${data}`);
    });

    startGame();
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
            
            startGame();  // only makes sense for 1 peer
        });

        conn.on('error', function(err) {
            console.log(`Failed to connect to ${peerId}: ${err}`);
        });

        conn.on('data', function(data) {
            console.log(`Received data from ${peerId}: ${data}`);
        });
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
        console.log(`Sent ${dataArray.length} bytes to peer`);
        return true;
    } catch (error) {
        console.error('Failed to send message to peer:', error);
        return false;
    }
}

