#pragma once

#include "../Util/nomfc_stdafx.h"
#include "../Util/MR_Types.h"

namespace NoMFC 
{
   typedef struct PALETTEENTRY {
      BYTE        peRed;
      BYTE        peGreen;
      BYTE        peBlue;
   } PALETTEENTRY;
}

NoMFC::PALETTEENTRY*       MR_GetColors( double pGamma, double pIntensity = 0.8, double pIntensityBase = 0.0 ); // return a vectors of MR_NB_COLORS-MR_RESERVED_COLORS
const NoMFC::PALETTEENTRY& MR_ConvertColor( MR_UInt8 pRed, MR_UInt8 pGreen, MR_UInt8 pBlue,
                                                   double pGamma, double pIntensity = 0.8, double pIntensityBase = 0.0 );
 