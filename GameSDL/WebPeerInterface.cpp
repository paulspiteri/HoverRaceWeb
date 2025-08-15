#include "WebPeerInterface.h"
#include "../Util/nomfc_stdafx.h"
#include <algorithm>
#include <cstring>

WebPeerInterface::WebPeerInterface()
{
    ASSERT(MR_NET_HEADER_LEN == 3);

    mPlayer = "Unknown Player!";
    mId = 0;
    mIsConnected = false;
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
        return false; // Not supported yet
    }

    // Calculate total message size
    int lToSend = MR_NET_HEADER_LEN + pMessage->mDataLen;
    int pQueueId = pLongPort ? 0 : 1;

    // ENet handles sequencing internally, so we don't need custom datagram numbers
    // Just clear the fields since they're not needed with ENet
    pMessage->mDatagramQueue = pQueueId;
    pMessage->mDatagramNumber = 0;

    // Create ENet packet from the message buffer
    // ENetPacket* packet = enet_packet_create(pMessage, lToSend,
    //                                         pLongPort ? ENET_PACKET_FLAG_RELIABLE : ENET_PACKET_FLAG_UNSEQUENCED);
    //
    // if (!packet)
    // {
    //     return false;
    // }
    //
    // int result = enet_peer_send(mConnectedPeer, pQueueId, packet);

    // return (result == 0);
    return true;
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
