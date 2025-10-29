#include "SDLGameApp.h"
#include "../Util/DllObjectFactory.h"
#include "../MainCharacter/MainCharacter.h"
#include "../Util/FuzzyLogic.h"
#include "../Util/StrRes.h"
#include "../ObjFac1/ObjFac1.h"
#include <SDL3/SDL.h>
#include <filesystem>

#include "GLObserver.h"
#include "NetworkSession.h"

// global registration variables
static BOOL         gKeyFilled        = TRUE;   // disabled demo mode

MR_SDLGameApp* MR_SDLGameApp::This;


MR_SDLGameApp::MR_SDLGameApp(SDL_Window* glWindow, SDL_GLContext glContext)
{
   This             = this;
   mObserver1       = NULL;
   mCurrentSession  = NULL;
   mGLWindow = glWindow;
   mGLContext = glContext;
   mGLRenderer = nullptr;
   mGLLevelLoader = nullptr;
   mClrScrTodo = 2;
}

MR_SDLGameApp::~MR_SDLGameApp()
{
   Clean();
   MR_DllObjectFactory::Clean( FALSE );
   DeleteObjFac1();

}

void MR_SDLGameApp::Clean()
{
   delete mCurrentSession;
   mCurrentSession = nullptr;
   
   delete mObserver1;
   mObserver1 = nullptr;

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

void MR_SDLGameApp::RefreshView()
{
   if( mCurrentSession != NULL )
   {
      if( mObserver1 != NULL )
      {
         mObserver1->RenderGLDisplay( mGLRenderer, mCurrentSession->GetMainCharacter(), mCurrentSession->GetSimulationTime() );
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
      // Create the new session
      MR_NetworkSession* lCurrentSession = new MR_NetworkSession(playerId, peers);
      std::cout << "NetworkSession created " << std::endl;

      VideoPalette* palette;
      const char* trackTitle;
      // Load the selected maze
      if( lSuccess )
      {
         MR_RecordFile* lTrackFile = new MR_RecordFile();
         lTrackFile->OpenForRead(trackFile);
         auto trackFileName = std::filesystem::path(trackFile).stem().string();
         trackTitle = trackFileName.c_str();
         lSuccess = lCurrentSession->LoadNew(trackTitle, lTrackFile, lNbLap, lAllowWeapons);
         if (lSuccess)
         {
            palette = new VideoPalette(lTrackFile, 1.0, 1.0, 1.0);
            std::cout << "Track file loaded" << std::endl;
         }
         lCurrentSession->SetPlayerName(("Player " + std::to_string(playerId)).c_str());
      }

      if( lSuccess )
      {
         mGLRenderer = new GLRenderer(mGLWindow, mGLContext, palette);
         mGLLevelLoader = new GLLevelLoader(mGLRenderer);
         mObserver1 = new GLObserver();

         auto level = lCurrentSession->GetCurrentLevel();
         mGLLevelLoader->LoadLevel(level, palette->GetBackImage());
         auto levelSize = mGLLevelLoader->GetLevelSize(level);
         mObserver1->SetMapSize(levelSize);
         bool hasConnectedPeers = std::any_of(peers.begin(), peers.end(), [](const auto& peer) { return peer.isConnected; });
         lCurrentSession->SetSimulationTime(hasConnectedPeers ? -13000 : -4000);
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
