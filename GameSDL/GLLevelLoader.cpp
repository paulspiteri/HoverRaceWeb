#include "GLLevelLoader.h"

#include "../ObjFacTools/ResBitmap.h"
#include "../ObjFacTools/BitmapSurface.h"
#include "../ObjFacTools/FreeElementBase.h"
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

        LoadRoomFreeElements(level, roomId);
    }

    LoadBackground(backImage);
    glRenderer->BindBackgroundVertices(bkgVerts);
    glRenderer->BindWorldVertices(worldVerts);
    glRenderer->BindWallVertices(wallVerts);
    glRenderer->BindFreeElementVertices(freeElementVerts);
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
    std::unique_ptr<MR_PolygonShape> roomShape(level->GetRoomShape(roomId));
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
}

void GLLevelLoader::LoadRoomFloor(const MR_Level* level, int roomId)
{
    std::unique_ptr<MR_PolygonShape> roomShape(level->GetRoomShape(roomId));
    auto surfaceElement = level->GetRoomBottomElement(roomId);
    if (surfaceElement->mId.mClassId == 63)
    {
        int waterLevel = roomShape->ZMax();
        for (int i = 0; i < roomShape->VertexCount(); i++)
        {
            int neighborId = level->GetNeighbor(roomId, i);
            if (neighborId != -1)
            {
                auto neighborRoomShape = level->GetRoomShape(neighborId);
                waterLevel = std::min(neighborRoomShape->ZMin(), waterLevel);
                delete neighborRoomShape;
            }
        }
        //LoadWater(roomShape, surfaceElement, waterLevel);
    }
    LoadFloor(roomShape.get(), surfaceElement);
}

void GLLevelLoader::LoadRoomCeiling(const MR_Level* level, int roomId)
{
    std::unique_ptr<MR_PolygonShape> roomShape(level->GetRoomShape(roomId));
    auto surfaceElement = level->GetRoomTopElement(roomId);
    LoadCeiling(roomShape.get(), surfaceElement, false);
}

void GLLevelLoader::LoadFeatureFloor(const MR_Level* level, int featureId)
{
    std::unique_ptr<MR_PolygonShape> featureShape(level->GetFeatureShape(featureId));
    auto surfaceElement = level->GetFeatureBottomElement(featureId);
    LoadFloor(featureShape.get(), surfaceElement, true);
}

void GLLevelLoader::LoadFeatureCeiling(const MR_Level* level, int featureId)
{
    std::unique_ptr<MR_PolygonShape> featureShape(level->GetFeatureShape(featureId));
    auto surfaceElement = level->GetFeatureTopElement(featureId);
    LoadCeiling(featureShape.get(), surfaceElement, true);
}

void GLLevelLoader::LoadFeatureWalls(const MR_Level* level, int featureId)
{
    std::unique_ptr<MR_PolygonShape> featureShape(level->GetFeatureShape(featureId));
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

void GLLevelLoader::LoadWater(MR_PolygonShape* shape, MR_SurfaceElement* surfaceElement, MR_Int32 level)
{
    auto bitmap = surfaceElement->GetResBitmap();
    auto lNbVertex = shape->VertexCount();
    auto baseIndex = worldVerts.vertices.size();
    auto textureAtlasId = glRenderer->LoadTexture(surfaceElement->mId.mClassId, bitmap);

    for (auto i = 0; i < lNbVertex; i++)
    {
        float u = shape->X(i) / static_cast<float>(bitmap->GetWidth());
        float v = shape->Y(i) / static_cast<float>(bitmap->GetHeight());
        worldVerts.vertices.push_back(
            SwapYZ(makeVertexWithTextureId(shape->X(i), shape->Y(i), level, u, v, textureAtlasId)));
    }

    // Triangulate using fan
    for (auto j = 1; j < lNbVertex - 1; ++j)
    {
        worldVerts.indices.push_back(baseIndex);
        worldVerts.indices.push_back(baseIndex + j + 0);
        worldVerts.indices.push_back(baseIndex + j + 1);
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

    std::optional<int> maxHeight;
    auto vstretchBitmap = dynamic_cast<MR_VStretchBitmapSurface*>(surfaceElement);
    if (vstretchBitmap != nullptr)
    {
        maxHeight = vstretchBitmap->GetMaxHeight();
    }

    auto bitmap2 = surfaceElement->GetResBitmap2();
    if (bitmap2 != nullptr && bitmap2 != bitmap)
    {
        MR_UInt32 id2 = surfaceElement->mId.mClassId | 0x80000000;  // turn on high bit for bitmap2
        glRenderer->LoadTexture(id2, bitmap2);

        int rotationSpeed = 0, rotationLength = 0;
        auto bitmapSurface = dynamic_cast<MR_BitmapSurface*>(surfaceElement);
        if (bitmapSurface != nullptr)
        {
            rotationSpeed = bitmapSurface->GetRotationSpeed();
            rotationLength = bitmapSurface->GetRotationLen();
        }
        AddAnimatedWall(lP0, lP1, textureAtlasId, bitmap->GetWidth(), bitmap->GetHeight(), maxHeight, rotationSpeed, rotationLength);
    }
    else
    {
        AddRegularWall(lP0, lP1, textureAtlasId, bitmap->GetWidth(), bitmap->GetHeight(), maxHeight);
    }
}

void GLLevelLoader::AddRegularWall(MR_3DCoordinate lP0, MR_3DCoordinate lP1, int textureAtlasId, float bitmapWidth, float bitmapHeight, std::optional<int> maxHeight)
{
    float dx = lP1.mX - lP0.mX;
    float dy = lP1.mY - lP0.mY;
    auto wallLength = sqrt(dx * dx + dy * dy);
    int wallHeight = lP0.mZ - lP1.mZ;

    float u0 = 0.0f;
    float u1 = 1.0f;
    float v0 = 1.0f;
    float v1 = 0.0f;

    if (maxHeight.has_value())
    {
        if (wallHeight > 0)
        {
            u1 = wallLength / wallHeight; // repeat width every wall height

            int lDivisor = 1 + (wallHeight - 1) / maxHeight.value();
            if (lDivisor > 1)
            {
                v0 = v0 * lDivisor;
                u1 = u1 * lDivisor;
            }
        }
    }
    else
    {
        u1 = wallLength / bitmapWidth; // tile horizontally

        v0 = 1.0f;
        v1 = 1.0f - (float)wallHeight / bitmapHeight;
    }

    wallVerts.vertices.push_back(
        SwapYZ(makeWallVertex(lP0.mX, lP0.mY, lP0.mZ, u0, v0, textureAtlasId)));
    wallVerts.vertices.push_back(
        SwapYZ(makeWallVertex(lP0.mX, lP0.mY, lP1.mZ, u0, v1, textureAtlasId)));
    wallVerts.vertices.push_back(
        SwapYZ(makeWallVertex(lP1.mX, lP1.mY, lP0.mZ, u1, v0, textureAtlasId)));
    wallVerts.vertices.push_back(
        SwapYZ(makeWallVertex(lP1.mX, lP1.mY, lP1.mZ, u1, v1, textureAtlasId)));
    uint16_t latestVertexIdx = wallVerts.vertices.size() - 1;

    wallVerts.indices.push_back(latestVertexIdx - 3);
    wallVerts.indices.push_back(latestVertexIdx - 1);
    wallVerts.indices.push_back(latestVertexIdx - 2);

    wallVerts.indices.push_back(latestVertexIdx - 2);
    wallVerts.indices.push_back(latestVertexIdx - 1);
    wallVerts.indices.push_back(latestVertexIdx);
}

void GLLevelLoader::AddAnimatedWall(MR_3DCoordinate lP0, MR_3DCoordinate lP1, int textureAtlasId, float bitmapWidth, float bitmapHeight, std::optional<int> maxHeight, int rotationSpeed, int rotationLength)
{
    float dx = lP1.mX - lP0.mX;
    float dy = lP1.mY - lP0.mY;
    auto wallLength = sqrt(dx * dx + dy * dy);
    int wallHeight = lP0.mZ - lP1.mZ;

    if (maxHeight.has_value() && wallHeight > 0)
    {
        bitmapHeight = wallHeight;
        bitmapWidth = wallHeight;
    }

    // Calculate direction vector for the wall
    float dirX = dx / wallLength;
    float dirY = dy / wallLength;

    // Calculate how many full segments fit in the wall length
    int numFullSegments = floor(wallLength / bitmapWidth);
    float remainingLength = wallLength - (numFullSegments * bitmapWidth);

    // Total number of segments (full + possible partial)
    int numSegments = remainingLength > 0 ? (numFullSegments + 1) : numFullSegments;

    for (int i = 0; i < numSegments; i++) {
        float startPos = i * bitmapWidth;
        float segmentWidth;

        // For the last segment, use the remaining length
        if (i == numFullSegments && remainingLength > 0) {
            segmentWidth = remainingLength;
        } else {
            segmentWidth = bitmapWidth;
        }

        float endPos = startPos + segmentWidth;

        MR_3DCoordinate segP0, segP1;
        segP0.mX = lP0.mX + dirX * startPos;
        segP0.mY = lP0.mY + dirY * startPos;
        segP0.mZ = lP0.mZ;

        segP1.mX = lP0.mX + dirX * endPos;
        segP1.mY = lP0.mY + dirY * endPos;
        segP1.mZ = lP0.mZ;

        float u0 = 0.0f;
        float u1;

        // If this is the last segment, and it's partial, adjust u1 to match the proportion
        if (i == numFullSegments && remainingLength > 0) {
            u1 = remainingLength / bitmapWidth; // Partial texture width
        } else {
            u1 = 1.0f; // Full texture width
        }
        // - the code implemented in non-animated wall to adjust height to maxHeight may be needed here
        float v0 = 1.0f;
        float v1 = 0.0f;

        wallVerts.vertices.push_back(
            SwapYZ(makeWallVertex(segP0.mX, segP0.mY, lP0.mZ, u0, v0, textureAtlasId, rotationSpeed,  rotationLength, i)));
        wallVerts.vertices.push_back(
            SwapYZ(makeWallVertex(segP0.mX, segP0.mY, lP1.mZ, u0, v1, textureAtlasId, rotationSpeed,  rotationLength, i)));
        wallVerts.vertices.push_back(
            SwapYZ(makeWallVertex(segP1.mX, segP1.mY, lP0.mZ, u1, v0, textureAtlasId, rotationSpeed,  rotationLength, i)));
        wallVerts.vertices.push_back(
            SwapYZ(makeWallVertex(segP1.mX, segP1.mY, lP1.mZ, u1, v1, textureAtlasId, rotationSpeed, rotationLength,  i)));

        uint16_t latestVertexIdx = wallVerts.vertices.size() - 1;

        // Add indices for the two triangles of this quad
        wallVerts.indices.push_back(latestVertexIdx - 3);
        wallVerts.indices.push_back(latestVertexIdx - 1);
        wallVerts.indices.push_back(latestVertexIdx - 2);

        wallVerts.indices.push_back(latestVertexIdx - 2);
        wallVerts.indices.push_back(latestVertexIdx - 1);
        wallVerts.indices.push_back(latestVertexIdx);
    }
}

void GLLevelLoader::LoadRoomFreeElements(const MR_Level* level, int roomId)
{
    MR_FreeElementHandle lHandle = level->GetFirstFreeElement(roomId);
    while(lHandle != nullptr)
    {
        MR_FreeElement* lElement = MR_Level::GetFreeElement(lHandle);
        auto freeElementBase = dynamic_cast<MR_FreeElementBase*>(lElement);
        if (freeElementBase != nullptr)
        {
            auto actor = freeElementBase->GetActor();
            auto patches = actor->GetActorPatches();
            for (auto patch : patches)
            {
                int lCounter;
                int lURes = patch->GetURes();
                int lVRes = patch->GetVRes();
                int lNbNodes = lURes*lVRes;
                const MR_3DCoordinate* lNodeList = patch->GetNodeList();
                for(lCounter = 0; lCounter < lNbNodes; lCounter++)
                {
                    auto node = lNodeList[lCounter];
                    freeElementVerts.vertices.push_back(makeVertexWithTextureId(node.mX, node.mY, node.mZ, 0.0f, 0.0f, 0));
                }
                freeElementVerts.indices.push_back(0);
                std::cout << "patch node count " << lNbNodes << std::endl;
            }
        }
        lHandle = MR_Level::GetNextFreeElement( lHandle );
    }
}
