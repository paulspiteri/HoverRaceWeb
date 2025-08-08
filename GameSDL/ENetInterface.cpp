#include "ENetInterface.h"
#include "../Util/nomfc_stdafx.h"

ENetInterface::ENetInterface()
{
    ASSERT( MR_NET_HEADER_LEN == 3 );

    mPlayer           = "Unknown Player!";
    mId               = 0;
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
    return false;
}

bool ENetInterface::SlavePreConnect(std::string& pGameName)
{
    return false;
}

bool ENetInterface::SlaveConnect(const char* pServerIP, unsigned pPort, const char* pGameName, int pReturnMessage)
{
    return false;
}

void ENetInterface::Disconnect()
{
}

int ENetInterface::GetClientCount() const
{
    return 0;
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
