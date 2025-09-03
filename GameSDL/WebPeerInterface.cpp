#include "WebPeerInterface.h"
#include "../Util/nomfc_stdafx.h"
#include <algorithm>
#include <cstring>
#include <queue>

// Global message queue for peer messages
struct ReceivedMessage {
    int clientId;
    MR_NetMessageBuffer buffer;

    ReceivedMessage(int clientId, const MR_UInt8* data, int length) : clientId(clientId) {
        int copyLen = std::min(length, static_cast<int>(sizeof(buffer)));
        memcpy(&buffer, data, copyLen);
    }
};

static std::queue<ReceivedMessage> g_messageQueue;

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

static bool SendPeerMessage(int clientId, const char* data, int length, bool reliable) {
    // Use EM_ASM to call JavaScript directly with binary data
    int result = EM_ASM_INT({
        // Create a new Uint8Array and copy the data
        var dataArray = new Uint8Array($2);
        for (var i = 0; i < $2; i++) {
            dataArray[i] = HEAPU8[$1 + i];
        }
        
        return sendGameMessage($0, dataArray, $3) ? 1 : 0;
    }, clientId, data, length, reliable);
    
    return result != 0;
}

extern "C" void ReceivePeerMessage(int playerId, const char* data, int length) {
    if (length >= MR_NET_HEADER_LEN && length <= static_cast<int>(sizeof(MR_NetMessageBuffer))) {
        ReceivedMessage msg(playerId, reinterpret_cast<const MR_UInt8*>(data), length);
        g_messageQueue.push(msg);
    }
}

#endif

WebPeerInterface::WebPeerInterface(int playerId, std::array<PeerStatus, eMaxClient> peers)
{
    ASSERT(MR_NET_HEADER_LEN == 3);

    mPlayer = "Unknown Player!";
    mId = playerId;
    mPeers = peers;
    // mServerMode       = FALSE;
    // mServerPort       = 0;
    //
    // mAllPreLoguedRecv = FALSE;
    //
    // for( int lCounter = 0; lCounter < eMaxClient; lCounter++ )
    // {
    //     mPreLoguedClient[ lCounter ]            = FALSE;
    //     mConnected[ lCounter ]                  = FALSE;
    //     mCanBePreLogued[ lCounter ]             = FALSE;
    // }
}

WebPeerInterface::~WebPeerInterface()
{
}

void WebPeerInterface::SetPlayerName(const char* pPlayerName)
{
    mPlayer = pPlayerName;
}

const char* WebPeerInterface::GetPlayerName() const
{
    return mPlayer.c_str();
}

void WebPeerInterface::Disconnect()
{
    std::ranges::for_each(mPeers, [](auto& peer) { peer.isConnected = false; });
}

void WebPeerInterface::DisconnectPlayer(int pIndex)
{
    mPeers[pIndex].isConnected = false;
}

int WebPeerInterface::GetClientCount() const
{
    return std::ranges::count_if(mPeers, [](const auto& peer) { return peer.isConnected; });
}

int WebPeerInterface::GetId() const
{
    //ASSERT( (mId!=0)||mServerMode );

    return mId;
}

int WebPeerInterface::GetLagFromServer() const
{
    std::cout << "GetLagFromServer " << mPeers[0].minLatency << "ms" << std::endl;
    return mPeers[0].minLatency / 2;
}

int WebPeerInterface::GetAvgLag(int pClient) const
{
    return mPeers[pClient].avgLatency / 2;
}

int WebPeerInterface::GetMinLag(int pClient) const
{
    return mPeers[pClient].minLatency / 2;
}

bool WebPeerInterface::UDPSend(int pClient, MR_NetMessageBuffer* pMessage, bool pLongPort, bool pResendLast)
{
    if (!mPeers[pClient].isConnected)
    {
        return true;    // false means buffer full in the old world
    }

    // Calculate total message size
    int lToSend = MR_NET_HEADER_LEN + pMessage->mDataLen;
    
    // Set datagram info (for compatibility with original protocol)
    int pQueueId = pLongPort ? 0 : 1;
    pMessage->mDatagramQueue = pQueueId;
    pMessage->mDatagramNumber = 0;

#ifdef __EMSCRIPTEN__
    SendPeerMessage(pClient, reinterpret_cast<const char*>(pMessage), lToSend, false);
#endif
    return true;
}

bool WebPeerInterface::BroadcastMessage(MR_NetMessageBuffer* pMessage, int pReqLevel)
{
    for( int lCounter = 0; lCounter < eMaxClient; lCounter++ )
    {
        if (pReqLevel == MR_NET_DATAGRAM)
        {
            UDPSend(lCounter, pMessage, false, false);
        }
        else
        {
            // todo
            // return Send( pMessage, pReqLevel );
           UDPSend(lCounter, pMessage, false, false);
        }
    }
    return TRUE;
}

bool WebPeerInterface::FetchMessage(int& pMessageType, int& pMessageLen, const MR_UInt8*& pMessage, int& pClientId, MR_NetMessageBuffer& pBuffer)
{
    if (g_messageQueue.empty()) {
        return false;
    }

    // Get the next message from the queue
    ReceivedMessage receivedMsg = g_messageQueue.front();
    g_messageQueue.pop();

    // Copy the message to the caller's buffer
    pBuffer = receivedMsg.buffer;
    
    // Set output parameters
    pMessage = pBuffer.mData;
    pMessageLen = pBuffer.mDataLen;
    pMessageType = pBuffer.mMessageType;
    pClientId = receivedMsg.clientId;
    
    return true;
}

const char* WebPeerInterface::GetPlayerName(int pIndex) const
{
    const char* lReturnValue = NULL;

    if (pIndex < 0)
    {
        lReturnValue = mPlayer.c_str();
    }
    else
    {
        if (pIndex < eMaxClient)
        {
            lReturnValue = mPeers[pIndex].name.c_str();
        }
    }
    return lReturnValue;
}

bool WebPeerInterface::IsConnected(int pIndex) const
{
    if (pIndex == -1)
    {
        return true;    // self
    }
    return mPeers.at(pIndex).isConnected;
}
