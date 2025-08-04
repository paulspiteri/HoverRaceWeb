// NetInterface.h
//
//
// Copyright (c) 1995-1998 - Richard Langlois and Grokksoft Inc.
//
// Licensed under GrokkSoft HoverRace SourceCode License v1.0(the "License");
// you may not use this file except in compliance with the License.
//
// A copy of the license should have been attached to the package from which 
// you have taken this file. If you can not find the license you can not use 
// this file.
//
//
// The author makes no representations about the suitability of
// this software for any purpose.  It is provided "as is" "AS IS",
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied.
//
// See the License for the specific language governing permissions 
// and limitations under the License.
//
//

#ifndef NET_INTERFACE_H
#define NET_INTERFACE_H

#include <sys/socket.h>     // Core socket functions
#include <netinet/in.h>     // Internet address family
#include <arpa/inet.h>      // inet_addr(), inet_ntoa()
#include <netdb.h>          // gethostbyname(), gethostbyaddr()
#include <unistd.h>         // close()
#include <fcntl.h>          // fcntl() for non-blocking sockets
#include <string>

#include "../Util/MR_Types.h"


#define MR_DEFAULT_NET_PORT  9530


#define MR_OUT_QUEUE_LEN       2048
#define MR_MAX_NET_MESSAGE_LEN 255

#define MR_NET_REQUIRED         1
#define MR_NET_TRY              2
#define MR_NOT_REQUIRED         0
#define MR_NET_DATAGRAM        -1


using SOCKET = int;

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


class MR_NetworkPort
{
   private:
      SOCKET  mSocket;

      // UDP Information
      SOCKET        mUDPRecvSocket;
      sockaddr_in   mUDPRemoteAddr;

      MR_UInt8      mLastSendedDatagramNumber[4];
      MR_UInt8      mLastReceivedDatagramNumber[4];

      int           mWatchdog;

      // Lag computation
      int     mAvgLag;
      int     mMinLag;
      int     mNbLagTest;
      int     mTotalLag;

      int                 mInputMessageBufferIndex;
      MR_NetMessageBuffer mInputMessageBuffer;

      // Output queue
      // This queue is only used for messages that must absolutly be sent
      // or those that are parially sended
      MR_UInt8            mOutQueue[ MR_OUT_QUEUE_LEN ]; 
      int                 mOutQueueLen;
      int                 mOutQueueHead;

   public:
      MR_NetworkPort();
      ~MR_NetworkPort();

      void Connect( SOCKET pSocket  );
      void SetRemoteUDPPort( unsigned int pPort );
      unsigned int GetUDPPort()const;
      void Disconnect();
      bool IsConnected()const;

      SOCKET GetSocket()const;
      SOCKET GetUDPSocket()const;

      const MR_NetMessageBuffer* Poll( );
      void                       Send( const MR_NetMessageBuffer* pMessage, int pReqLevel );
      bool                       UDPSend( SOCKET pSocket, MR_NetMessageBuffer* pMessage, unsigned pQueueId, bool pResendLast );


      // Time related stuff
      bool  AddLagSample( int pLag );
      bool  LagDone()const;

      int           GetAvgLag()const;
      int           GetMinLag()const;
      void          SetLag( int pAvgLag, int pMinLag );

};



class MR_NetworkInterface
{
   public:
      enum
      { 
         eMaxClient = 8
      };


   private:


      std::string  mPlayer;
      int      mId;
      bool     mServerMode;
      SOCKET   mRegistrySocket;
      int      mServerPort;
      std::string  mServerAddr;
      std::string  mGameName;

      // UDP port
      SOCKET   mUDPOutShortPort;
      SOCKET   mUDPOutLongPort;

      // Data
      MR_NetworkPort mClient[ eMaxClient ];
      std::string    mClientName[ eMaxClient ];
      bool           mAllPreLoguedRecv; // Used by client to know if all prelogued have been received
      bool           mCanBePreLogued[ eMaxClient ];
      bool           mPreLoguedClient[ eMaxClient ];
      bool           mConnected[ eMaxClient ];       // Correctly connected state used by the server
      uint32_t       mClientAddr[ eMaxClient ];      // Used only in server mode
      int            mClientPort[ eMaxClient ];      // Used only in server mode

      int            mReturnMessage; // Message to return to the parent window in modeless mode

      // Dialog functions
      static MR_NetworkInterface* mActiveInterface;
      // static BOOL CALLBACK ServerPortCallBack(   HWND pWindow, UINT  pMsgId, WPARAM  pWParam, LPARAM  pLParam );
      // static BOOL CALLBACK ServerAddrCallBack(   HWND pWindow, UINT  pMsgId, WPARAM  pWParam, LPARAM  pLParam );
      // static BOOL CALLBACK WaitGameNameCallBack( HWND pWindow, UINT  pMsgId, WPARAM  pWParam, LPARAM  pLParam );
      // static BOOL CALLBACK ListCallBack(         HWND pWindow, UINT  pMsgId, WPARAM  pWParam, LPARAM  pLParam );

      // Helper func
      void SendConnectionDoneIfNeeded();

   public:
      // Creation and destruction
      MR_NetworkInterface(); 
      ~MR_NetworkInterface();

      void  SetPlayerName( const char* pPlayerName );
      const char* GetPlayerName()const;

      // bool MasterConnect( HWND pWindow, const char* pGameName, bool pPromptForPort = TRUE, unsigned pDefaultPort = MR_DEFAULT_NET_PORT, HWND* pModalessDlg = NULL, int pReturnMessage = 0);
      // bool SlavePreConnect( HWND pWindow, CString& pGameName );
      // bool SlaveConnect( HWND pWindow, const char* pServerIP=NULL, unsigned pPort = MR_DEFAULT_NET_PORT, const char* pGameName = NULL, HWND* pModalessDlg = NULL, int pReturnMessage = 0 );

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


      
#endif



