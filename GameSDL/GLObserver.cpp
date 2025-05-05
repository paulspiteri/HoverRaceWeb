#include "Observer.h"

void MR_Observer::RenderGLView(const MR_ClientSession* pSession, const MR_MainCharacter* pViewingCharacter,
                               MR_SimulationTime pTime, const MR_UInt8* pBackImage)
{
    const MR_Level* lLevel = pSession->GetCurrentLevel();

    MR_3DCoordinate lCharacterPos = pViewingCharacter->mPosition;
    MR_Angle lOrientation = pViewingCharacter->mOrientation;
    int lRoom = pViewingCharacter->mRoom;

    lCharacterPos.mZ += 1800; // Fix the eyes at 1m80

    mGLView.SetCameraPosition(lCharacterPos, lOrientation);

    std::vector<Vertex> vertices;
    std::vector<uint16_t> vertexIdxs;

    // Draw the walls and features of the visible rooms
    int lRoomCount;
    lLevel->GetVisibleZones(lRoom, lRoomCount);

    for (int lCounter = -1; lCounter < lRoomCount; lCounter++)
    {
        // Draw the room and all the features
        for (int lCounter2 = -1; lCounter2 < lLevel->GetFeatureCount(lRoom); lCounter2++)
        {
            MR_SectionId lSectionId{};
            if (lCounter2 == -1)
            {
                lSectionId.mType = MR_SectionId::eRoom;
                lSectionId.mId = lRoom;
            }
            else
            {
                lSectionId.mType = MR_SectionId::eFeature;
                lSectionId.mId = lLevel->GetFeature(lRoom, lCounter2);
            }
            DrawGLSection(lLevel, lSectionId, vertices, vertexIdxs);
        }
    }

    mGLView.SetWallVertices(vertices, vertexIdxs);
}

void MR_Observer::DrawGLSection(const MR_Level* pLevel, const MR_SectionId& pSectionId, std::vector<Vertex>& vertices, std::vector<uint16_t>& vertexIdxs) const
{
    MR_PolygonShape* lSectionShape;

    if (pSectionId.mType == MR_SectionId::eRoom)
    {
        lSectionShape = pLevel->GetRoomShape(pSectionId.mId);
    }
    else
    {
        lSectionShape = pLevel->GetFeatureShape(pSectionId.mId);
    }

    int lVertexCount = lSectionShape->VertexCount();

    vertices.push_back(SwapYZ(makeVertex(lSectionShape->X(lVertexCount-1 ), lSectionShape->Y(lVertexCount-1), lSectionShape->ZMin())));
    vertices.push_back(SwapYZ(makeVertex(lSectionShape->X(lVertexCount-1 ), lSectionShape->Y(lVertexCount-1 ), lSectionShape->ZMax(), 1,0,0)));

    for (int lVertex = 0; lVertex < lVertexCount; lVertex++)
    {
        vertices.push_back(SwapYZ(makeVertex(lSectionShape->X(lVertex), lSectionShape->Y(lVertex), lSectionShape->ZMin())));
        vertices.push_back(SwapYZ(makeVertex(lSectionShape->X(lVertex), lSectionShape->Y(lVertex), lSectionShape->ZMax(),1,0,0)));
        uint16_t latestVertexIdx = vertices.size() - 1;

        vertexIdxs.push_back(latestVertexIdx - 3);
        vertexIdxs.push_back(latestVertexIdx - 2);
        vertexIdxs.push_back(latestVertexIdx - 1);

        vertexIdxs.push_back(latestVertexIdx - 2);
        vertexIdxs.push_back(latestVertexIdx);
        vertexIdxs.push_back(latestVertexIdx - 1);
    }

    delete lSectionShape;
}
