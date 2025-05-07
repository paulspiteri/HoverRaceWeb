#include "Observer.h"
#include "../Model/Level.h"

void MR_Observer::RenderGLView(const MR_ClientSession* pSession, const MR_MainCharacter* pViewingCharacter,
                               MR_SimulationTime pTime, const MR_UInt8* pBackImage)
{
    const MR_Level* lLevel = pSession->GetCurrentLevel();
    MR_Angle lOrientation = pViewingCharacter->mOrientation;

    int lDist = 3400;
    if (pTime < -3000)
    {
        int lFactor = (-pTime - 3000) * 2 / 3;
        int rotateMRDegrees = lFactor * MR_2PI / 11000; // some fraction of a game circle
        lOrientation = static_cast<MR_Int16>((lOrientation + rotateMRDegrees) % MR_2PI);

        lDist += lFactor;
    }

    MR_3DCoordinate lCameraPos;
    MR_3DCoordinate lCharacterPos = pViewingCharacter->mPosition;
    lCameraPos.mX = lCharacterPos.mX - cos(lDist);
    lCameraPos.mY = lCharacterPos.mY - sin(lDist);
    lCameraPos.mZ = lCharacterPos.mZ + 1700;
    if (mLastGlCameraPos.has_value())
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

    int totalRooms = lLevel->GetRoomCount();
    for (int roomId = 0; roomId < totalRooms; roomId++)
    {
        auto roomShape = lLevel->GetRoomShape(roomId);
        DrawGLSection(lLevel, roomId, roomShape, vertices, vertexIdxs);

        auto floorTexture = lLevel->GetRoomBottomElement(roomId);
        if (floorTexture != nullptr)
        {
            DrawGLRoomFloor(roomShape, vertices, vertexIdxs);
        }

        auto ceilingTexture = lLevel->GetRoomTopElement(roomId);
        if (ceilingTexture != nullptr)
        {
            DrawGLRoomCeiling(roomShape, vertices, vertexIdxs);
        }


        int totalRoomFeatures = lLevel->GetFeatureCount(roomId);
        for (int roomFeatureIdx = 0; roomFeatureIdx < totalRoomFeatures; roomFeatureIdx++)
        {
            auto roomFeatureId = lLevel->GetFeature(roomId, roomFeatureIdx);
            auto roomFeatureShape = lLevel->GetFeatureShape(roomFeatureId);
            //DrawGLSection(roomFeatureShape, vertices, vertexIdxs);
            delete roomFeatureShape;
        }
        delete roomShape;
    }

    mGLView.SetWallVertices(vertices, vertexIdxs);
}

void MR_Observer::AddWallVertices(std::vector<Vertex>& vertices, std::vector<uint16_t>& vertexIdxs, MR_3DCoordinate lP0, MR_3DCoordinate lP1) const
{
    vertices.push_back(SwapYZ(makeVertex(lP0.mX, lP0.mY, lP0.mZ, 1, 0, 0)));
    vertices.push_back(SwapYZ(makeVertex(lP0.mX, lP0.mY, lP1.mZ,1, 0, 0)));
    vertices.push_back(SwapYZ(makeVertex(lP1.mX, lP1.mY, lP0.mZ, 1, 0, 0)));
    vertices.push_back(SwapYZ(makeVertex(lP1.mX, lP1.mY, lP1.mZ,1, 0, 0)));
    uint16_t latestVertexIdx = vertices.size() - 1;

    vertexIdxs.push_back(latestVertexIdx - 3);
    vertexIdxs.push_back(latestVertexIdx - 1);
    vertexIdxs.push_back(latestVertexIdx - 2);

    vertexIdxs.push_back(latestVertexIdx - 2);
    vertexIdxs.push_back(latestVertexIdx - 1);
    vertexIdxs.push_back(latestVertexIdx);
}

void MR_Observer::DrawGLSection(const MR_Level* pLevel, int pRoomId, const MR_PolygonShape* sectionShape,
                                std::vector<Vertex>& vertices, std::vector<uint16_t>& vertexIdxs) const
{
    MR_3DCoordinate lP0;
    MR_3DCoordinate lP1;

    MR_Int32 lFloorLevel = sectionShape->ZMin();
    MR_Int32 lCeilingLevel = sectionShape->ZMax();

    lP0.mX = sectionShape->X(0);
    lP0.mY = sectionShape->Y(0);

    int lVertexCount = sectionShape->VertexCount();

    for (int lVertex = 0; lVertex < lVertexCount; lVertex++)
    {
        int lNext = lVertex + 1;
        if (lNext == lVertexCount)
        {
            lNext = 0;
        }

        lP1.mX = sectionShape->X(lNext);
        lP1.mY = sectionShape->Y(lNext);

        MR_SurfaceElement* lElement = pLevel->GetRoomWallElement(pRoomId, lVertex);
        if (lElement != nullptr)
        {
            int lNeighbor = pLevel->GetNeighbor(pRoomId, lVertex);
            if (lNeighbor == -1)
            {
                lP0.mZ = lCeilingLevel;
                lP1.mZ = lFloorLevel;
                AddWallVertices(vertices, vertexIdxs, lP0, lP1);
            }
            else
            {
                MR_Int32 lNeighborFloor = pLevel->GetRoomBottomLevel(lNeighbor);
                MR_Int32 lNeighborCeiling = pLevel->GetRoomTopLevel(lNeighbor);

                if (lFloorLevel < lNeighborFloor)
                {
                    lP0.mZ = lNeighborFloor;
                    lP1.mZ = lFloorLevel;
                    AddWallVertices(vertices, vertexIdxs, lP0, lP1);
                }

                if (lCeilingLevel > lNeighborCeiling)
                {
                    lP0.mZ = lCeilingLevel;
                    lP1.mZ = lNeighborCeiling;
                    AddWallVertices(vertices, vertexIdxs, lP0, lP1);
                }
            }
        }

        lP0.mX = lP1.mX;
        lP0.mY = lP1.mY;
    }
}

void MR_Observer::DrawGLRoomFloor(const MR_PolygonShape* roomShape, std::vector<Vertex>& vertices, std::vector<uint16_t>& vertexIdxs)
{
    auto height = roomShape->ZMin();
    auto lNbVertex = roomShape->VertexCount();
    auto baseIndex = vertices.size();

    for (auto i = 0; i < lNbVertex; i++)
    {
        vertices.push_back(
            SwapYZ(makeVertex(roomShape->X(i), roomShape->Y(i), height, 0.8, 0.8, 0.8)));
    }
    // Triangulate using fan
    for (auto j = 1; j < lNbVertex - 1; ++j)
    {
        vertexIdxs.push_back(baseIndex); // Center
        vertexIdxs.push_back(baseIndex + j);
        vertexIdxs.push_back(baseIndex + j + 1);
    }
}

void MR_Observer::DrawGLRoomCeiling(const MR_PolygonShape* roomShape, std::vector<Vertex>& vertices, std::vector<uint16_t>& vertexIdxs)
{
    auto height = roomShape->ZMax();
    auto lNbVertex = roomShape->VertexCount();
    auto baseIndex = vertices.size();

    for (auto i = 0; i < lNbVertex; i++)
    {
        vertices.push_back(
            SwapYZ(makeVertex(roomShape->X(i), roomShape->Y(i), height, 0.2, 0.2, 0.2)));
    }
    // Triangulate using fan
    for (auto j = 1; j < lNbVertex - 1; ++j)
    {
        vertexIdxs.push_back(baseIndex); // Center
        vertexIdxs.push_back(baseIndex + j + 1);
        vertexIdxs.push_back(baseIndex + j);
    }
}
