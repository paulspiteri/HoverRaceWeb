#include "WebPeerInterface.h"
#include "../Util/nomfc_stdafx.h"
#include <algorithm>
#include <cstring>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

static bool SendPeerMessage(const char* data, int length) {
    // Use EM_ASM to call JavaScript directly with binary data
    int result = EM_ASM_INT({
        // Create a new Uint8Array and copy the data
        var dataArray = new Uint8Array($1);
        for (var i = 0; i < $1; i++) {
            dataArray[i] = HEAPU8[$0 + i];
        }
        
        // Call our sendGameMessage function with the binary data
        return sendGameMessage(dataArray) ? 1 : 0;
    }, data, length);
    
    printf("SendPeerMessage: sent %d bytes, result=%d\n", length, result);
    return result != 0;
}
#endif

WebPeerInterface::WebPeerInterface()
{
    ASSERT(MR_NET_HEADER_LEN == 3);

    mPlayer = "Unknown Player!";
    mId = 0;
    mIsConnected = true;    //                      I SET TO TRUE BY DEFAULT
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

bool WebPeerInterface::MasterConnect(const char* pGameName, bool pPromptForPort, unsigned pDefaultPort, int pReturnMessage)
{
  return true;
}

bool WebPeerInterface::SlavePreConnect(std::string& pGameName)
{
    return false;
}

bool WebPeerInterface::SlaveConnect(const char* pServerIP, unsigned pPort, const char* pGameName, int pReturnMessage)
{
  return true;
}

void WebPeerInterface::Disconnect()
{
    mIsConnected = false;
}

int WebPeerInterface::GetClientCount() const
{
    return mIsConnected ? 1 : 0;
}

int WebPeerInterface::GetId() const
{
    //ASSERT( (mId!=0)||mServerMode );

    return mId;
}

int WebPeerInterface::GetLagFromServer() const
{
    return 0;
}

int WebPeerInterface::GetAvgLag(int pClient) const
{
    return 0;
}

int WebPeerInterface::GetMinLag(int pClient) const
{
    return 0;
}

bool WebPeerInterface::UDPSend(int pClient, MR_NetMessageBuffer* pMessage, bool pLongPort, bool pResendLast)
{
    if (!mIsConnected)
    {
        return false;
    }

    if (pClient != 0)
    {
        return false; // Only single peer (client 0) supported
    }

    // Calculate total message size
    int lToSend = MR_NET_HEADER_LEN + pMessage->mDataLen;
    
    // Set datagram info (for compatibility with original protocol)
    int pQueueId = pLongPort ? 0 : 1;
    pMessage->mDatagramQueue = pQueueId;
    pMessage->mDatagramNumber = 0;

#ifdef __EMSCRIPTEN__
    std::cout << "WebPeerInterface::UDPSend sending via PeerJS " << lToSend << " bytes" << std::endl;
    // Send via PeerJS using our bridge function
    bool result = SendPeerMessage(reinterpret_cast<const char*>(pMessage), lToSend);
    if (!result) {
        printf("WebPeerInterface::UDPSend failed to send %d bytes\n", lToSend);
    }
    return result;
#else
    // Non-Emscripten builds don't support peer messaging yet
    printf("WebPeerInterface::UDPSend not implemented for non-Emscripten builds\n");
    return false;
#endif
}

bool WebPeerInterface::BroadcastMessage(MR_NetMessageBuffer* pMessage, int pReqLevel)
{
    if (pReqLevel == MR_NET_DATAGRAM)
    {
        return UDPSend(0, pMessage, false, false);
    }
    else
    {
        // todo
        // return Send( pMessage, pReqLevel );
        return UDPSend(0, pMessage, false, false);
    }
}

bool WebPeerInterface::FetchMessage(int& pMessageType, int& pMessageLen, const MR_UInt8*& pMessage, int& pClientId, MR_NetMessageBuffer& pBuffer)
{
    // if (!mENet)
    // {
    //     return false;
    // }
    //
    // ENetEvent event;
    // while (enet_host_service(mENet, &event, 0) > 0)
    // {
    //     switch (event.type)
    //     {
    //     case ENET_EVENT_TYPE_CONNECT:
    //         {
    //             // Already have a connection, disconnect new peer
    //             enet_peer_disconnect_now(event.peer, 0);
    //             printf("ENet: Rejected connection - already connected\n");
    //         }
    //         break;
    //
    //     case ENET_EVENT_TYPE_DISCONNECT:
    //         if (event.peer == mConnectedPeer)
    //         {
    //             mConnectedPeer = nullptr;
    //             mIsConnected = false;
    //             printf("ENet: Client disconnected\n");
    //         }
    //         break;
    //
    //     case ENET_EVENT_TYPE_RECEIVE:
    //         {
    //             if (event.packet->dataLength >= MR_NET_HEADER_LEN)
    //             {
    //                 // Copy packet data to caller-provided buffer so it survives packet destruction
    //                 memcpy(&pBuffer, event.packet->data,
    //                        std::min(event.packet->dataLength, sizeof(pBuffer)));
    //
    //                 // Set output parameters to point to caller's buffer
    //                 pMessage = pBuffer.mData;
    //                 pMessageLen = pBuffer.mDataLen;
    //                 pMessageType = pBuffer.mMessageType;
    //                 pClientId =0;// mId == 0 ? 1 : 0; // Server sees client as ID 1, client sees server as ID 0
    //
    //                 // Now safe to destroy the packet
    //                 enet_packet_destroy(event.packet);
    //                 return true;
    //             }
    //
    //             // Clean up packet if invalid
    //             enet_packet_destroy(event.packet);
    //         }
    //         break;
    //     }
    // }

    return false;
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
            lReturnValue = mClientName[pIndex].c_str();
        }
    }
    return lReturnValue;
}

bool WebPeerInterface::IsConnected(int pIndex) const
{
    return pIndex == 0 && mIsConnected;
}
