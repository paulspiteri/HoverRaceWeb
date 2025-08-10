#include "ENetInterface.h"
#include "../Util/nomfc_stdafx.h"

ENetInterface::ENetInterface()
{
    ASSERT( MR_NET_HEADER_LEN == 3 );

    if (enet_initialize() != 0) {
        // Handle initialization error
    }

    mPlayer           = "Unknown Player!";
    mId               = 0;
    mENet             = nullptr;
    mConnectedPeer    = nullptr;
    mIsConnected      = false;
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

bool ENetInterface::MasterConnect(const char* pGameName, bool pPromptForPort, unsigned pDefaultPort, int pReturnMessage)
{
    if (mENet) {
        return false;
    }

    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = pDefaultPort;

    mENet = enet_host_create(&address, 1, 2, 0, 0);
    if (mENet == nullptr) {
        printf("ENet: Failed to create server host\n");
        return false;
    }

    mId = 0; // Server is always ID 0
    printf("ENet: Server started listening on port %u\n", pDefaultPort);
    return true;
}

bool ENetInterface::SlavePreConnect(std::string& pGameName)
{
    return false;
}

bool ENetInterface::SlaveConnect(const char* pServerIP, unsigned pPort, const char* pGameName, int pReturnMessage)
{
    if (mENet) {
        return false;
    }

    mENet = enet_host_create(nullptr, 1, 2, 0, 0);
    if (mENet == nullptr) {
        return false;
    }

    ENetAddress address;
    if (enet_address_set_host(&address, pServerIP) < 0) {
        printf("ENet: Failed to resolve server address: %s\n", pServerIP);
        enet_host_destroy(mENet);
        mENet = nullptr;
        return false;
    }
    address.port = pPort;
    printf("ENet: Attempting to connect to %s:%u\n", pServerIP, pPort);

    ENetPeer* peer = enet_host_connect(mENet, &address, 2, 0);
    if (peer == nullptr) {
        enet_host_destroy(mENet);
        mENet = nullptr;
        return false;
    }

    ENetEvent event;
    // Wait for connection with timeout handling
    bool connected = false;
    int attempts = 50; // 5 second timeout (50 * 100ms)
    while (attempts-- > 0 && !connected) {
        int result = enet_host_service(mENet, &event, 100);
        if (result > 0) {
            switch (event.type) {
                case ENET_EVENT_TYPE_CONNECT:
                    connected = true;
                    break;
                default:
                    break;
            }
        }
    }

    if (connected) {
        mConnectedPeer = peer;
        mIsConnected = true;
        mId = 1;
        printf("ENet: Successfully connected to server\n");
        return true;
    } else {
        printf("ENet: Connection timeout\n");
        enet_peer_disconnect_now(peer, 0);
        enet_host_destroy(mENet);
        mENet = nullptr;
        return false;
    }
}

void ENetInterface::ProcessNetworkEvents()
{
    if (!mENet) {
        return;
    }
    
    ENetEvent event;
    while (enet_host_service(mENet, &event, 0) > 0) {
        switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
                if (mConnectedPeer == nullptr) {
                    mConnectedPeer = event.peer;
                    mIsConnected = true;
                    printf("ENet: Client connected\n");
                } else {
                    // Already have a connection, disconnect new peer
                    enet_peer_disconnect_now(event.peer, 0);
                    printf("ENet: Rejected connection - already connected\n");
                }
                break;
                
            case ENET_EVENT_TYPE_DISCONNECT:
                if (event.peer == mConnectedPeer) {
                    mConnectedPeer = nullptr;
                    mIsConnected = false;
                    printf("ENet: Client disconnected\n");
                }
                break;
                
            case ENET_EVENT_TYPE_RECEIVE:
                // Handle received packets (for future implementation)
                enet_packet_destroy(event.packet);
                break;
                
            default:
                break;
        }
    }
}

void ENetInterface::Disconnect()
{
    if (mENet) {
        if (mConnectedPeer) {
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
    return false;
}

bool ENetInterface::BroadcastMessage(MR_NetMessageBuffer* pMessage, int pReqLevel)
{
    return false;
}

bool ENetInterface::FetchMessage(uint32_t& pTimeStamp, int& pMessageType, int& pMessageLen, const MR_UInt8*& pMessage,
    int& pClientId)
{
    return false;
}

const char* ENetInterface::GetPlayerName(int pIndex) const
{
    const char* lReturnValue = NULL;

    if( pIndex < 0 )
    {
        lReturnValue = mPlayer.c_str();
    }
    else
    {
        if( pIndex < eMaxClient )
        {
            lReturnValue = mClientName[ pIndex ].c_str();
        }
    }
    return lReturnValue;
}

bool ENetInterface::IsConnected(int pIndex) const
{
    return false;
}
