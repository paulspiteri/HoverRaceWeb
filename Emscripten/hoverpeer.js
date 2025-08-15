const getQueryParam = (name) => {
    const urlParams = new URLSearchParams(window.location.search);
    return urlParams.get(name);
};

const myPeerId = getQueryParam('id');
const peer = new Peer(myPeerId);

peer.on('open', function(id) {
    console.log(`Peer initialized with ID: ${id}`);
    const peersToConnect = getQueryParam('peers');
    if (peersToConnect) {
        connectToPeers(peersToConnect);
    }
});

peer.on('connection', function(conn) {
    console.log(`Incoming connection from: ${conn.peer}`);

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

