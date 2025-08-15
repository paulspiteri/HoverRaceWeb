#pragma once

#include <string>

#include "../Util/MR_Types.h"

#define MR_DEFAULT_NET_PORT  9530
#define MR_MAX_NET_MESSAGE_LEN 255

#define MR_NET_REQUIRED         1
#define MR_NET_TRY              2
#define MR_NOT_REQUIRED         0
#define MR_NET_DATAGRAM        -1

class MR_NetMessageBuffer
{
public:
    MR_UInt16  mDatagramNumber:8;  // used only for datagram
    MR_UInt16  mDatagramQueue:2;   // used only for datagram
    MR_UInt16  mMessageType:6;
    MR_UInt8   mDataLen;
    MR_UInt8   mData[ MR_MAX_NET_MESSAGE_LEN ];
};

#define MR_NET_HEADER_LEN  (sizeof( MR_NetMessageBuffer )-MR_MAX_NET_MESSAGE_LEN)

class WebPeerInterface
{
public:
    enum
    {
        eMaxClient = 8
    };

private:
    std::string  mPlayer;
    int          mId;
    std::string  mClientName[ eMaxClient ];
    bool         mIsConnected;
    
public:
    WebPeerInterface();
    ~WebPeerInterface();

    void  SetPlayerName( const char* pPlayerName );
    const char* GetPlayerName()const;

    bool MasterConnect( const char* pGameName, bool pPromptForPort = true, unsigned pDefaultPort = MR_DEFAULT_NET_PORT, int pReturnMessage = 0);
    bool SlavePreConnect( std::string& pGameName );
    bool SlaveConnect( const char* pServerIP=nullptr, unsigned pPort = MR_DEFAULT_NET_PORT, const char* pGameName = nullptr, int pReturnMessage = 0 );

    void Disconnect();

    int  GetClientCount()const;
    int  GetId()const;

    int  GetLagFromServer()const;
    int  GetAvgLag( int pClient )const;
    int  GetMinLag( int pClient )const;

    bool UDPSend( int pClient, MR_NetMessageBuffer* pMessage, bool pLongPort, bool pResendLast = false ); // return TRUE if queue not full
    bool BroadcastMessage( MR_NetMessageBuffer* pMessage, int pReqLevel );
    // BOOL BroadcastMessage( DWORD  pTimeStamp, int  pMessageType, int pMessageLen, const MR_UInt8* pMessage );
    bool FetchMessage( int& pMessageType, int& pMessageLen, const MR_UInt8*& pMessage, int& pClientId, MR_NetMessageBuffer& pBuffer ); // Caller provides buffer

    const char* GetPlayerName( int pIndex )const;
    bool        IsConnected( int pIndex )const;

};

