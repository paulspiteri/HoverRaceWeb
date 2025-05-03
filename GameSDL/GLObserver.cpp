#include "Observer.h"

void MR_Observer::RenderGLView( const MR_ClientSession* pSession, const MR_MainCharacter* pViewingCharacter, MR_SimulationTime pTime, const MR_UInt8* pBackImage)
{
    const MR_Level* lLevel = pSession->GetCurrentLevel();
    mWireFrameView.Clear( 0 );

    MR_3DCoordinate lCharacterPos   = pViewingCharacter->mPosition;
    MR_Angle        lOrientation    = pViewingCharacter->mOrientation;
    int             lRoom           = pViewingCharacter->mRoom;

    lCharacterPos.mZ += 1800; // Fix the eyes at 1m80

    mWireFrameView.SetupCameraPosition(lCharacterPos, lOrientation, mScroll);

    // Draw the walls and features of the visible rooms
    int        lRoomCount;
    const int* lRoomList = lLevel->GetVisibleZones( lRoom, lRoomCount );

    for( int lCounter = -1; lCounter < lRoomCount; lCounter++ )
    {
        int      lRoomId;

        if( lCounter == -1 )
        {
            lRoomId = lRoom;
        }
        else
        {
            lRoomId = lRoomList[ lCounter ];
        }

        // Draw the room and all the features
        for( int lCounter2 = -1; lCounter2 < lLevel->GetFeatureCount( lRoom ); lCounter2++ )
        {
            MR_SectionId lSectionId;
            if( lCounter2 == -1 )
            {
                lSectionId.mType  = MR_SectionId::eRoom;
                lSectionId.mId    = lRoom;
            }
            else
            {
                lSectionId.mType  = MR_SectionId::eFeature;
                lSectionId.mId    = lLevel->GetFeature( lRoom, lCounter2 );
            }
            DrawGLSection( lLevel, lSectionId);
        }
    }
}

void MR_Observer::DrawGLSection( const MR_Level* pLevel, const MR_SectionId& pSectionId)
{
   MR_PolygonShape* lSectionShape;

   if( pSectionId.mType == MR_SectionId::eRoom )
   {
      lSectionShape = pLevel->GetRoomShape( pSectionId.mId );
   }
   else
   {
      lSectionShape = pLevel->GetFeatureShape( pSectionId.mId );
   }

   // Draw the contour

   int lVertexCount = lSectionShape->VertexCount();
   MR_3DCoordinate previous;
   previous.mX = lSectionShape->X( lVertexCount-1 );
   previous.mY = lSectionShape->Y( lVertexCount-1 );

   for( int lVertex = 0; lVertex < lVertexCount; lVertex++ )
   {
      MR_3DCoordinate current;
      current.mX = lSectionShape->X( lVertex );
      current.mY = lSectionShape->Y( lVertex );

      MR_3DCoordinate topLineFrom, topLineTo;
      topLineFrom.mX = previous.mX;
      topLineFrom.mY = previous.mY;
      topLineFrom.mZ = lSectionShape->ZMax();
      topLineTo.mX = current.mX;
      topLineTo.mY = current.mY;
      topLineTo.mZ = lSectionShape->ZMax();
      // horizontal line (top)
      //mWireFrameView.DrawWFLine( topLineFrom, topLineTo, pColor );

      MR_3DCoordinate bottomLineFrom, BottomLineTo;
      bottomLineFrom.mX = previous.mX;
      bottomLineFrom.mY = previous.mY;
      bottomLineFrom.mZ = lSectionShape->ZMin();
      BottomLineTo.mX = current.mX;
      BottomLineTo.mY = current.mY;
      BottomLineTo.mZ = lSectionShape->ZMin();
      // horizontal line (bottom)
      //mWireFrameView.DrawWFLine( bottomLineFrom, BottomLineTo, pColor );

      MR_3DCoordinate rightLineFrom, rightLineTo;
      rightLineFrom.mX = previous.mX;
      rightLineFrom.mY = previous.mY;
      rightLineFrom.mZ = lSectionShape->ZMax();
      rightLineTo.mX = previous.mX;
      rightLineTo.mY = previous.mY;
      rightLineTo.mZ = lSectionShape->ZMin();
      // vertical line (right)
      //mWireFrameView.DrawWFLine( rightLineFrom, rightLineTo, pColor );

      MR_3DCoordinate leftLineFrom, leftLineTo;
      leftLineFrom.mX = current.mX;
      leftLineFrom.mY = current.mY;
      leftLineFrom.mZ = lSectionShape->ZMax();
      leftLineTo.mX = current.mX;
      leftLineTo.mY = current.mY;
      leftLineTo.mZ = lSectionShape->ZMin();
      // vertical line (left)
      //mWireFrameView.DrawWFLine( leftLineFrom, leftLineTo, pColor );

      previous = current;
   }

   delete lSectionShape;
}
