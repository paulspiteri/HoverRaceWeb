#include "GLLevelLoader.h"

#include <optional>

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
        LoadRoomWalls(level, roomId, verts);
        LoadRoomFloor(level, roomId, verts);
        LoadRoomCeiling(level, roomId, verts);

        int totalRoomFeatures = level->GetFeatureCount(roomId);
        for (int roomFeatureIdx = 0; roomFeatureIdx < totalRoomFeatures; roomFeatureIdx++)
        {
            auto roomFeatureId = level->GetFeature(roomId, roomFeatureIdx);
            auto roomFeatureShape = level->GetFeatureShape(roomFeatureId);
            //DrawGLSection(roomFeatureShape, vertices, vertexIdxs);
            delete roomFeatureShape;
        }
    }

    glRenderer->BindVertices(verts);
    glRenderer->BindTextures();
}

void GLLevelLoader::LoadRoomWalls(const MR_Level* level, int roomId, VerticesData& verts) const
{
    auto roomShape = level->GetRoomShape(roomId);
    MR_3DCoordinate lP0;
    MR_3DCoordinate lP1;
    MR_Int32 lFloorLevel = roomShape->ZMin();
    MR_Int32 lCeilingLevel = roomShape->ZMax();
    lP0.mX = roomShape->X(0);
    lP0.mY = roomShape->Y(0);
    int lVertexCount = roomShape->VertexCount();
    for (int lVertex = 0; lVertex < lVertexCount; lVertex++)
    {
        int lNext = lVertex + 1;
        if (lNext == lVertexCount)
        {
            lNext = 0;
        }

        lP1.mX = roomShape->X(lNext);
        lP1.mY = roomShape->Y(lNext);

        auto surfaceElement = level->GetRoomWallElement(roomId, lVertex);

        int lNeighbor = level->GetNeighbor(roomId, lVertex);
        if (lNeighbor == -1)
        {
            lP0.mZ = lCeilingLevel;
            lP1.mZ = lFloorLevel;
            AddWallVertices(verts, lP0, lP1, surfaceElement);
        }
        else
        {
            MR_Int32 lNeighborFloor = level->GetRoomBottomLevel(lNeighbor);
            MR_Int32 lNeighborCeiling = level->GetRoomTopLevel(lNeighbor);

            if (lFloorLevel < lNeighborFloor)
            {
                lP0.mZ = lNeighborFloor;
                lP1.mZ = lFloorLevel;
                AddWallVertices(verts, lP0, lP1, surfaceElement);
            }

            if (lCeilingLevel > lNeighborCeiling)
            {
                lP0.mZ = lCeilingLevel;
                lP1.mZ = lNeighborCeiling;
                AddWallVertices(verts, lP0, lP1, surfaceElement);
            }
        }

        lP0.mX = lP1.mX;
        lP0.mY = lP1.mY;
    }
    delete roomShape;
}

void GLLevelLoader::LoadRoomFloor(const MR_Level* level, int roomId, VerticesData& verts) const
{
    auto roomShape = level->GetRoomShape(roomId);
    auto surfaceElement = level->GetRoomBottomElement(roomId);
    auto bitmap = surfaceElement->GetResBitmap();
    if (bitmap == nullptr)
    {
        return;
    }
    auto height = roomShape->ZMin();
    auto lNbVertex = roomShape->VertexCount();
    auto baseIndex = verts.vertices.size();
    auto textureAtlasId = glRenderer->LoadTexture(surfaceElement->mId.mClassId, bitmap);

    for (auto i = 0; i < lNbVertex; i++)
    {
        float u = roomShape->X(i) / bitmap->GetWidth();
        float v = roomShape->Y(i) / bitmap->GetHeight();
        verts.vertices.push_back(
            SwapYZ(makeVertex(roomShape->X(i), roomShape->Y(i), height,
                              1.0f, 1.0f, 1.0f, 1.0f,
                              u, v, textureAtlasId)));
    }

    // Triangulate using fan
    for (auto j = 1; j < lNbVertex - 1; ++j)
    {
        verts.indices.push_back(baseIndex);
        verts.indices.push_back(baseIndex + j);
        verts.indices.push_back(baseIndex + j + 1);
    }
    delete roomShape;
}

void GLLevelLoader::LoadRoomCeiling(const MR_Level* level, int roomId, VerticesData& verts) const
{
    auto roomShape = level->GetRoomShape(roomId);
    auto surfaceElement = level->GetRoomTopElement(roomId);
    auto bitmap = surfaceElement->GetResBitmap();
    if (bitmap == nullptr)
    {
        return;
    }
    auto height = roomShape->ZMax();
    auto lNbVertex = roomShape->VertexCount();
    auto baseIndex = verts.vertices.size();
    auto textureAtlasId = glRenderer->LoadTexture(surfaceElement->mId.mClassId, bitmap);

    for (auto i = 0; i < lNbVertex; i++)
    {
        float u = roomShape->X(i) / bitmap->GetWidth();
        float v = roomShape->Y(i) / bitmap->GetHeight();
        verts.vertices.push_back(
            SwapYZ(makeVertex(roomShape->X(i), roomShape->Y(i), height,
                              1.0f, 1.0f, 1.0f, 0.0f,
                              u, v, textureAtlasId)));
    }
    // Triangulate using fan
    for (auto j = 1; j < lNbVertex - 1; ++j)
    {
        verts.indices.push_back(baseIndex); // Center
        verts.indices.push_back(baseIndex + j + 1);
        verts.indices.push_back(baseIndex + j);
    }
    delete roomShape;
}

void GLLevelLoader::AddWallVertices(VerticesData& verts, MR_3DCoordinate lP0, MR_3DCoordinate lP1,
                                    MR_SurfaceElement* surfaceElement) const
{
    auto bitmap = surfaceElement->GetResBitmap();
    if (bitmap == nullptr)
    {
        return;
    }
    int textureAtlasId = textureAtlasId = glRenderer->LoadTexture(surfaceElement->mId.mClassId, bitmap);
    float u0 = lP0.mX / bitmap->GetWidth();
    float v0 = lP0.mZ / bitmap->GetHeight();
    float u1 = lP1.mX / bitmap->GetWidth();
    float v1 = lP1.mZ / bitmap->GetHeight();


    verts.vertices.push_back(SwapYZ(makeVertex(lP0.mX, lP0.mY, lP0.mZ, 1, 0, 0, 1, u0, v0, textureAtlasId)));
    verts.vertices.push_back(SwapYZ(makeVertex(lP0.mX, lP0.mY, lP1.mZ, 1, 0, 0, 1, u0, v1, textureAtlasId)));
    verts.vertices.push_back(SwapYZ(makeVertex(lP1.mX, lP1.mY, lP0.mZ, 1, 0, 0, 1, u1, v0, textureAtlasId)));
    verts.vertices.push_back(SwapYZ(makeVertex(lP1.mX, lP1.mY, lP1.mZ, 1, 0, 0, 1, u1, v1, textureAtlasId)));
    uint16_t latestVertexIdx = verts.vertices.size() - 1;

    verts.indices.push_back(latestVertexIdx - 3);
    verts.indices.push_back(latestVertexIdx - 1);
    verts.indices.push_back(latestVertexIdx - 2);

    verts.indices.push_back(latestVertexIdx - 2);
    verts.indices.push_back(latestVertexIdx - 1);
    verts.indices.push_back(latestVertexIdx);
}
