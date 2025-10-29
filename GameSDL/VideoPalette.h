#pragma once
#include "../VideoServices/ColorPaletteEntry.h"

class MR_RecordFile;

class VideoPalette
{
    NoMFC::PALETTEENTRY  mPaletteEntries[256]{};
    MR_UInt8*         mBackImage;

    MR_UInt8* ReadBackPaletteFromTrackFile( MR_RecordFile* pRecordFile );

public:
    VideoPalette( MR_RecordFile* pRecordFile, double gamma, double contract, double brightness );
    NoMFC::PALETTEENTRY* GetPalette() { return mPaletteEntries; }
    const MR_UInt8* GetBackImage()const { return mBackImage; }
};
