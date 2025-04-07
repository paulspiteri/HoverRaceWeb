#include "stdafx.h"
#include "MR_Types.h"
#include "DllObjectFactory.h"
#include "RecordFile.h"

class MR_ObjectFromStaticFactory
{   
   // Base class for object created with a Statically Linked Factory

   private:
      
   MR_ObjectFromFactoryId mId;

   public:

      // Construction and destruction
      MR_ObjectFromStaticFactory( const MR_ObjectFromFactoryId& pId );

      const MR_ObjectFromFactoryId& GetTypeId()const;

      static  void SerializePtr( CArchive& pArchive, MR_ObjectFromFactory*& pPtr );
      static  void SerializePtr( NoMFC::CArchive& pArchive, MR_ObjectFromFactory*& pPtr );
};