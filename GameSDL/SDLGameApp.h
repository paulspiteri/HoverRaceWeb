#pragma once

#include "Observer.h"
#include "ClientSession.h"
#include <SDL3/SDL.h>

class MR_SDLGameApp
{   
   private:
      static MR_SDLGameApp* This; // unique instance pointer
    
      SDL_Texture*             mTexture;
      MR_VideoBuffer*          mVideoBuffer;
      MR_Observer*             mObserver1;
      MR_ClientSession*        mCurrentSession;

      int                      mClrScrTodo;

      // Keyboard configuration
      int mMotorOn1;
      int mRight1;
      int mLeft1;
      int mJump1;
      int mFire1;
      int mBreak1;
      int mWeapon1;

      int mMotorOn2;
      int mRight2;
      int mLeft2;
      int mJump2;
      int mFire2;
      int mBreak2;
      int mWeapon2;

      double mGamma;
      double mContrast;
      double mBrightness;

      void Clean();
      void DrawBackground( );

   public:

      MR_SDLGameApp(SDL_Texture* texture);
      ~MR_SDLGameApp();

      BOOL InitApplication();
      BOOL InitGame();
      void LoadSelectedTrack(const char* trackFile);
      void RefreshView();
      void Simulate();
      void SetControlState(int pState1);
};
