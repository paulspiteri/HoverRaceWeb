#include "GLLevelLoader.h"
#include "../ObjFacTools/ResBitmap.h"

GLLevelLoader::GLLevelLoader(GLRenderer* renderer): glRenderer(renderer)
{
}

void GLLevelLoader::LoadLevel(const MR_Level* level)
{
    VerticesData verts;

    int totalRooms = level->GetRoomCount();
    for (int roomId = 0; roomId < totalRooms; roomId++)
    {
        auto roomShape = level->GetRoomShape(roomId);
        LoadRoom(level, roomId, roomShape, verts);

        auto floorTexture = level->GetRoomBottomElement(roomId);
        if (floorTexture != nullptr)    // these null checks may be superfluous
        {
            auto bitmap = floorTexture->GetResBitmap();
            if (bitmap != nullptr)
            {
                glRenderer->LoadTexture(floorTexture->mId.mClassId, bitmap);
                LoadRoomFloor(roomShape, verts, bitmap);
            }
        }

        auto ceilingTexture = level->GetRoomTopElement(roomId);
        if (ceilingTexture != nullptr)
        {
            LoadRoomCeiling(roomShape, verts);
        }

        int totalRoomFeatures = level->GetFeatureCount(roomId);
        for (int roomFeatureIdx = 0; roomFeatureIdx < totalRoomFeatures; roomFeatureIdx++)
        {
            auto roomFeatureId = level->GetFeature(roomId, roomFeatureIdx);
            auto roomFeatureShape = level->GetFeatureShape(roomFeatureId);
            //DrawGLSection(roomFeatureShape, vertices, vertexIdxs);
            delete roomFeatureShape;
        }
        delete roomShape;
    }

    glRenderer->BindVertices(verts);
    glRenderer->BindTextures();
}

void GLLevelLoader::LoadRoom(const MR_Level* pLevel, int pRoomId, const MR_PolygonShape* sectionShape,
                               VerticesData& verts) const
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
                AddWallVertices(verts, lP0, lP1);
            }
            else
            {
                MR_Int32 lNeighborFloor = pLevel->GetRoomBottomLevel(lNeighbor);
                MR_Int32 lNeighborCeiling = pLevel->GetRoomTopLevel(lNeighbor);

                if (lFloorLevel < lNeighborFloor)
                {
                    lP0.mZ = lNeighborFloor;
                    lP1.mZ = lFloorLevel;
                    AddWallVertices(verts, lP0, lP1);
                }

                if (lCeilingLevel > lNeighborCeiling)
                {
                    lP0.mZ = lCeilingLevel;
                    lP1.mZ = lNeighborCeiling;
                    AddWallVertices(verts, lP0, lP1);
                }
            }
        }

        lP0.mX = lP1.mX;
        lP0.mY = lP1.mY;
    }
}

void GLLevelLoader::LoadRoomFloor(const MR_PolygonShape* roomShape, VerticesData& verts, MR_ResBitmap* bitmap) const
{
    auto height = roomShape->ZMin();
    auto lNbVertex = roomShape->VertexCount();
    auto baseIndex = verts.vertices.size();

    for (auto i = 0; i < lNbVertex; i++)
    {
        float u = roomShape->X(i) / bitmap->GetWidth();
        float v = roomShape->Y(i) / bitmap->GetHeight();

        verts.vertices.push_back(
            SwapYZ(makeVertex(roomShape->X(i), roomShape->Y(i), height,
                              1.0f, 1.0f, 1.0f, 1.0f,
                              u, v, 4)));
    }

    // Triangulate using fan
    for (auto j = 1; j < lNbVertex - 1; ++j)
    {
        verts.indices.push_back(baseIndex);
        verts.indices.push_back(baseIndex + j);
        verts.indices.push_back(baseIndex + j + 1);
    }
}

void GLLevelLoader::LoadRoomCeiling(const MR_PolygonShape* roomShape, VerticesData& verts) const
{
    auto height = roomShape->ZMax();
    auto lNbVertex = roomShape->VertexCount();
    auto baseIndex = verts.vertices.size();

    for (auto i = 0; i < lNbVertex; i++)
    {
        verts.vertices.push_back(
            SwapYZ(makeVertex(roomShape->X(i), roomShape->Y(i), height, 0.25f, 0.4f, 0.25f, 0)));
    }
    // Triangulate using fan
    for (auto j = 1; j < lNbVertex - 1; ++j)
    {
        verts.indices.push_back(baseIndex); // Center
        verts.indices.push_back(baseIndex + j + 1);
        verts.indices.push_back(baseIndex + j);
    }
}

void GLLevelLoader::AddWallVertices(VerticesData& verts, MR_3DCoordinate lP0, MR_3DCoordinate lP1) const
{
    verts.vertices.push_back(SwapYZ(makeVertex(lP0.mX, lP0.mY, lP0.mZ, 1, 0, 0)));
    verts.vertices.push_back(SwapYZ(makeVertex(lP0.mX, lP0.mY, lP1.mZ, 1, 0, 0)));
    verts.vertices.push_back(SwapYZ(makeVertex(lP1.mX, lP1.mY, lP0.mZ, 1, 0, 0)));
    verts.vertices.push_back(SwapYZ(makeVertex(lP1.mX, lP1.mY, lP1.mZ, 1, 0, 0)));
    uint16_t latestVertexIdx = verts.vertices.size() - 1;

    verts.indices.push_back(latestVertexIdx - 3);
    verts.indices.push_back(latestVertexIdx - 1);
    verts.indices.push_back(latestVertexIdx - 2);

    verts.indices.push_back(latestVertexIdx - 2);
    verts.indices.push_back(latestVertexIdx - 1);
    verts.indices.push_back(latestVertexIdx);
}
