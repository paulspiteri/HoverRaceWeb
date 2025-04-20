// SoundServer.h // DirectSound encapsulation module
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


#ifndef SOUND_SERVER_H
#define SOUND_SERVER_H

#include "../Util/nomfc_stdafx.h"

#ifdef MR_VIDEO_SERVICES
   #define MR_DllDeclare //   __declspec( dllexport )
#else
   #define MR_DllDeclare //   __declspec( dllimport )
#endif

class MR_ShortSound;
class MR_ContinuousSound;


namespace MR_SoundServer
{   

   MR_DllDeclare BOOL Init();

   MR_DllDeclare MR_ShortSound*      CreateShortSound( const char* pData, int pNbCopy );
   MR_DllDeclare void                DeleteShortSound( MR_ShortSound* pSound );
   MR_DllDeclare void                Play( MR_ShortSound* pSound, int pDB = 0, double pSpeed = 1.0, int pPan = 0 );

   MR_DllDeclare MR_ContinuousSound* CreateContinuousSound( const char* pData, int pNbCopy );
   MR_DllDeclare void                DeleteContinuousSound( MR_ContinuousSound* pSound );
   MR_DllDeclare void                Play( MR_ContinuousSound* pSound, int pCopy, int pDB = 0, double pSpeed = 1.0, int pPan = 0 );
   
   MR_DllDeclare void                ApplyContinuousPlay();
};



#undef MR_DllDeclare

#endif



