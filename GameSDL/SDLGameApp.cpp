#include "SDLGameApp.h"
#include "../Util/DllObjectFactory.h"
#include "../MainCharacter/MainCharacter.h"
#include "../Util/FuzzyLogic.h"
#include "../Util/StrRes.h"
#include "../ObjFac1/ObjFac1.h"
#include <SDL3/SDL.h>
#include <filesystem>

#include "NetworkSession.h"

// global registration variables
static BOOL         gKeyFilled        = TRUE;   // disabled demo mode

MR_SDLGameApp* MR_SDLGameApp::This;


MR_SDLGameApp::MR_SDLGameApp(SDL_Window* glWindow, SDL_GLContext glContext)
{
   This             = this;
   mVideoBuffer     = NULL;
   mObserver1       = NULL;
   mCurrentSession  = NULL;
   mGLWindow = glWindow;
   mGLContext = glContext;
   mGLRenderer = nullptr;
   mGLLevelLoader = nullptr;
   mClrScrTodo = 2;

   // Screen
   mGamma       = 1.2;
   mContrast    = 0.95;
   mBrightness  = 0.95;

}

MR_SDLGameApp::~MR_SDLGameApp()
{
   Clean();
   MR_DllObjectFactory::Clean( FALSE );
   DeleteObjFac1();
   delete mVideoBuffer;
   mVideoBuffer = nullptr;

}

void MR_SDLGameApp::Clean()
{
   delete mCurrentSession;
   mCurrentSession = NULL;
   
   mObserver1->Delete();
   mObserver1 = NULL;

   delete mGLRenderer;
   mGLRenderer = nullptr;
   delete mGLLevelLoader;
   mGLLevelLoader = nullptr;

   MR_DllObjectFactory::Clean( TRUE );

   mClrScrTodo    = 2;
}

BOOL MR_SDLGameApp::InitGame()
{
   BOOL lReturnValue =TRUE;

   // Init needed modules
   MR_InitTrigoTables();
   MR_InitFuzzyModule();
   MR_DllObjectFactory::Init();
   
   ObjFac1RegisterFactory();
   MR_MainCharacter::RegisterFactory();

   if( lReturnValue )
   {
      mVideoBuffer = new MR_VideoBuffer( mGamma, mContrast, mBrightness );
   }

   if( lReturnValue )
   {
       mVideoBuffer->SetVideoMode(640, 400);
   }

   return lReturnValue;
}

bool MR_SDLGameApp::Simulate()
{
   if (mCurrentSession != nullptr) 
   {
      return mCurrentSession->Process();
   }
   throw std::runtime_error("Cannot simulate when no current session");
}

void MR_SDLGameApp::RefreshView(SDL_Texture* texture)
{
   // Game processing
   if( mVideoBuffer != NULL )
   {
      if( mVideoBuffer->Lock() )
      {
         if( mCurrentSession != NULL )
         {
            MR_SimulationTime lTime = mCurrentSession->GetSimulationTime();

            if( !gKeyFilled && (lTime<20000) )
            {
               if( lTime > 100 )
               {
                  MR_MainCharacter* lCharacter = mCurrentSession->GetMainCharacter();

                  if( lCharacter != NULL )
                  {
                     if( lCharacter->GetHoverModel() != 0 )
                     {
                        lCharacter->SetHoverModel( 0 );
                        mCurrentSession->AddMessage( MR_LoadString( IDS_CAR_FOR_REG ) );
                     }
                  }
               }
            }

            if( mClrScrTodo > 0 )
            {
               mClrScrTodo--;
               DrawBackground();
            }

            if( mObserver1 != NULL )
            {
               mObserver1->RenderNormalDisplay( mVideoBuffer, mGLRenderer, mCurrentSession, mCurrentSession->GetMainCharacter(), lTime, mCurrentSession->GetBackImage() );
            }
         }
         else
         {
            mVideoBuffer->Clear(0);
         }
         mVideoBuffer->Unlock(texture);
      }
   }

   if (mGLRenderer != nullptr)
   {
      mGLRenderer->MakeGLContextCurrent();

      auto freeElementInstances = mGLLevelLoader->GetFreeElementInstances(mCurrentSession->GetCurrentLevel());
      mGLRenderer->BindFreeElementInstances(freeElementInstances);
      mGLRenderer->BeginRender();

      mGLRenderer->BeginImguiFrame();
      mObserver1->RenderGLHUD(mGLRenderer, mCurrentSession);
      mGLRenderer->EndImguiFrame();

      mGLRenderer->EndRender();
      mGLRenderer->RenderMiniMap(mObserver1->GetMapSize());
      SDL_GL_SwapWindow(mGLWindow);
   }

   // Sound refresh
   if( mCurrentSession != NULL )
   {
       if( mObserver1 != NULL )
       {
          mObserver1->PlaySounds( mCurrentSession->GetCurrentLevel(), mCurrentSession->GetMainCharacter() );
       }

       MR_SoundServer::ApplyContinuousPlay();
   }
}

void MR_SDLGameApp::LoadSelectedTrack(const char* trackFile, int playerId, const std::array<PeerStatus, WebPeerInterface::eMaxClient>& peers, bool hasWeapons, int laps)
{
   BOOL lSuccess = TRUE;

   // Delete the current session
   Clean();

   // Use passed game settings
   int     lNbLap = laps;
   BOOL    lAllowWeapons = hasWeapons;

   if( lSuccess )
   {
      mGLRenderer = new GLRenderer(mGLWindow, mGLContext, mVideoBuffer);
      mGLLevelLoader = new GLLevelLoader(mGLRenderer);
      mObserver1 = MR_Observer::New();

      // Create the new session
      MR_NetworkSession* lCurrentSession = new MR_NetworkSession(playerId, peers);
      std::cout << "NetworkSession created " << std::endl;

      const char* trackTitle;
      // Load the selected maze
      if( lSuccess )
      {
         MR_RecordFile* lTrackFile = new MR_RecordFile();
         lTrackFile->OpenForRead(trackFile);
         auto trackFileName = std::filesystem::path(trackFile).stem().string();
         trackTitle = trackFileName.c_str();
         lSuccess = lCurrentSession->LoadNew(trackTitle, lTrackFile, lNbLap, lAllowWeapons, mVideoBuffer);
         if (lSuccess) 
         {
            std::cout << "Track file loaded" << std::endl;
         }
         lCurrentSession->SetPlayerName(("Player " + std::to_string(playerId)).c_str());
      }

      if( lSuccess )
      {
         auto level = lCurrentSession->GetCurrentLevel();
         mGLLevelLoader->LoadLevel(level, lCurrentSession->GetBackImage());
         auto levelSize = mGLLevelLoader->GetLevelSize(level);
         mObserver1->SetMapSize(levelSize);
         lCurrentSession->SetSimulationTime( -13000 );
      }

      // Create the main character
      if( lSuccess )
      {
         lSuccess = lCurrentSession->CreateMainCharacter();
         std::cout << "Main Character created" << std::endl;
      }

      if( !lSuccess )
      {
         std::cout << "Deleting session due to failure" << std::endl;
         // Clean everytings
         Clean();
         delete lCurrentSession;
      }
      else
      {
         mCurrentSession = lCurrentSession;
         std::cout << "New Local Session created" << std::endl;
      }
   }
}

void MR_SDLGameApp::SetVideoMode(int width, int height)
{
   mVideoBuffer->SetVideoMode(width, height);
}

void MR_SDLGameApp::DrawBackground()
{
   MR_UInt8* lDest         = mVideoBuffer->GetBuffer();
   int       lXRes         = mVideoBuffer->GetXRes();
   int       lYRes         = mVideoBuffer->GetYRes();
   int       lDestLineStep = mVideoBuffer->GetLineLen()-lXRes;

   int lColorIndex;


   for( int lY=0 ; lY<lYRes; lY++ )
   {
      lColorIndex = lY;
      for( int lX = 0; lX<lXRes; lX++ )
      {
         *lDest = (lColorIndex&16)?11:39;

         lColorIndex++;
         lDest++;
      }
      lDest+= lDestLineStep;
   }
}


void MR_SDLGameApp::SetControlState(int pState1)
{
   if (mCurrentSession != nullptr)
   {
      mCurrentSession->SetControlState( pState1, 0 );
   }
}

void MR_SDLGameApp::SetCurrentWeapon(MR_MainCharacter::eWeapon pWeapon)
{
   if (mCurrentSession != nullptr)
   {
      mCurrentSession->SetCurrentWeapon( pWeapon );
   }
}

void MR_SDLGameApp::SetResolution(int width, int height)
{
   mVideoBuffer->SetVideoMode(width, height);
}

void MR_SDLGameApp::SetOpenGLResolution(int width, int height)
{
   mGLRenderer->ChangeResolution(width, height);
}

void MR_SDLGameApp::DisconnectPlayer(int playerId)
{
   if (mCurrentSession != nullptr)
   {
      mCurrentSession->DisconnectPlayer(playerId);
   }
}
