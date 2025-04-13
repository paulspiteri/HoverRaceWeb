// Patch.cpp

#include "Patch.h"

const MR_3DCoordinate& MR_Patch::GetNodePos( int pU, int pV )const
{
   return GetNodePos( pU+pV*GetURes() );
}

const MR_3DCoordinate& MR_Patch::GetNodePos( int pIndex )const
{
   return GetNodeList()[ pIndex ];
}


