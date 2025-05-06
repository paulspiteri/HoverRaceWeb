#include "Observer.h"

void MR_Observer::RenderGLView(const MR_ClientSession* pSession, const MR_MainCharacter* pViewingCharacter,
                               MR_SimulationTime pTime, const MR_UInt8* pBackImage)
{
    const MR_Level* lLevel = pSession->GetCurrentLevel();
    int lRoom = pViewingCharacter->mRoom;
    MR_Angle lOrientation = pViewingCharacter->mOrientation;

    int lDist = 3400;
    if( pTime < -3000 )
    {
        int lFactor = (-pTime-3000)*2/3;
        int rotateMRDegrees = lFactor*MR_2PI/11000; // some fraction of a game circle
        lOrientation = static_cast<MR_Int16>((lOrientation + rotateMRDegrees) % MR_2PI);

        lDist += lFactor;
    }

    MR_3DCoordinate lCameraPos;
    MR_3DCoordinate lCharacterPos = pViewingCharacter->mPosition;
    lCameraPos.mX  =lCharacterPos.mX - cos(lDist);
    lCameraPos.mY  = lCharacterPos.mY - sin(lDist);
    lCameraPos.mZ  = lCharacterPos.mZ + 1700;
    if( mLastGlCameraPos.has_value())
    {
        constexpr float XY_SMOOTHING = 0.75f;
        constexpr float Z_SMOOTHING = 0.667f;
        lCameraPos.mX = std::lerp(mLastGlCameraPos.value().mX, lCameraPos.mX, XY_SMOOTHING);
        lCameraPos.mY = std::lerp(mLastGlCameraPos.value().mY, lCameraPos.mY, XY_SMOOTHING);
        lCameraPos.mZ = std::lerp(mLastGlCameraPos.value().mZ, lCameraPos.mZ, Z_SMOOTHING);
    }
    mLastGlCameraPos = lCameraPos;
    mGLView.SetCameraPosition(lCameraPos, lOrientation);

    std::vector<Vertex> vertices;
    std::vector<uint16_t> vertexIdxs;

    int lTotalSections = lLevel->GetNbVisibleSurface(lRoom);
    const MR_SectionId* lFloorList = lLevel->GetVisibleFloorList(lRoom);
    const MR_SectionId* lCeilingList = lLevel->GetVisibleCeilingList(lRoom);
    for (int lCounter = 0; lCounter < lTotalSections; lCounter++)
    {
        DrawGLFloorOrCeiling(lLevel, lFloorList[lCounter], true, vertices, vertexIdxs);
        DrawGLFloorOrCeiling(lLevel, lCeilingList[lCounter], false, vertices, vertexIdxs);
    }


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

void MR_Observer::DrawGLSection(const MR_Level* pLevel, const MR_SectionId& pSectionId, std::vector<Vertex>& vertices,
                                std::vector<uint16_t>& vertexIdxs) const
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

    vertices.push_back(SwapYZ(makeVertex(lSectionShape->X(lVertexCount - 1), lSectionShape->Y(lVertexCount - 1),
                                         lSectionShape->ZMin())));
    vertices.push_back(SwapYZ(makeVertex(lSectionShape->X(lVertexCount - 1), lSectionShape->Y(lVertexCount - 1),
                                         lSectionShape->ZMax(), 1, 0, 0)));

    for (int lVertex = 0; lVertex < lVertexCount; lVertex++)
    {
        vertices.push_back(SwapYZ(makeVertex(lSectionShape->X(lVertex), lSectionShape->Y(lVertex),
                                             lSectionShape->ZMin())));
        vertices.push_back(SwapYZ(makeVertex(lSectionShape->X(lVertex), lSectionShape->Y(lVertex),
                                             lSectionShape->ZMax(), 1, 0, 0)));
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

void MR_Observer::DrawGLFloorOrCeiling(const MR_Level* pLevel, const MR_SectionId pSectionId, bool pFloor,
                                       std::vector<Vertex>& vertices, std::vector<uint16_t>& vertexIdxs)
{
    MR_Int32 lLevel;
    MR_PolygonShape* lShape;
    MR_SurfaceElement* lElement;

    if (pSectionId.mType == MR_SectionId::eRoom)
    {
        lShape = pLevel->GetRoomShape(pSectionId.mId);
        if (pFloor)
        {
            lLevel = lShape->ZMin();
            lElement = pLevel->GetRoomBottomElement(pSectionId.mId);
        }
        else
        {
            lLevel = lShape->ZMax();
            lElement = pLevel->GetRoomTopElement(pSectionId.mId);
        }
    }
    else
    {
        lShape = pLevel->GetFeatureShape(pSectionId.mId);
        if (!pFloor)
        {
            lLevel = lShape->ZMin();
            lElement = pLevel->GetFeatureBottomElement(pSectionId.mId);
        }
        else
        {
            lLevel = lShape->ZMax();
            lElement = pLevel->GetFeatureTopElement(pSectionId.mId);
        }
    }

    if (lElement != nullptr)
    {
        uint16_t lNbVertex = lShape->VertexCount();
        uint16_t baseIndex = vertices.size();

        // Add all vertices (assuming counter-clockwise order)
        for (uint16_t i = 0; i < lNbVertex; i++)
        {
            vertices.push_back(
                SwapYZ(makeVertex(lShape->X(i), lShape->Y(i), lLevel, pFloor ? 0.25 : 0 , 0.25, pFloor ? 0.2 : 0)));
        }
        // Triangulate using fan
        for (uint16_t j = 1; j < lNbVertex - 1; ++j)
        {
            vertexIdxs.push_back(baseIndex); // Center
            if (pFloor)
            {
                vertexIdxs.push_back(baseIndex + j); // Current
                vertexIdxs.push_back(baseIndex + j + 1); // Next
            }
            else
            {
                vertexIdxs.push_back(baseIndex + j + 1); // Next
                vertexIdxs.push_back(baseIndex + j); // Current
            }
        }
    }

    delete lShape;
}
