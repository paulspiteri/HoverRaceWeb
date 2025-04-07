#include "../Util/ObjectFromStaticFactory.h"
#include "ObjFac1.h"

MR_ObjectFromStaticFactory::MR_ObjectFromStaticFactory( const MR_ObjectFromFactoryId& pId )
{
    mId   = pId;
}

const MR_ObjectFromFactoryId& MR_ObjectFromStaticFactory::GetTypeId()const
{
    return mId;
}

void MR_ObjectFromStaticFactory::SerializePtr( NoMFC::CArchive& pArchive, MR_ObjectFromFactory*& pPtr )
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
        if (lId.mDllId == 1) 
        {
            pPtr = MR_GetObject( lId.mClassId );
            pPtr->Serialize( pArchive );
        }
        else 
        {
            // unknown resource - not statically linked
            ASSERT( FALSE );
            AfxThrowNotSupportedException();
        }
      }
   }
}
