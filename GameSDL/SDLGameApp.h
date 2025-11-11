#pragma once

#include <SDL3/SDL.h>

#include "GLLevelLoader.h"
#include "GLObserver.h"
#include "NetworkSession.h"
#include "WebPeerInterface.h"
#include "../VideoServices/GL/GLRenderer.h"

class MR_SDLGameApp
{   
   private:
      static MR_SDLGameApp* This; // unique instance pointer

      SDL_Window*              mGLWindow;
      SDL_GLContext            mGLContext;
      GLRenderer*              mGLRenderer;
      GLLevelLoader*           mGLLevelLoader;
      GLObserver*              mObserver1;
      MR_NetworkSession*       mCurrentSession;

      int                      mClrScrTodo;

      bool mIsHost;

   public:

      MR_SDLGameApp(SDL_Window* glWindow, SDL_GLContext glContext);
      ~MR_SDLGameApp();

      void Clean();
      BOOL InitGame();
      void LoadSelectedTrack(const char* trackFile, int playerId, const std::array<PeerStatus, WebPeerInterface::eMaxClient>& peers, bool hasWeapons, int laps);
      void RefreshView();
      bool Simulate();
      void SetControlState(int pState1);
      void SetCurrentWeapon(MR_MainCharacter::eWeapon pWeapon);
      void SetOpenGLResolution(int width, int height);
      void DisconnectPlayer(int playerId);
      void LoadBestLapGhost(const unsigned char* ghostData, int dataSize);
};
