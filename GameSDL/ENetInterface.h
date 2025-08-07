#ifndef ENETINTERFACE_H
#define ENETINTERFACE_H

#include "../Util/MR_Types.h"

#define MR_DEFAULT_NET_PORT  9530
#define MR_MAX_NET_MESSAGE_LEN 255


class MR_NetMessageBuffer
{
public:
    MR_UInt16  mDatagramNumber:8;  // used only for datagram
    MR_UInt16  mDatagramQueue:2;   // used only for datagram
    MR_UInt16  mMessageType:6;
    MR_UInt8   mDataLen;
    MR_UInt8   mData[ MR_MAX_NET_MESSAGE_LEN ];
};

class ENetInterface
{
public:
    enum
    {
        eMaxClient = 8
     };

    void  SetPlayerName( const char* pPlayerName );
    const char* GetPlayerName()const;

    void Disconnect();

    int  GetClientCount()const;
    int  GetId()const;

    int  GetLagFromServer()const;
    int  GetAvgLag( int pClient )const;
    int  GetMinLag( int pClient )const;

    bool UDPSend( int pClient, MR_NetMessageBuffer* pMessage, bool pLongPort, bool pResendLast = false ); // return TRUE if queue not full
    bool BroadcastMessage( MR_NetMessageBuffer* pMessage, int pReqLevel );
    // BOOL BroadcastMessage( DWORD  pTimeStamp, int  pMessageType, int pMessageLen, const MR_UInt8* pMessage );
    bool FetchMessage(     uint32_t& pTimeStamp, int& pMessageType, int& pMessageLen, const MR_UInt8*& pMessage, int& pClientId ); // pTimeStamp must be set to current time stamp before fetch

    const char* GetPlayerName( int pIndex )const;
    bool        IsConnected( int pIndex )const;

};

#endif //ENETINTERFACE_H
