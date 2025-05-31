#pragma once

#include "Observer.h"
#include "ClientSession.h"
#include <SDL3/SDL.h>

#include "GLLevelLoader.h"
#include "../VideoServices/GL/GLRenderer.h"

class MR_SDLGameApp
{   
   private:
      static MR_SDLGameApp* This; // unique instance pointer

      SDL_Window*              mGLWindow;
      SDL_GLContext            mGLContext;
      MR_VideoBuffer*          mVideoBuffer;
      GLRenderer*              mGLRenderer;
      GLLevelLoader*           mGLLevelLoader;
      MR_Observer*             mObserver1;
      MR_ClientSession*        mCurrentSession;

      int                      mClrScrTodo;

      double mGamma;
      double mContrast;
      double mBrightness;

      void DrawBackground();

   public:

      MR_SDLGameApp(SDL_Window* glWindow, SDL_GLContext glContext);
      ~MR_SDLGameApp();

      void Clean();
      BOOL InitGame();
      void LoadSelectedTrack(const char* trackFile);
      void SetVideoMode(int width, int height);
      void RefreshView(SDL_Texture* texture);
      void Simulate();
      void SetControlState(int pState1);
      void SetResolution(int width, int height);
      void SetOpenGLResolution(int width, int height);
};
