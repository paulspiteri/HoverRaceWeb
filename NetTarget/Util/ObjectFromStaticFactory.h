#include "stdafx.h"
#include "MR_Types.h"
#include "DllObjectFactory.h"

class MR_ObjectFromStaticFactory: public CObject
{   
   // Base class for object created with a Statically Linked Factory

   private:
      
   MR_ObjectFromFactoryId mId;

   public:

      // Construction and destruction
      MR_ObjectFromStaticFactory( const MR_ObjectFromFactoryId& pId );

      const MR_ObjectFromFactoryId& GetTypeId()const;

      static  void SerializePtr( CArchive& pArchive, MR_ObjectFromFactory*& pPtr );
      virtual void Serialize( CArchive& pArchive );
};