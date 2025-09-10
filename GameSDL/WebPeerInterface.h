#pragma once

#include <array>
#include <string>

#include "../Util/MR_Types.h"

#define MR_MAX_NET_MESSAGE_LEN 255

#define MR_NET_REQUIRED         1
#define MR_NET_TRY              2
#define MR_NOT_REQUIRED         0
#define MR_NET_DATAGRAM        (-1)

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

struct PeerStatus {
    bool isConnected;
    int minLatency;
    int avgLatency;
    std::string name;
    std::uint8_t lastSentDatagramNumber;
    std::uint8_t lastRecievedDatagramNumber;

    PeerStatus() : isConnected(false), minLatency(0), avgLatency(0), name("Unknown Player"), lastSentDatagramNumber(0),
                   lastRecievedDatagramNumber(0)
    {
    }
};

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
    std::array<PeerStatus, eMaxClient> mPeers;

public:
    WebPeerInterface(int playerId, const std::array<PeerStatus, eMaxClient>& peers);

    void  SetPlayerName( const char* pPlayerName );
    const char* GetPlayerName()const;

    void Disconnect();
    void DisconnectPlayer(int pIndex);

    int  GetClientCount()const;
    int  GetId()const;

    int  GetLagFromServer()const;
    int  GetAvgLag( int pClient )const;
    int  GetMinLag( int pClient )const;

    bool UDPSend( int pClient, MR_NetMessageBuffer* pMessage, bool pResendLast = false ); // return TRUE if queue not full
    bool ReqSend( int pClient, MR_NetMessageBuffer* pMessage);
    bool BroadcastMessage( MR_NetMessageBuffer* pMessage, int pReqLevel );
    bool FetchMessage( int& pMessageType, int& pMessageLen, const MR_UInt8*& pMessage, int& pClientId, MR_NetMessageBuffer& pBuffer ); // Caller provides buffer

    const char* GetPlayerName( int pIndex )const;
    bool        IsConnected( int pIndex )const;

};

