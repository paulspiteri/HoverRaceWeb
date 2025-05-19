#include "GLLevelLoader.h"

#include "../ObjFacTools/ResBitmap.h"
#include "../ObjFacTools/BitmapSurface.h"
#include <numbers>

GLLevelLoader::GLLevelLoader(GLRenderer* renderer): glRenderer(renderer)
{
}

void GLLevelLoader::LoadLevel(const MR_Level* level, const MR_UInt8* backImage)
{
    int totalRooms = level->GetRoomCount();
    for (int roomId = 0; roomId < totalRooms; roomId++)
    {
        LoadRoomWalls(level, roomId);
        LoadRoomFloor(level, roomId);
        LoadRoomCeiling(level, roomId);

        int totalRoomFeatures = level->GetFeatureCount(roomId);
        for (int roomFeatureIdx = 0; roomFeatureIdx < totalRoomFeatures; roomFeatureIdx++)
        {
            auto roomFeatureId = level->GetFeature(roomId, roomFeatureIdx);
            LoadFeatureWalls(level, roomFeatureId);
            LoadFeatureFloor(level, roomFeatureId);
            LoadFeatureCeiling(level, roomFeatureId);
        }
    }

    LoadBackground(backImage);
    glRenderer->BindBackgroundVertices(bkgVerts);
    glRenderer->BindWorldVertices(worldVerts);
    glRenderer->BindWallVertices(wallVerts);
    glRenderer->BindWorldTextures();
}

void GLLevelLoader::LoadBackground(const MR_UInt8* backImage)
{
    glRenderer->BindBackgroundTexture(backImage);

    const int segments = 24; // Number of segments around the cylinder
    const float radius = 200000.0f;
    const float height = 160000.0f;
    const float centerX = 0.0f;
    const float centerY = 0.0f;
    const float centerZ = 0.0f;

    const float startAngleOffset = -(std::numbers::pi / 2.0f); // -90 degrees
    for (int i = 0; i <= segments; i++)
    {
        float angle = 2.0f * std::numbers::pi * i / segments + startAngleOffset;
        float x = centerX + radius * cos(angle);
        float z = centerZ + radius * sin(angle);
        float u = float(i) / segments;

        bkgVerts.vertices.push_back(makeVertex(x, centerY, z, u, 1));
        bkgVerts.vertices.push_back(makeVertex(x, centerY + height, z, u, 0));
    }

    for (int i = 0; i < segments; i++)
    {
        int current = i * 2;
        int next = (i + 1) * 2;
        bkgVerts.indices.push_back(current);
        bkgVerts.indices.push_back(current + 1);
        bkgVerts.indices.push_back(next);

        bkgVerts.indices.push_back(next);
        bkgVerts.indices.push_back(current + 1);
        bkgVerts.indices.push_back(next + 1);
    }
}

void GLLevelLoader::LoadRoomWalls(const MR_Level* level, int roomId)
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
            AddWall(lP0, lP1, surfaceElement);
        }
        else
        {
            MR_Int32 lNeighborFloor = level->GetRoomBottomLevel(lNeighbor);
            MR_Int32 lNeighborCeiling = level->GetRoomTopLevel(lNeighbor);

            if (lFloorLevel < lNeighborFloor)
            {
                lP0.mZ = lNeighborFloor;
                lP1.mZ = lFloorLevel;
                AddWall(lP0, lP1, surfaceElement);
            }

            if (lCeilingLevel > lNeighborCeiling)
            {
                lP0.mZ = lCeilingLevel;
                lP1.mZ = lNeighborCeiling;
                AddWall(lP0, lP1, surfaceElement);
            }
        }

        lP0.mX = lP1.mX;
        lP0.mY = lP1.mY;
    }
    delete roomShape;
}

void GLLevelLoader::LoadRoomFloor(const MR_Level* level, int roomId)
{
    auto roomShape = level->GetRoomShape(roomId);
    auto surfaceElement = level->GetRoomBottomElement(roomId);
    LoadFloor(roomShape, surfaceElement);
    delete roomShape;
}

void GLLevelLoader::LoadRoomCeiling(const MR_Level* level, int roomId)
{
    auto roomShape = level->GetRoomShape(roomId);
    auto surfaceElement = level->GetRoomTopElement(roomId);
    LoadCeiling(roomShape, surfaceElement, false);
    delete roomShape;
}

void GLLevelLoader::LoadFeatureFloor(const MR_Level* level, int featureId)
{
    auto featureShape = level->GetFeatureShape(featureId);
    auto surfaceElement = level->GetFeatureBottomElement(featureId);
    LoadFloor(featureShape, surfaceElement, true);
    delete featureShape;
}

void GLLevelLoader::LoadFeatureCeiling(const MR_Level* level, int featureId)
{
    auto featureShape = level->GetFeatureShape(featureId);
    auto surfaceElement = level->GetFeatureTopElement(featureId);
    LoadCeiling(featureShape, surfaceElement, true);
    delete featureShape;
}

void GLLevelLoader::LoadFeatureWalls(const MR_Level* level, int featureId)
{
    auto featureShape = level->GetFeatureShape(featureId);
    int lVertexCount = featureShape->VertexCount();

    MR_3DCoordinate lP0;
    MR_3DCoordinate lP1;
    lP0.mZ = featureShape->ZMax();
    lP1.mX = featureShape->X( 0 );
    lP1.mY = featureShape->Y( 0 );
    lP1.mZ = featureShape->ZMin();

    for(int lVertex = 0; lVertex < lVertexCount; lVertex++)
    {
        int lNext = lVertex + 1;
        if( lNext == lVertexCount )
        {
            lNext = 0;
        }
        lP0.mX = featureShape->X( lNext );
        lP0.mY = featureShape->Y( lNext );

        MR_SurfaceElement* surfaceElement = level->GetFeatureWallElement(featureId, lVertex);
        if(surfaceElement != nullptr)
        {
            AddWall(lP0, lP1, surfaceElement);
        }

        lP1.mX = lP0.mX;
        lP1.mY = lP0.mY;
    }
    delete featureShape;
}

void GLLevelLoader::LoadFloor(MR_PolygonShape* shape, MR_SurfaceElement* surfaceElement, bool upsideDown)
{
    auto bitmap = surfaceElement->GetResBitmap();
    if (bitmap == nullptr)
    {
        return;
    }
    auto height = shape->ZMin();
    auto lNbVertex = shape->VertexCount();
    auto baseIndex = worldVerts.vertices.size();
    auto textureAtlasId = glRenderer->LoadTexture(surfaceElement->mId.mClassId, bitmap);

    for (auto i = 0; i < lNbVertex; i++)
    {
        float u = shape->X(i) / static_cast<float>(bitmap->GetWidth());
        float v = shape->Y(i) / static_cast<float>(bitmap->GetHeight());
        worldVerts.vertices.push_back(
            SwapYZ(makeVertexWithTextureId(shape->X(i), shape->Y(i), height, u, v, textureAtlasId)));
    }

    // Triangulate using fan
    for (auto j = 1; j < lNbVertex - 1; ++j)
    {
        worldVerts.indices.push_back(baseIndex);
        worldVerts.indices.push_back(baseIndex + j + (upsideDown ? 1 : 0));
        worldVerts.indices.push_back(baseIndex + j + (upsideDown ? 0 : 1));
    }
}

void GLLevelLoader::LoadCeiling(MR_PolygonShape* shape, MR_SurfaceElement* surfaceElement, bool upsideDown)
{
    auto bitmap = surfaceElement->GetResBitmap();
    if (bitmap == nullptr)
    {
        return;
    }
    auto height = shape->ZMax();
    auto lNbVertex = shape->VertexCount();
    auto baseIndex = worldVerts.vertices.size();
    auto textureAtlasId = glRenderer->LoadTexture(surfaceElement->mId.mClassId, bitmap);

    for (auto i = 0; i < lNbVertex; i++)
    {
        float u = shape->X(i) / static_cast<float>(bitmap->GetWidth());
        float v = shape->Y(i) / static_cast<float>(bitmap->GetHeight());
        worldVerts.vertices.push_back(
            SwapYZ(makeVertexWithTextureId(shape->X(i), shape->Y(i), height, u, v, textureAtlasId)));
    }
    // Triangulate using fan
    for (auto j = 1; j < lNbVertex - 1; ++j)
    {
        worldVerts.indices.push_back(baseIndex); // Center
        worldVerts.indices.push_back(baseIndex + j + (upsideDown ? 0 : 1));
        worldVerts.indices.push_back(baseIndex + j + (upsideDown ? 1 : 0));
    }
}

void GLLevelLoader::AddWall(MR_3DCoordinate lP0, MR_3DCoordinate lP1, MR_SurfaceElement* surfaceElement)
{
    auto bitmap = surfaceElement->GetResBitmap();
    if (bitmap == nullptr)
    {
        return;
    }
    int textureAtlasId = glRenderer->LoadTexture(surfaceElement->mId.mClassId, bitmap);
    auto bitmap2 = surfaceElement->GetResBitmap2();
    if (bitmap2 != nullptr)
    {
        MR_UInt32 id2 = surfaceElement->mId.mClassId | 0x80000000;  // turn on high bit for bitmap2
        glRenderer->LoadTexture(id2, bitmap2);
    }
    float dx = lP1.mX - lP0.mX;
    float dy = lP1.mY - lP0.mY;
    auto wallLength = sqrt(dx * dx + dy * dy);
    int wallHeight = lP0.mZ - lP1.mZ;

    float u0 = 0.0f;
    float u1 = 1.0f;
    float v0 = 1.0f;
    float v1 = 0.0f;

    auto vstretchBitmap = dynamic_cast<MR_VStretchBitmapSurface*>(surfaceElement);
    if (vstretchBitmap != nullptr)
    {
        if (wallHeight > 0)
        {
            u1 = wallLength / wallHeight; // repeat width every wall height

            int lDivisor = 1 + (wallHeight - 1) / vstretchBitmap->GetMaxHeight();
            if (lDivisor > 1)
            {
                v0 = v0 * lDivisor;
                u1 = u1 * lDivisor;
            }
        }
    }
    else
    {
        u1 = wallLength / bitmap->GetWidth(); // tile horizontally

        v0 = 1.0f;
        v1 = 1.0f - ((float)wallHeight / bitmap->GetHeight());
    }

    int rotationSpeed = 0;
    auto bitmapSurface = dynamic_cast<MR_BitmapSurface*>(surfaceElement);
    if (bitmapSurface != nullptr)
    {
        rotationSpeed = bitmapSurface->GetRotationSpeed();
    }
    wallVerts.vertices.push_back(
        SwapYZ(makeWallVertex(lP0.mX, lP0.mY, lP0.mZ, u0, v0, textureAtlasId, rotationSpeed)));
    wallVerts.vertices.push_back(
        SwapYZ(makeWallVertex(lP0.mX, lP0.mY, lP1.mZ, u0, v1, textureAtlasId, rotationSpeed)));
    wallVerts.vertices.push_back(
        SwapYZ(makeWallVertex(lP1.mX, lP1.mY, lP0.mZ, u1, v0, textureAtlasId, rotationSpeed)));
    wallVerts.vertices.push_back(
        SwapYZ(makeWallVertex(lP1.mX, lP1.mY, lP1.mZ, u1, v1, textureAtlasId, rotationSpeed)));
    uint16_t latestVertexIdx = wallVerts.vertices.size() - 1;

    wallVerts.indices.push_back(latestVertexIdx - 3);
    wallVerts.indices.push_back(latestVertexIdx - 1);
    wallVerts.indices.push_back(latestVertexIdx - 2);

    wallVerts.indices.push_back(latestVertexIdx - 2);
    wallVerts.indices.push_back(latestVertexIdx - 1);
    wallVerts.indices.push_back(latestVertexIdx);
}
