// MainCharacterState.h
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

#ifndef MAIN_CHARACTER_STATE_H
#define MAIN_CHARACTER_STATE_H

#include "../Util/BitPacking.h"

// Main character network state structure (18 bytes)
class MR_MainCharacterState: private MR_BitPack
{
	friend class MR_MainCharacter;

	// Packing description
	//                     Offset  len   Prec
	#define  MC_POSX           0,   26,     5
	#define  MC_POSY          26,   26,     5
	#define  MC_POSZ          52,   15,     0
	#define  MC_ROOM          67,   11,     0
	#define  MC_ORIENTATION   78,    9,     3
	#define  MC_SPEED_X_256  111,   14,     2
	#define  MC_SPEED_Y_256  125,   14,     2
	#define  MC_SPEED_Z_256  139,    9,     2
	#define  MC_CONTROL_ST   148,   15,     0
	#define  MC_ON_FLOOR     163,    1,     0
	#define  MC_HOVER_MODEL  164,    3,     0
	#define  MC_PADDING      167,    9,     0

	// Total                 146  = 18 bytes
	MR_UInt8  mFieldList[18];

public:
	// Allow access to raw data for serialization
	const MR_UInt8* GetData() const { return mFieldList; }
	MR_UInt8* GetData() { return mFieldList; }
};

#endif
