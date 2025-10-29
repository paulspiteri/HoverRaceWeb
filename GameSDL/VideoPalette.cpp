#include "VideoPalette.h"

#include <cstring>

#include "TrackCommonStuff.h"
#include "../VideoServices/3DViewport.h"
#include "../VideoServices/ColorPalette.h"


VideoPalette::VideoPalette( MR_RecordFile* pRecordFile, double gamma, double contract, double brightness )
{
    auto backPalette = ReadBackPaletteFromTrackFile(pRecordFile);

    NoMFC::PALETTEENTRY lPalette[256];

    int lCounter;

    if( gamma < 0.2 )
    {
        gamma = 0.2;
    }

    if( gamma > 4.0 )
    {
        gamma = 4.0;
    }

    if( contract > 1.0 )
    {
        contract = 1.0;
    }

    if( contract < 0.3 )
    {
        contract = 0.3;
    }

    if( brightness > 1 )
    {
        brightness = 1.0;
    }

    if( brightness < 0.3 )
    {
        brightness = 0.3;
    }

    NoMFC::PALETTEENTRY* lOurEntries = MR_GetColors( 1.0/gamma, contract*brightness, brightness-(contract*brightness) );

    for( lCounter = 0; lCounter<MR_BASIC_COLORS; lCounter++ )
    {
        lPalette[ MR_RESERVED_COLORS_BEGINNING+lCounter ] = lOurEntries[ lCounter ];
    }
    delete []lOurEntries;


    for( lCounter = 0; lCounter<MR_BACK_COLORS; lCounter++ )
    {
        lPalette[ MR_RESERVED_COLORS_BEGINNING+MR_BASIC_COLORS+lCounter ] =
           MR_ConvertColor( backPalette[ lCounter*3], backPalette[ lCounter*3+1], backPalette[ lCounter*3+2],
                            1.0/gamma, contract*brightness, brightness-(contract*brightness) );
    }

    memcpy(mPaletteEntries, lPalette, sizeof(mPaletteEntries));
}


MR_UInt8* VideoPalette::ReadBackPaletteFromTrackFile( MR_RecordFile* pRecordFile )
{
    MR_UInt8* lPalette = nullptr;
    // Read level background palette
    if(pRecordFile->GetNbRecords()>=3 )
    {
        pRecordFile->SelectRecord( 2 );

        {
            NoMFC::CArchive lArchive( pRecordFile->File(), NoMFC::CArchive::load );

            int lImageType;

            lArchive >> lImageType;

            if( lImageType == MR_RAWBITMAP )
            {
                lPalette = new MR_UInt8[ MR_BACK_COLORS*3 ];

                if( mBackImage == NULL )
                {
                    mBackImage = new MR_UInt8[ MR_BACK_X_RES*MR_BACK_Y_RES ];
                }

                lArchive.Read( lPalette, MR_BACK_COLORS*3 );
                lArchive.Read( mBackImage, MR_BACK_X_RES*MR_BACK_Y_RES );
            }
        }
    }

    return lPalette;
}

