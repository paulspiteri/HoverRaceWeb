#include "SoundServer.h"

// Forward declarations of classes
class MR_ShortSound {};
class MR_ContinuousSound {};

namespace MR_SoundServer
{
   BOOL Init(HWND pWindow)
   {
      return TRUE; // Return success but do nothing
   }

   void Close()
   {
      // Do nothing
   }

   MR_ShortSound* CreateShortSound(const char* pData, int pNbCopy)
   {
      return nullptr; // Return nullptr as we're not creating anything
   }

   void DeleteShortSound(MR_ShortSound* pSound)
   {
      // Do nothing
   }

   void Play(MR_ShortSound* pSound, int pDB, double pSpeed, int pPan)
   {
      // Do nothing
   }

   int GetNbCopy(MR_ShortSound* pSound)
   {
      return 0; // Return 0 as we're not tracking copies
   }

   MR_ContinuousSound* CreateContinuousSound(const char* pData, int pNbCopy)
   {
      return nullptr; // Return nullptr as we're not creating anything
   }

   void DeleteContinuousSound(MR_ContinuousSound* pSound)
   {
      // Do nothing
   }

   void Play(MR_ContinuousSound* pSound, int pCopy, int pDB, double pSpeed, int pPan)
   {
      // Do nothing
   }

   int GetNbCopy(MR_ContinuousSound* pSound)
   {
      return 0; // Return 0 as we're not tracking copies
   }

   void ApplyContinuousPlay()
   {
      // Do nothing
   }
}

