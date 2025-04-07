// DllObjectFactory.cpp
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

#include "DllObjectFactory.h"
#include <unordered_map>

// DllObjectFactory file name prefix
static const char gsFilePrefix[] = ".\\ObjFac";

// Local class declaration
class MR_FactoryDll
{
   // This class is used to kepp information about a loaded dll
   public:
      BOOL       mDynamic;
      int        mRefCount;

      MR_ObjectFromFactory* (*mGetObject)( MR_UInt16 pClassId );

      // Initialisation
      MR_FactoryDll();
      ~MR_FactoryDll();

      BOOL Open( MR_UInt16 pDllId ); // Must be called only once
};

// Local functions declarations
static MR_FactoryDll* GetDll( MR_UInt16 pDllId, BOOL pTrowOnError );


// Module variables
static std::unordered_map<MR_UInt16, MR_FactoryDll*> gsDllList;
      
// Module functions
void MR_DllObjectFactory::Init()
{
   // Notting to do
}

void MR_DllObjectFactory::Clean( BOOL pOnlyDynamic )
{
    for (auto it = gsDllList.begin(); it != gsDllList.end(); )
    {
        MR_FactoryDll *lDllPtr = it->second;
        if ((lDllPtr->mRefCount <= 0) && (lDllPtr->mDynamic || !pOnlyDynamic))
        {
            // Erase returns the iterator pointing to the next element
            it = gsDllList.erase(it);
            // Free the dll memory
            delete lDllPtr;
        }
        else
        {
            ++it;
        }
    }
}

BOOL MR_DllObjectFactory::OpenDll( MR_UInt16 pDllId )
{
   return( GetDll( pDllId, FALSE ) != NULL );
}

void MR_DllObjectFactory::RegisterLocalDll( MR_UInt16 pDllId, MR_ObjectFromFactory* (*pFunc)(MR_UInt16) )
{

   MR_FactoryDll* lDllPtr;

   ASSERT( pDllId != 0 ); // Number 0 is reserved for NULL entry
   ASSERT( pFunc != NULL );

   // Verify if the entry do not already exist
   ASSERT( gsDllList.find( pDllId ) == gsDllList.end() ); // Dll already registered

   lDllPtr = new MR_FactoryDll;

   lDllPtr->mGetObject = pFunc;

   gsDllList.insert(std::make_pair(pDllId, lDllPtr ));

      
}


void MR_DllObjectFactory::IncrementReferenceCount( MR_UInt16 pDllId )
{
   auto it = gsDllList.find(pDllId);
   if(it != gsDllList.end())
   {
       it->second->mRefCount++;
   }
   else
   {
       ASSERT(FALSE); // Dll not loaded
   }
}

void MR_DllObjectFactory::DecrementReferenceCount( MR_UInt16 pDllId )
{
   auto it = gsDllList.find(pDllId);
   if(it != gsDllList.end())
   {
       it->second->mRefCount--;
   }
   else
   {
       ASSERT(FALSE); // Dll not loaded
   }
}

MR_ObjectFromFactory* MR_DllObjectFactory::CreateObject( const MR_ObjectFromFactoryId& pId )
{
   MR_ObjectFromFactory* lReturnValue;


   MR_FactoryDll* lDllPtr = GetDll( pId.mDllId, TRUE );

   ASSERT( lDllPtr->mGetObject != NULL );
   
   lReturnValue = lDllPtr->mGetObject( pId.mClassId ); 
     

   return lReturnValue;
}

MR_FactoryDll* GetDll( MR_UInt16 pDllId, BOOL pTrowOnError )
{

   ASSERT( pDllId != 0 ); // Number 0 is reserved for NULL entry

   auto it = gsDllList.find( pDllId );
   if (it == gsDllList.end())
   // Verify if the entry do not already exist
   if( it == gsDllList.end() )
   {
         ASSERT( FALSE ); // Unable to open the DLL
         throw std::runtime_error("Not supported");
   }

   return it->second;
}



// MR_ObjectFromFactory methods
MR_ObjectFromFactory::MR_ObjectFromFactory( const MR_ObjectFromFactoryId& pId )
{
   mId   = pId;

   // Increment this object dll reference count
   // This will prevent the Dll from being discarted
   MR_DllObjectFactory::IncrementReferenceCount( mId.mDllId );

}

MR_ObjectFromFactory::~MR_ObjectFromFactory()
{
   
   // Decrement this object dll reference count
   // This will allow the dll to be freed if not needed anymore
   MR_DllObjectFactory::DecrementReferenceCount( mId.mDllId );

}


const MR_ObjectFromFactoryId& MR_ObjectFromFactory::GetTypeId()const
{
   return mId;
}

void MR_ObjectFromFactory::SerializePtr( NoMFC::CArchive& pArchive, MR_ObjectFromFactory*& pPtr )
{
   MR_ObjectFromFactoryId lId = {0,0};

   if( pArchive.IsStoring() )
   {
      if( pPtr != NULL )
      {
         lId   = pPtr->mId;
      }
      
      lId.Serialize( pArchive );               

      if( pPtr != NULL )
      {
         pPtr->Serialize( pArchive );
      }
   }
   else
   {
      lId.Serialize( pArchive );

      if( lId.mDllId == 0 )
      {
         pPtr = NULL;
      }
      else
      {
         pPtr = MR_DllObjectFactory::CreateObject( lId );
         pPtr->Serialize( pArchive );
      }
   }
}

void MR_ObjectFromFactory::Serialize( NoMFC::CArchive& pArchive )
{
   // Notting to serialize at that point
   // Object type should be already initialize if Loading
}

// MR_ObjectFromFactoryId
void MR_ObjectFromFactoryId::Serialize( NoMFC::CArchive& pArchive )
{
   if( pArchive.IsStoring() )
   {
      pArchive << mDllId
               << mClassId;

   }
   else
   {
      pArchive >> mDllId
               >> mClassId;
   }
}

int MR_ObjectFromFactoryId::operator ==( const MR_ObjectFromFactoryId& pId )const
{
   return( (mDllId == pId.mDllId)&&(mClassId==pId.mClassId));
}


// class MR_FactoryDll methods
      
MR_FactoryDll::MR_FactoryDll()
{
   mDynamic  = FALSE;
   mRefCount = 0;

   mGetObject            = NULL;


}

MR_FactoryDll::~MR_FactoryDll()
{
   ASSERT( mRefCount == 0 );
}

