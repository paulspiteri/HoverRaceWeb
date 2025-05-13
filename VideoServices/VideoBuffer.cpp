// VideoBuffer.cpp
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

#include "VideoBuffer.h"
#include "ColorPalette.h"
#include "ColorPaletteEntry.h"

#include "../Util/Profiler.h"
#include <SDL3/SDL.h>


void PrintLog( const char* pFormat, ... );

#define OPEN_LOG()
#define CLOSE_LOG()

#ifdef _DEBUG
   #define PRINT_LOG          1?NULL:
#else
   #define PRINT_LOG          if( FALSE )PrintLog
#endif



MR_VideoBuffer::MR_VideoBuffer( double pGamma, double pContrast, double pBrightness )
{
   OPEN_LOG();
   PRINT_LOG( "VIDEO_BUFFER_CREATION" );

   mZBuffer     = NULL;
   mBuffer      = NULL;
   mBackPalette = NULL;

   mModeSettingInProgress = FALSE;

   mGamma      = pGamma;
   mContrast   = pContrast;
   mBrightness = pBrightness;
}


MR_VideoBuffer::~MR_VideoBuffer()
{
   delete []mBackPalette;

   PRINT_LOG( "VIDEO_BUFFER_DESTRUCTION\n\n" );
   CLOSE_LOG();

}

void MR_VideoBuffer::DeleteInternalSurfaces()
{
   PRINT_LOG( "DeleteInternalSurfaces" );

   delete []mZBuffer;
   mZBuffer = NULL;

   delete []mBuffer;
   mBuffer  = NULL;
}

void MR_VideoBuffer::CreatePalette( double pGamma, double pContrast, double pBrightness )
{
   PRINT_LOG( "CreatePalette" );

   NoMFC::PALETTEENTRY lPalette[256];

   int lCounter;

   mGamma      = pGamma;
   mContrast   = pContrast;
   mBrightness = pBrightness;

   if( mGamma < 0.2 )
   {
       mGamma = 0.2;
   }

   if( mGamma > 4.0 )
   {
       mGamma = 4.0;
   }

   if( mContrast > 1.0 )
   {
      mContrast = 1.0;
   }

   if( mContrast < 0.3 )
   {
      mContrast = 0.3;
   }

   if( mBrightness > 1 )
   {
      mBrightness = 1.0;
   }

   if( mBrightness < 0.3 )
   {
      mBrightness = 0.3;
   }

  // if( mVideoBufferDirectDraw->mDirectDraw != NULL )
   {
      // Add out own entries
      NoMFC::PALETTEENTRY* lOurEntries = MR_GetColors( 1.0/mGamma, mContrast*mBrightness, mBrightness-(mContrast*mBrightness) );

      for( lCounter = 0; lCounter<MR_BASIC_COLORS; lCounter++ )
      {
         lPalette[ MR_RESERVED_COLORS_BEGINNING+lCounter ] = lOurEntries[ lCounter ];
      }
      delete []lOurEntries;


      if( mBackPalette != NULL )
      {
         for( lCounter = 0; lCounter<MR_BACK_COLORS; lCounter++ )
         { 
            lPalette[ MR_RESERVED_COLORS_BEGINNING+MR_BASIC_COLORS+lCounter ] = 
               MR_ConvertColor( mBackPalette[ lCounter*3], mBackPalette[ lCounter*3+1], mBackPalette[ lCounter*3+2],
                                1.0/mGamma, mContrast*mBrightness, mBrightness-(mContrast*mBrightness) );
         }
      }
      
      memcpy(mPaletteEntries, lPalette, sizeof(mPaletteEntries));
   }
}

void MR_VideoBuffer::GetPaletteAttrib( double& pGamma, double& pContrast, double& pBrightness )
{
   pGamma      = mGamma;
   pContrast   = mContrast;
   pBrightness = mBrightness;
}

void MR_VideoBuffer::SetBackPalette( MR_UInt8* pPalette )
{
   delete []mBackPalette;
   mBackPalette = pPalette;

   CreatePalette( mGamma, mContrast, mBrightness );
}

BOOL MR_VideoBuffer::SetVideoMode(int width, int height)
{
   PRINT_LOG( "SetVideoMode(Window)" );
   BOOL            lReturnValue;

   ASSERT( !mModeSettingInProgress );

   mModeSettingInProgress = TRUE;

   CreatePalette(mGamma, mContrast, mBrightness);

   DeleteInternalSurfaces();

   mXRes = width;
   mYRes = height;
   mLineLen = width;

   // Create a local memory ZBuffer
   // We do not use DirectDrawZBuffer for now
   ASSERT( mZBuffer == NULL );
   mZBuffer = new MR_UInt16[ mXRes*mYRes ];

   ASSERT( mBuffer == NULL );
   mBuffer    = new MR_UInt8[ mXRes*mYRes ];
   mLineLen   = mXRes;

   mModeSettingInProgress = FALSE;

   return lReturnValue;
}

BOOL MR_VideoBuffer::IsWindowMode()const
{
   return TRUE;
}

BOOL MR_VideoBuffer::IsIconMode()const
{
   return FALSE;
}

BOOL MR_VideoBuffer::IsModeSettingInProgress()const
{
   return mModeSettingInProgress;
}

int MR_VideoBuffer::GetXRes()const
{
   return mXRes;
}

int MR_VideoBuffer::GetYRes()const
{
   return mYRes;
}

int MR_VideoBuffer::GetLineLen()const
{
   return mLineLen;
}

int MR_VideoBuffer::GetZLineLen()const
{
   return mXRes;
}


MR_UInt8* MR_VideoBuffer::GetBuffer()
{
   return mBuffer;
}

MR_UInt16* MR_VideoBuffer::GetZBuffer()
{
   return mZBuffer;
}

// THESE ARE NOT SUPPOSED TO BE CONSTANTS 
int MR_VideoBuffer::GetXPixelMeter()const
{
   {
      return 3 * 1024;
   }
}

int MR_VideoBuffer::GetYPixelMeter()const
{
   {
      return 4 * 768;
   }
}

BOOL MR_VideoBuffer::Lock()
{
   PRINT_LOG( "Lock" );

   MR_SAMPLE_CONTEXT( "LockVideoBuffer" );

   BOOL lReturnValue = TRUE;

   ASSERT( mBuffer != NULL );

   return lReturnValue;
}

void MR_VideoBuffer::Unlock(/* SDL_Texture* */ void* texture)
{
   SDL_Texture* sdlTexture = static_cast<SDL_Texture*>(texture);
   void* pixels = nullptr;
   int pitch = 0;
   if (SDL_LockTexture(sdlTexture, NULL, &pixels, &pitch)) {
      MR_UInt8* lSrc     = mBuffer;
      uint32_t* lDest = (uint32_t*)pixels;
      int lLineLen = pitch / sizeof(uint32_t);

      for( int lCounter = 0; lCounter < mYRes; lCounter++ )
      {
          for (int x = 0; x < mXRes; x++) {
              MR_UInt8 colorIndex = lSrc[x];
              NoMFC::PALETTEENTRY paletteEntry = mPaletteEntries[colorIndex];
              uint32_t color = 0xFF000000 | (paletteEntry.peRed << 16) | (paletteEntry.peGreen << 8) | paletteEntry.peBlue;
              lDest[x] = color;
          }
         lDest += lLineLen;
         lSrc  += mLineLen;
      }

      SDL_UnlockTexture(sdlTexture);
   } 
   else 
   {
      ASSERT(FALSE);
      TRACE("Failed to lock texture");
   }
}

void MR_VideoBuffer::Clear( MR_UInt8 pColor )
{
   ASSERT( mBuffer != NULL );

   memset( mBuffer, pColor, mLineLen*mYRes );
}

void MR_VideoBuffer::EnterIconMode()
{
   PRINT_LOG( "EnterIconMode" );
}

void MR_VideoBuffer::ExitIconMode()
{
   PRINT_LOG( "ExitIconMode" );


}



