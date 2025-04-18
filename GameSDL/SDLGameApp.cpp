#include "SDLGameApp.h"
#include "../Util/DllObjectFactory.h"
#include "../VideoServices/ColorPalette.h"
#include "../MainCharacter/MainCharacter.h"
#include "../Util/FuzzyLogic.h"
#include "../Util/Profiler.h"
#include "../Util/StrRes.h"
#include "../ObjFac1/ObjFac1.h"
#include <SDL3/SDL.h>
#include <future>
#include <filesystem>

// global registration variables
static BOOL         gKeyFilled        = TRUE;   // disabled demo mode

MR_SDLGameApp* MR_SDLGameApp::This;


MR_SDLGameApp::MR_SDLGameApp(SDL_Texture* texture)
{
   This             = this;
   mVideoBuffer     = NULL;
   mObserver1       = NULL;
   mCurrentSession  = NULL;
   mTexture         = texture;
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
   MR_SoundServer::Close();
   delete mVideoBuffer;
}

void MR_SDLGameApp::Clean()
{
   delete mCurrentSession;
   mCurrentSession = NULL;
   
   mObserver1->Delete();
   mObserver1 = NULL;

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
       mVideoBuffer->SetVideoMode();
   }

   return lReturnValue;
}

void MR_SDLGameApp::Simulate()
{
   ASSERT( mCurrentSession != NULL );
   mCurrentSession->Process();
}

void MR_SDLGameApp::RefreshView()
{
   static int lColor = 0;

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
               mObserver1->RenderNormalDisplay( mVideoBuffer, mCurrentSession, mCurrentSession->GetMainCharacter(), lTime, mCurrentSession->GetBackImage() );
            }
         }
         else
         {
            mVideoBuffer->Clear( lColor++ );
         }
         mVideoBuffer->Unlock(mTexture);
      }
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

struct DialogCallbackContext {
   MR_SDLGameApp* app;
   std::promise<void>* promise;
};

void MR_SDLGameApp::NewLocalSession()
{   
   std::string defaultTrackFile = ".\\Tracks\\ClassicH.trk";
   if (std::filesystem::exists(defaultTrackFile)) {
      LoadSelectedTrack(defaultTrackFile.c_str());
   } 
   else 
   {
      std::promise<void> dialogPromise;
      std::future<void> dialogFuture = dialogPromise.get_future();
      DialogCallbackContext context { this, &dialogPromise };
      SDL_ShowOpenFileDialog([](void* userdata, const char* const* filelist, int filter) 
         {
            DialogCallbackContext* context = static_cast<DialogCallbackContext*>(userdata);
            if (filelist && *filelist) 
            {
            context->app->LoadSelectedTrack(filelist[0]);
            }
            context->promise->set_value();
         }, &context, NULL, NULL, 0, NULL, FALSE);

         dialogFuture.get();
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
      MR_SoundServer::Init( NULL );
      mObserver1 = MR_Observer::New();

      // Create the new session
      MR_ClientSession* lCurrentSession = new MR_ClientSession;


      // Load the selected maze
      if( lSuccess )
      {
         MR_RecordFile* lTrackFile = new MR_RecordFile();
         lTrackFile->OpenForRead(trackFile);
         auto trackFileName = std::filesystem::path(trackFile).stem().string();
         const char* trackTitle = trackFileName.c_str();
         lSuccess = lCurrentSession->LoadNew(trackTitle, lTrackFile, lNbLap, lAllowWeapons, mVideoBuffer);
        }

      // Create the main character
      if( lSuccess )
      {
         lCurrentSession->SetSimulationTime( -6000 );
      }

      // Create the main character
      if( lSuccess )
      {
         lSuccess = lCurrentSession->CreateMainCharacter();
      }

      if( !lSuccess )
      {
         // Clean everytings
         Clean();
         delete lCurrentSession;
      }
      else
      {
         mCurrentSession = lCurrentSession;
      }
   }
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
   ASSERT( mCurrentSession != NULL );
   mCurrentSession->SetControlState( pState1, NULL );
}
