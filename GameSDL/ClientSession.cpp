// ClientSession.cpp
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



#include "ClientSession.h"
#include "TrackCommonStuff.h"
#include "EmscriptenInterop.h"
#include <cstring>
#include <iostream>

MR_ClientSession::MR_ClientSession(int playerId)
                 :mSession( TRUE )
{
   mMainCharacter1      = NULL;
   mMainCharacter2      = NULL;
   mGhostCharacter      = NULL;
   mNbLap               = 1;
   mAllowWeapons        = TRUE;
   mGhostRecorder       = new GhostRecorder(playerId);
   mGhostPlayer         = new GhostPlayer();
}

MR_ClientSession::~MR_ClientSession()
{
   delete mGhostRecorder;
   mGhostRecorder = nullptr;

   delete mGhostPlayer;
   mGhostPlayer = nullptr;
}

bool MR_ClientSession::Process( int pSpeedFactor )
{
   bool result = mSession.Simulate();
   if (result)
   {
      if (mGhostRecorder->IsRecording())
         {
         MR_ElementNetState netState = mMainCharacter1->GetNetState();
         const MR_MainCharacterState* charState = reinterpret_cast<const MR_MainCharacterState*>(netState.mData);
         mGhostRecorder->RecordFrame(*charState, GetSimulationTime());
      }

      if (mGhostCharacter != nullptr) {
         auto result = mGhostPlayer->GetNextFrame(GetSimulationTime());
         if (result.frame != nullptr) {
            int lOldRoom = mGhostCharacter->mRoom;
            mGhostCharacter->SetNetState(sizeof(MR_MainCharacterState), reinterpret_cast<const MR_UInt8*>(&result.frame->mState));
            // Move element if needed
            if(mGhostCharacter->mRoom != lOldRoom)
            {
               MR_Level* lCurrentLevel = mSession.GetCurrentLevel();
               lCurrentLevel->MoveElement(mGhostCharacterHandle, mGhostCharacter->mRoom);
            }
         }
         if (result.isCompleted) {
            DestroyGhostCharacter();
         }
      }
   }

   return result;
}

BOOL MR_ClientSession::LoadNew( const char* pTitle, MR_RecordFile* pMazeFile, int pNbLap, BOOL pAllowWeapons)
{
   BOOL lReturnValue;
   mNbLap        = pNbLap;
   mAllowWeapons = pAllowWeapons;
   lReturnValue  = mSession.LoadNew( pTitle, pMazeFile );

   return lReturnValue;
}

// Main character controll and interogation
BOOL MR_ClientSession::CreateMainCharacter()
{
   return CreateMainCharacterAtPosition(0);
}

BOOL MR_ClientSession::CreateMainCharacterAtPosition(int pPlayerIndex)
{

   // Add a main character in

   ASSERT( mMainCharacter1 == NULL ); // why creating it twice?
   ASSERT( mSession.GetCurrentLevel() != NULL );

   mMainCharacter1 = MR_MainCharacter::New( mNbLap, mAllowWeapons );

   // Insert the character in the current level
   MR_Level* lCurrentLevel = mSession.GetCurrentLevel();

   mMainCharacter1->mRoom        = lCurrentLevel->GetStartingRoom( pPlayerIndex );
   mMainCharacter1->mPosition    = lCurrentLevel->GetStartingPos( pPlayerIndex );
   mMainCharacter1->SetOrientation( lCurrentLevel->GetStartingOrientation( pPlayerIndex ));
   mMainCharacter1->SetHoverId( pPlayerIndex );

   lCurrentLevel->InsertElement( mMainCharacter1, lCurrentLevel->GetStartingRoom( pPlayerIndex ) );

   // Set up lap change callback for ghost recording
   mMainCharacter1->SetLapChangeCallback([this](int newLap, MR_SimulationTime lapDuration) {
      this->OnLapChange(newLap, lapDuration);
   });

   // Insert two dummy (TEST)
   /*
   MR_MainCharacter* lMainCharacter;

   lMainCharacter = MR_MainCharacter::New();
   lMainCharacter->SetAsSlave();      
 
   lMainCharacter->mPosition    = lCurrentLevel->GetStartingPos( 1 );
   lMainCharacter->mOrientation = lCurrentLevel->GetStartingOrientation( 1 );
   lCurrentLevel->InsertElement( lMainCharacter, lCurrentLevel->GetStartingRoom( 1 ) );


   lMainCharacter = MR_MainCharacter::New();
   lMainCharacter->SetAsSlave();      
 
   lMainCharacter->mPosition    = lCurrentLevel->GetStartingPos( 2 );
   lMainCharacter->mOrientation = lCurrentLevel->GetStartingOrientation( 2 );
   lCurrentLevel->InsertElement( lMainCharacter, lCurrentLevel->GetStartingRoom( 2 ) );
   */

   return TRUE;
}

BOOL MR_ClientSession::CreateMainCharacter2()
{

   // Add a main character in 
   
   ASSERT( mMainCharacter2 == NULL ); // why creating it twice?
   ASSERT( mSession.GetCurrentLevel() != NULL );

   mMainCharacter2 = MR_MainCharacter::New( mNbLap, mAllowWeapons );

   // Insert the character in the current level
   MR_Level* lCurrentLevel = mSession.GetCurrentLevel();

   mMainCharacter2->mRoom        = lCurrentLevel->GetStartingRoom( 1 );      
   mMainCharacter2->mPosition    = lCurrentLevel->GetStartingPos( 1 );
   mMainCharacter2->SetOrientation( lCurrentLevel->GetStartingOrientation( 1 ) );
   mMainCharacter2->SetHoverId( 1 );

   lCurrentLevel->InsertElement( mMainCharacter2, lCurrentLevel->GetStartingRoom( 1 ) );

   return TRUE;
}



MR_MainCharacter*  MR_ClientSession::GetMainCharacter()const      
{
   return mMainCharacter1; 
}

MR_MainCharacter*  MR_ClientSession::GetMainCharacter2()const      
{
   return mMainCharacter2; 
}

void MR_ClientSession::SetSimulationTime( MR_SimulationTime pTime )
{
   mSession.SetSimulationTime( pTime );
}

MR_SimulationTime MR_ClientSession::GetSimulationTime( )const
{
   return mSession.GetSimulationTime();
}

void MR_ClientSession::SetControlState( int pState1, int pState2 )
{
   if( mMainCharacter1 != NULL )
   {
      mMainCharacter1->SetControlState( pState1, mSession.GetSimulationTime() );
   }

   if( mMainCharacter2 != NULL )
   {
      mMainCharacter2->SetControlState( pState2, mSession.GetSimulationTime() );
   }
}

void MR_ClientSession::SetCurrentWeapon( MR_MainCharacter::eWeapon pWeapon )
{
   if( mMainCharacter1 != NULL )
   {
      mMainCharacter1->SetCurrentWeapon( pWeapon );
   }
}

const MR_Sprite* MR_ClientSession::GetMap() const
{
   return nullptr;
}

void MR_ClientSession::ConvertMapCoordinate(int& pX, int& pY, int pRatio) const
{
}

const MR_Level* MR_ClientSession::GetCurrentLevel()const
{
   MR_GameSession* lSession = (MR_GameSession*)&mSession;

   return lSession->GetCurrentLevel();
}

int MR_ClientSession::ResultAvaillable()const
{
   return 0;
}

void MR_ClientSession::GetResult( int, const char*& pPlayerName, int&, BOOL&, int&, MR_SimulationTime&, MR_SimulationTime& )const
{
   pPlayerName = "?";
   ASSERT( FALSE );
}

void MR_ClientSession::GetHitResult( int pPosition, const char*& pPlayerName, int& pId, BOOL& pConnected, int& pNbHitOther, int& pNbHitHimself )const
{
   pPlayerName = "?";
   ASSERT( FALSE );
}

int MR_ClientSession::GetNbPlayers()const
{
   BOOL lReturnValue = 0;

   if( mMainCharacter1 != NULL )
   {
      lReturnValue++;
   }
   if( mMainCharacter2 != NULL )
   {
      lReturnValue++;
   }
   return lReturnValue;
}

int MR_ClientSession::GetRank( const MR_MainCharacter* pPlayer )const
{
   int lReturnValue = 1;

   if( mMainCharacter1 != NULL )
   {
      if( pPlayer == mMainCharacter1 )
      {
         if( mMainCharacter2->HasFinish() )
         {
            if( mMainCharacter2->GetTotalTime() <  mMainCharacter1->GetTotalTime() )
            {
               lReturnValue = 2;
            }
         }
      }   
      else
      {
         lReturnValue = 2;

         if( mMainCharacter1->HasFinish() )
         {
            if( mMainCharacter1->GetTotalTime() <  mMainCharacter2->GetTotalTime() )
            {
               lReturnValue = 1;
            }
         }
      }
   }
   return lReturnValue;
}

const MR_MainCharacter* MR_ClientSession::GetPlayer( int pPlayerIndex )const
{
   const MR_MainCharacter* lReturnValue = NULL;

   switch( pPlayerIndex )
   {
      case 0:
         lReturnValue = mMainCharacter1;
         break;

      case 1:
         lReturnValue = mMainCharacter2;
         break;
   }
   ASSERT( lReturnValue != NULL );

   return lReturnValue;
}

void MR_ClientSession::AddMessageKey( char /*pKey*/ )
{

}

void MR_ClientSession::GetCurrentMessage( char* pDest )const
{
   pDest[0] = 0;
}

BOOL MR_ClientSession::GetMessageStack( int pLevel, char* pDest, int pExpiration )const
{
   BOOL lReturnValue = FALSE;


   if( pLevel < MR_CHAT_MESSAGE_STACK )
   {
      if( ((mMessageStack[ pLevel ].mCreationTime+pExpiration) > time( NULL ))&&(mMessageStack[ pLevel ].mBuffer.length() > 0) )
      {
         lReturnValue = TRUE;
         std::strcpy( pDest, mMessageStack[ pLevel ].mBuffer.c_str() );
      }
   }   

   return lReturnValue;
}

void MR_ClientSession::AddMessage( const char* pMessage )
{
   for( int lCounter = MR_CHAT_MESSAGE_STACK -1; lCounter > 0 ; lCounter-- )
   {
      mMessageStack[ lCounter ] =  mMessageStack[ lCounter-1 ];
   }

   mMessageStack[0].mCreationTime = time(NULL);

   mMessageStack[0].mBuffer  = Ascii2Simple( pMessage );
}

void MR_ClientSession::OnLapChange(int newLap, MR_SimulationTime lapDuration)
{
   mGhostRecorder->StopRecording(lapDuration);
   GhostFile ghostData = mGhostRecorder->GetGhostFile(mSession.GetTitle());
   int vehicleType = mMainCharacter1->GetHoverModel();
   EmscriptenInterop::OnLap(newLap, lapDuration, vehicleType, ghostData);

   if (lapDuration > 0 && (lapDuration < mGhostPlayer->GetLapDuration() || !mGhostPlayer->IsLoaded()))
   {
      std::cout << "New lap record!" << std::endl;
      mGhostRecorder->Save(mSession.GetTitle());
   }

   // Restart ghost playback at the start of every lap
   if (mGhostPlayer->IsLoaded())
   {
      mGhostPlayer->StartPlayback(GetSimulationTime());
      CreateGhostCharacter(mGhostPlayer->GetPlayerId());
   }

   // start new lap recording
   mGhostRecorder->StartRecording(GetSimulationTime());
}

void MR_ClientSession::LoadBestLapGhost(const unsigned char* ghostData, int dataSize)
{
   GhostFile ghostFile;

   if (GhostFile::FromBinaryData(ghostData, dataSize, ghostFile) &&
       mGhostPlayer->LoadFromData(ghostFile))
   {
      mGhostPlayer->StartPlayback(mMainCharacter1->GetLastLapCompletion());
      CreateGhostCharacter(mGhostPlayer->GetPlayerId());
      std::cout << "Best lap ghost loaded successfully!" << std::endl;
   }
   else
   {
      std::cout << "Failed to load best lap ghost" << std::endl;
   }
}

void MR_ClientSession::CreateGhostCharacter(int hoverId)
{
   // Only create if ghost is loaded and doesn't already exist
   if (!mGhostPlayer->IsLoaded() || mGhostCharacter != nullptr) {
      return;
   }

   mGhostCharacter = MR_MainCharacter::New(1, false);
   mGhostCharacter->SetAsSlave(true);
   mGhostCharacter->SetHoverId(hoverId);

   MR_Level* lCurrentLevel = mSession.GetCurrentLevel();
   if (lCurrentLevel != nullptr) {
      mGhostCharacterHandle = lCurrentLevel->InsertElement(mGhostCharacter, mGhostCharacter->mRoom);
   }
}

void MR_ClientSession::DestroyGhostCharacter()
{
   if (mGhostCharacter != nullptr) {
      MR_Level* lCurrentLevel = mSession.GetCurrentLevel();
      if (lCurrentLevel != NULL) {
         lCurrentLevel->DeleteElement(mGhostCharacterHandle);
      }
      mGhostCharacter = nullptr;
   }
}

