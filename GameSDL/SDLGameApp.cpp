#include "SDLGameApp.h"
#include "../Util/DllObjectFactory.h"
#include "../VideoServices/ColorPalette.h"
#include "../MainCharacter/MainCharacter.h"
#include "../Util/FuzzyLogic.h"
#include "../Util/Profiler.h"
#include "../Util/StrRes.h"
#include "../ObjFac1/ObjFac1.h"
#include <SDL3/SDL.h>
#include <filesystem>

// global registration variables
static BOOL         gKeyFilled        = TRUE;   // disabled demo mode

MR_SDLGameApp* MR_SDLGameApp::This;


MR_SDLGameApp::MR_SDLGameApp(SDL_Texture* texture, SDL_Window* glWindow, SDL_GLContext glContext)
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

   // Built-in defaults
   // Controls
   mMotorOn1   = 1;
   mRight1     = 5;
   mLeft1      = 6;
   mJump1      = 3;
   mFire1      = 2;
   mBreak1     = 4;
   mWeapon1    = 11;

   mMotorOn2   = 38;
   mRight2     = 18;
   mLeft2      = 31;
   mJump2      = 17;
   mFire2      = 13;
   mBreak2     = 16;
   mWeapon2    = 29;

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

void MR_SDLGameApp::Simulate()
{
   if (mCurrentSession != nullptr) 
   {
      mCurrentSession->Process();
   }
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

   if (mGLRenderer)
   {
      mGLRenderer->Render();
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

void MR_SDLGameApp::LoadSelectedTrack(const char* trackFile)
{
   BOOL lSuccess = TRUE;

   // Delete the current session
   Clean();

   // Prompt the user for a maze name
   int     lNbLap = 5;
   BOOL    lAllowWeapons = TRUE;

   if( lSuccess )
   {
      mGLRenderer = new GLRenderer(mGLWindow, mGLContext, mVideoBuffer);
      mGLLevelLoader = new GLLevelLoader(mGLRenderer);
      mObserver1 = MR_Observer::New();

      // Create the new session
      MR_ClientSession* lCurrentSession = new MR_ClientSession;
      std::cout << "ClientSession created " << std::endl;

      // Load the selected maze
      if( lSuccess )
      {
         MR_RecordFile* lTrackFile = new MR_RecordFile();
         lTrackFile->OpenForRead(trackFile);
         auto trackFileName = std::filesystem::path(trackFile).stem().string();
         const char* trackTitle = trackFileName.c_str();
         lSuccess = lCurrentSession->LoadNew(trackTitle, lTrackFile, lNbLap, lAllowWeapons, mVideoBuffer);
         if (lSuccess) 
         {
            std::cout << "Track file loaded" << std::endl;
         }
        }

      // Create the main character
      if( lSuccess )
      {
         auto level = lCurrentSession->GetCurrentLevel();
         mGLLevelLoader->LoadLevel(level, lCurrentSession->GetBackImage());

         lCurrentSession->SetSimulationTime( -6000 );
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

void MR_SDLGameApp::SetResolution(int width, int height)
{
   mVideoBuffer->SetVideoMode(width, height);
}

void MR_SDLGameApp::SetOpenGLResolution(int width, int height)
{
   mGLRenderer->state.swapchain.width = width;
   mGLRenderer->state.swapchain.height = height;
}
