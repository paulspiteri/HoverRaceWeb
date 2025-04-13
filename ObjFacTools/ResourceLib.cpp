// ResourceLib.cpp
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


#include "ResourceLib.h"
#include <exception>

MR_ResourceLib::MR_ResourceLib( const char* pResFile )
{
   if( mRecordFile.OpenForRead( pResFile ) )
   {

      mRecordFile.SelectRecord( 0 );

      {
         int lMagicNumber;

         NoMFC::CArchive lArchive( mRecordFile.File(), NoMFC::CArchive::load );

         lArchive >> lMagicNumber;

         if( lMagicNumber == MR_RESOURCE_FILE_MAGIC )
         {
            // Load the Bitmaps
            LoadBitmaps( lArchive );

            // Load the Actors
            LoadActors( lArchive );

            // Load the sprites
            LoadSprites( lArchive );

            // Load the sprites
            LoadSounds( lArchive );

         }
         else
         {
            ASSERT( FALSE );
            throw std::runtime_error("Invalid resource file format");
         }
      }
   }
   else
   {
      ASSERT( FALSE );
      throw std::runtime_error("Failed to open resource file");
   }
}

MR_ResourceLib::MR_ResourceLib()
{
}

   
MR_ResourceLib::~MR_ResourceLib()
{
   for(auto it = mActorList.begin(); it != mActorList.end(); ++it)
   {
      delete it->second;
   }

   for(auto it = mBitmapList.begin(); it != mBitmapList.end(); ++it)
   {
      delete it->second;
   }

   for(auto it = mSpriteList.begin(); it != mSpriteList.end(); ++it)
   {
      delete it->second;
   }

   for(auto it = mShortSoundList.begin(); it != mShortSoundList.end(); ++it)
   {
      delete it->second;
   }

   for(auto it = mContinuousSoundList.begin(); it != mContinuousSoundList.end(); ++it)
   {
      delete it->second;
   }
}

/*const*/ MR_ResBitmap* MR_ResourceLib::GetBitmap( int pBitmapId )
{
   auto it = mBitmapList.find(pBitmapId);
   if (it != mBitmapList.end())
       return it->second;
   return nullptr;
}

const MR_ResActor* MR_ResourceLib::GetActor(  int pActorId  )
{
   auto it = mActorList.find(pActorId);
   if (it != mActorList.end())
       return it->second;
   return nullptr;
}

const MR_ResSprite* MR_ResourceLib::GetSprite(  int pSpriteId  )
{
   auto it = mSpriteList.find(pSpriteId);
   if (it != mSpriteList.end())
       return it->second;
   return nullptr;
}

const MR_ResShortSound* MR_ResourceLib::GetShortSound( int pSoundId  )
{
   auto it = mShortSoundList.find(pSoundId);
   if (it != mShortSoundList.end())
       return it->second;
   return nullptr;
}

const MR_ResContinuousSound* MR_ResourceLib::GetContinuousSound( int pSoundId  )
{
   auto it = mContinuousSoundList.find(pSoundId);
   if (it != mContinuousSoundList.end())
       return it->second;
   return nullptr;
}


void MR_ResourceLib::LoadBitmaps( NoMFC::CArchive& pArchive )
{
   int lNbBitmap;

   pArchive >> lNbBitmap;

   for( int lCounter = 0; lCounter < lNbBitmap; lCounter++ )
   {
      int           lKey;
      MR_ResBitmap* lValue;

      pArchive >> lKey;

      lValue = new MR_ResBitmap( lKey );
      
      lValue->Serialize( pArchive );
      
      mBitmapList[lKey] = lValue;
   }  
}


void MR_ResourceLib::LoadActors( NoMFC::CArchive& pArchive )
{
   int lNbActor;

   pArchive >> lNbActor;

   for( int lCounter = 0; lCounter < lNbActor; lCounter++ )
   {
      int lKey;
      MR_ResActor* lValue;     

      pArchive >> lKey;

      lValue = new MR_ResActor( lKey );
      lValue->Serialize( pArchive, this );

      mActorList[lKey] = lValue;
   }  
}


void MR_ResourceLib::LoadSprites( NoMFC::CArchive& pArchive )
{
   int lNbSprite;

   pArchive >> lNbSprite;

   for( int lCounter = 0; lCounter < lNbSprite; lCounter++ )
   {
      int lKey;
      MR_ResSprite* lValue;     

      pArchive >> lKey;

      lValue = new MR_ResSprite( lKey );
      lValue->Serialize( pArchive );
      
      mSpriteList[lKey] = lValue;
   }  
}


void MR_ResourceLib::LoadSounds( NoMFC::CArchive& pArchive )
{
   int lNbSound;
   int lCounter;

   pArchive >> lNbSound;

   for( lCounter = 0; lCounter < lNbSound; lCounter++ )
   {
      int lKey;
      MR_ResShortSound* lValue;     

      pArchive >> lKey;

      lValue = new MR_ResShortSound( lKey );
      lValue->Serialize( pArchive );

      mShortSoundList[lKey] = lValue;
   }  

   pArchive >> lNbSound;

   for( lCounter = 0; lCounter < lNbSound; lCounter++ )
   {
      int lKey;
      MR_ResContinuousSound* lValue;     

      pArchive >> lKey;

      lValue = new MR_ResContinuousSound( lKey );
      lValue->Serialize( pArchive );

      mContinuousSoundList[lKey] = lValue;
   }  

}


