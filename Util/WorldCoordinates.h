// WorldCoordinates.h
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

#include "MR_Types.h"
#include "RecordFile.h"
#include <cmath>
#include <numbers>

#ifndef WORLD_COORDINATES_H
#define WORLD_COORDINATES_H


#ifdef MR_UTIL
   #define MR_DllDeclare //   __declspec( dllexport )
#else
   #define MR_DllDeclare //   __declspec( dllimport )
#endif



// All geometrical coordinates are in milimeters,

class MR_DllDeclare MR_2DCoordinate
{
   public:
      MR_Int32  mX;
      MR_Int32  mY;

      void Serialize( NoMFC::CArchive& pArchive );

      // Comparaison operator
      BOOL operator ==( const MR_2DCoordinate& pCoordinate )const;
      BOOL operator !=( const MR_2DCoordinate& pCoordinate )const;

      // Inline constructors
      MR_2DCoordinate(){};
      MR_2DCoordinate( MR_Int32 pX, MR_Int32 pY){ mX = pX; mY=pY; };
};

class MR_DllDeclare MR_3DCoordinate : public MR_2DCoordinate
{
   public:
      MR_Int32  mZ;
   
      void Serialize( NoMFC::CArchive& pArchive );

      // Comparaison operator


      // Inline constructors
      MR_3DCoordinate(){};
      MR_3DCoordinate( MR_Int32 pX, MR_Int32 pY, MR_Int32 pZ){ mX = pX; mY=pY; mZ = pZ;};


};

inline MR_3DCoordinate operator+(const MR_3DCoordinate& lhs, const MR_3DCoordinate& rhs)
{
   return MR_3DCoordinate(
       lhs.mX + rhs.mX,
       lhs.mY + rhs.mY,
       lhs.mZ + rhs.mZ
   );
}

// Angles
typedef MR_Int16 MR_Angle;

#define MR_PI             2048
#define MR_2PI            4096

#define MR_NORMALIZE_ANGLE( pAngle ) ( (MR_Angle) (( 2*MR_2PI+(pAngle) )%MR_2PI) )

// rewritten due to emscripten precision issue
inline MR_Angle RAD_2_MR_ANGLE(double radians) 
{
   // convert to engine‑units (double)
   double units = radians * (double(MR_2PI) / (2.0 * std::numbers::pi));

   // round to nearest integer
   long rounded = std::lround(units);

   // wrap into [0, MR_2PI)
   rounded %= MR_2PI;
   if (rounded < 0) rounded += MR_2PI;

   return static_cast<MR_Angle>(rounded);
}

// Temporal unities
typedef MR_Int32  MR_SimulationTime; // In 1/1000th of seconds
                                     // Do not stay in a maze for more than 49 days
                                     // or the time will wrap-up

// Sin and Cos tables
MR_DllDeclare extern MR_Int16 MR_Sin[ MR_2PI ];
MR_DllDeclare extern MR_Int16 MR_Cos[ MR_2PI ];


#define MR_TRIGO_FRACT   16384 // unity of the trigo tables
#define MR_TRIGO_SHIFT      14

// Function prototypes
MR_DllDeclare void MR_InitTrigoTables();


#undef MR_DllDeclare

#endif
