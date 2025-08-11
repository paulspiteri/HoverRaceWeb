#include "ENetInterface.h"
#include "../Util/nomfc_stdafx.h"
#include <algorithm>
#include <cstring>

ENetInterface::ENetInterface()
{
    ASSERT(MR_NET_HEADER_LEN == 3);

    if (enet_initialize() != 0)
    {
        // Handle initialization error
    }

    mPlayer = "Unknown Player!";
    mId = 0;
    mENet = nullptr;
    mConnectedPeer = nullptr;
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

ENetInterface::~ENetInterface()
{
    Disconnect();
    enet_deinitialize();
}

void ENetInterface::SetPlayerName(const char* pPlayerName)
{
    mPlayer = pPlayerName;
}

const char* ENetInterface::GetPlayerName() const
{
    return mPlayer.c_str();
}

bool ENetInterface::WaitForConnection(int timeoutSeconds)
{
    if (!mENet)
    {
        return false;
    }

    ENetEvent event;
    bool connected = false;
    int attempts = timeoutSeconds * 10; // 100ms intervals

    while (attempts-- > 0 && !connected)
    {
        int result = enet_host_service(mENet, &event, 100);
        if (result > 0)
        {
            switch (event.type)
            {
            case ENET_EVENT_TYPE_CONNECT:
                mConnectedPeer = event.peer;
                mIsConnected = true;
                connected = true;
                printf("ENet: Connection established\n");
                break;

            case ENET_EVENT_TYPE_DISCONNECT:
                printf("ENet: Peer disconnected during connection\n");
                break;

            default:
                break;
            }
        }
    }

    if (!connected)
    {
        printf("ENet: Connection timeout after %d seconds\n", timeoutSeconds);
        enet_host_destroy(mENet);
        mENet = nullptr;
        return false;
    }

    return true;
}

bool ENetInterface::MasterConnect(const char* pGameName, bool pPromptForPort, unsigned pDefaultPort, int pReturnMessage)
{
    if (mENet)
    {
        return false;
    }

    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = pDefaultPort;

    mENet = enet_host_create(&address, 1, 2, 0, 0);
    if (mENet == nullptr)
    {
        printf("ENet: Failed to create server host\n");
        return false;
    }

    mId = 0; // Server is always ID 0
    printf("ENet: Server started listening on port %u, waiting for client...\n", pDefaultPort);

    return WaitForConnection(30); // 30 second timeout
}

bool ENetInterface::SlavePreConnect(std::string& pGameName)
{
    return false;
}

bool ENetInterface::SlaveConnect(const char* pServerIP, unsigned pPort, const char* pGameName, int pReturnMessage)
{
    if (mENet)
    {
        return false;
    }

    mENet = enet_host_create(nullptr, 1, 2, 0, 0);
    if (mENet == nullptr)
    {
        return false;
    }

    ENetAddress address;
    if (enet_address_set_host(&address, pServerIP) < 0)
    {
        printf("ENet: Failed to resolve server address: %s\n", pServerIP);
        enet_host_destroy(mENet);
        mENet = nullptr;
        return false;
    }
    address.port = pPort;
    printf("ENet: Attempting to connect to %s:%u\n", pServerIP, pPort);

    ENetPeer* peer = enet_host_connect(mENet, &address, 2, 0);
    if (peer == nullptr)
    {
        enet_host_destroy(mENet);
        mENet = nullptr;
        return false;
    }

    mId = 1; // Client is always ID 1
    return WaitForConnection(5); // 5 second timeout
}

void ENetInterface::Disconnect()
{
    if (mENet)
    {
        if (mConnectedPeer)
        {
            enet_peer_disconnect_now(mConnectedPeer, 0);
            mConnectedPeer = nullptr;
        }
        mIsConnected = false;

        enet_host_destroy(mENet);
        mENet = nullptr;
    }
}

int ENetInterface::GetClientCount() const
{
    return mIsConnected ? 1 : 0;
}

int ENetInterface::GetId() const
{
    //ASSERT( (mId!=0)||mServerMode );

    return mId;
}

int ENetInterface::GetLagFromServer() const
{
    return 0;
}

int ENetInterface::GetAvgLag(int pClient) const
{
    return 0;
}

int ENetInterface::GetMinLag(int pClient) const
{
    return 0;
}

bool ENetInterface::UDPSend(int pClient, MR_NetMessageBuffer* pMessage, bool pLongPort, bool pResendLast)
{
    // For single-peer implementation, ignore pClient parameter
    if (!mENet || !mConnectedPeer || !mIsConnected)
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
    ENetPacket* packet = enet_packet_create(pMessage, lToSend,
                                            pLongPort ? ENET_PACKET_FLAG_RELIABLE : ENET_PACKET_FLAG_UNSEQUENCED);

    if (!packet)
    {
        return false;
    }

    int result = enet_peer_send(mConnectedPeer, pQueueId, packet);

    return (result == 0);
}

bool ENetInterface::BroadcastMessage(MR_NetMessageBuffer* pMessage, int pReqLevel)
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

bool ENetInterface::FetchMessage(int& pMessageType, int& pMessageLen, const MR_UInt8*& pMessage, int& pClientId, MR_NetMessageBuffer& pBuffer)
{
    if (!mENet)
    {
        return false;
    }

    ENetEvent event;
    while (enet_host_service(mENet, &event, 0) > 0)
    {
        switch (event.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
            {
                // Already have a connection, disconnect new peer
                enet_peer_disconnect_now(event.peer, 0);
                printf("ENet: Rejected connection - already connected\n");
            }
            break;

        case ENET_EVENT_TYPE_DISCONNECT:
            if (event.peer == mConnectedPeer)
            {
                mConnectedPeer = nullptr;
                mIsConnected = false;
                printf("ENet: Client disconnected\n");
            }
            break;

        case ENET_EVENT_TYPE_RECEIVE:
            {
                if (event.packet->dataLength >= MR_NET_HEADER_LEN)
                {
                    // Copy packet data to caller-provided buffer so it survives packet destruction
                    memcpy(&pBuffer, event.packet->data, 
                           std::min(event.packet->dataLength, sizeof(pBuffer)));
                    
                    // Set output parameters to point to caller's buffer
                    pMessage = pBuffer.mData;
                    pMessageLen = pBuffer.mDataLen;
                    pMessageType = pBuffer.mMessageType;
                    pClientId =0;// mId == 0 ? 1 : 0; // Server sees client as ID 1, client sees server as ID 0
                    
                    // Now safe to destroy the packet
                    enet_packet_destroy(event.packet);
                    return true;
                }
                
                // Clean up packet if invalid
                enet_packet_destroy(event.packet);
            }
            break;
        }
    }

    return false;
}

const char* ENetInterface::GetPlayerName(int pIndex) const
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

bool ENetInterface::IsConnected(int pIndex) const
{
    return pIndex == 0 && mIsConnected;
}
