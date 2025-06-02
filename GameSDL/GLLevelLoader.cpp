#include "GLLevelLoader.h"
#include <numbers>
#include "../ObjFacTools/ResBitmap.h"
#include "../ObjFacTools/BitmapSurface.h"
#include "../ObjFacTools/FreeElementBase.h"
#include "../ObjFacTools/ObjectFactoryData.h"
#include "../ObjFac1/ObjFac1Res.h"
#include "../ObjFac1/HoverRender.h"
#include "../MainCharacter/MainCharacter.h"

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
    glRenderer->BindWaterVertices(waterVerts);
    glRenderer->BindWallVertices(wallVerts);
    glRenderer->BindWorldTextures();
    glRenderer->BindFreeElementVertices(LoadGameFreeElements());
    glRenderer->BindFreeElementTextures();
    LoadGameSprites();
    glRenderer->BindSpriteTextures();
}

std::unordered_map<int, std::vector<FreeElementInstance>> GLLevelLoader::GetFreeElementInstances(
    const MR_Level* level)
{
    std::unordered_map<int, std::vector<FreeElementInstance>> freeElementInstances;
    int totalRooms = level->GetRoomCount();
    for (int roomId = 0; roomId < totalRooms; roomId++)
    {
        MR_FreeElementHandle lHandle = level->GetFirstFreeElement(roomId);
        while (lHandle != nullptr)
        {
            MR_FreeElement* lElement = MR_Level::GetFreeElement(lHandle);
            auto actorPosition = lElement->mPosition;
            auto position = SwapYZ(glm::ivec3(actorPosition.mX, actorPosition.mY, actorPosition.mZ));
            const MR_ResActor* actor = nullptr;
            int type = 0, orientation = 0, sequence = 0, frame = 0;
            if (lElement->mId.mDllId == MR_MAIN_CHARACTER_DLL_ID && lElement->mId.mClassId ==
                MR_MAIN_CHARACTER_CLASS_ID)
            {
                auto mainCharacter = static_cast<MR_MainCharacter*>(lElement);
                auto mainCharacterRenderer = mainCharacter->GetRenderer();
                auto hoverRenderer = dynamic_cast<MR_HoverRender*>(mainCharacterRenderer);
                if (hoverRenderer == nullptr)
                {
                    throw std::runtime_error("MainCharacter has unknown renderer");
                }
                auto hoverModelId = mainCharacter->GetHoverModel();
                actor = hoverRenderer->GetActor(hoverModelId);
                type = actor->GetResourceId();
                orientation = mainCharacter->GetCabinOrientation();
                bool isMotorOn = mainCharacter->GetMotorDisplay() > 0;
                sequence = isMotorOn ? 1 : 0;
                if (isMotorOn)
                {
                    frame = hoverRenderer->GetFrame(); // note - is dependent on legacy renderer running
                }
            }
            else
            {
                auto freeElementBase = dynamic_cast<MR_FreeElementBase*>(lElement);
                if (freeElementBase != nullptr)
                {
                    actor = freeElementBase->GetActor();
                    type = actor->GetResourceId();
                    orientation = type == MR_PWRUP ? 0 : lElement->mOrientation;
                    // orientation for powerups implemented in the shader to avoid unnecessary vertex updates
                    sequence = freeElementBase->GetCurrentSequence();
                    frame = freeElementBase->GetCurrentFrame();
                }
            }
            if (actor != nullptr)
            {
                FreeElementInstance instance = {
                    .position = position, .type = type, .orientation = orientation, .sequence = sequence, .frame = frame
                };
                freeElementInstances[type].push_back(instance);
            }

            lHandle = MR_Level::GetNextFreeElement(lHandle);
        }
    }
    return freeElementInstances;
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
        if (waterLevel > roomShape->ZMin())
        {
            waterLevel -= (waterLevel - roomShape->ZMin()) / 4;
            LoadWater(roomShape.get(), surfaceElement, waterLevel);
        }
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
    lP1.mX = featureShape->X(0);
    lP1.mY = featureShape->Y(0);
    lP1.mZ = featureShape->ZMin();

    for (int lVertex = 0; lVertex < lVertexCount; lVertex++)
    {
        int lNext = lVertex + 1;
        if (lNext == lVertexCount)
        {
            lNext = 0;
        }
        lP0.mX = featureShape->X(lNext);
        lP0.mY = featureShape->Y(lNext);

        MR_SurfaceElement* surfaceElement = level->GetFeatureWallElement(featureId, lVertex);
        if (surfaceElement != nullptr)
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

void GLLevelLoader::LoadWater(MR_PolygonShape* shape, MR_SurfaceElement* surfaceElement, MR_Int32 waterLevel)
{
    auto bitmap = surfaceElement->GetResBitmap();
    auto lNbVertex = shape->VertexCount();
    auto baseIndex = waterVerts.vertices.size();
    int textureId = surfaceElement->mId.mClassId | 0x40000000; // turn on high bit for alpha copy
    auto textureAtlasId = glRenderer->LoadTexture(textureId, bitmap, 0x80);

    for (auto i = 0; i < lNbVertex; i++)
    {
        float u = shape->X(i) / static_cast<float>(bitmap->GetWidth());
        float v = shape->Y(i) / static_cast<float>(bitmap->GetHeight());
        waterVerts.vertices.push_back(
            SwapYZ(makeVertexWithTextureId(shape->X(i), shape->Y(i), waterLevel, u, v, textureAtlasId)));
    }

    // Triangulate using fan
    for (auto j = 1; j < lNbVertex - 1; ++j)
    {
        waterVerts.indices.push_back(baseIndex);
        waterVerts.indices.push_back(baseIndex + j + 0);
        waterVerts.indices.push_back(baseIndex + j + 1);
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
        MR_UInt32 id2 = surfaceElement->mId.mClassId | 0x80000000; // turn on high bit for bitmap2
        glRenderer->LoadTexture(id2, bitmap2);

        int rotationSpeed = 0, rotationLength = 0;
        auto bitmapSurface = dynamic_cast<MR_BitmapSurface*>(surfaceElement);
        if (bitmapSurface != nullptr)
        {
            rotationSpeed = bitmapSurface->GetRotationSpeed();
            rotationLength = bitmapSurface->GetRotationLen();
        }
        AddAnimatedWall(lP0, lP1, textureAtlasId, bitmap->GetWidth(), bitmap->GetHeight(), maxHeight, rotationSpeed,
                        rotationLength);
    }
    else
    {
        AddRegularWall(lP0, lP1, textureAtlasId, bitmap->GetWidth(), bitmap->GetHeight(), maxHeight);
    }
}

void GLLevelLoader::AddRegularWall(MR_3DCoordinate lP0, MR_3DCoordinate lP1, int textureAtlasId, float bitmapWidth,
                                   float bitmapHeight, std::optional<int> maxHeight)
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

void GLLevelLoader::AddAnimatedWall(MR_3DCoordinate lP0, MR_3DCoordinate lP1, int textureAtlasId, float bitmapWidth,
                                    float bitmapHeight, std::optional<int> maxHeight, int rotationSpeed,
                                    int rotationLength)
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

    for (int i = 0; i < numSegments; i++)
    {
        float startPos = i * bitmapWidth;
        float segmentWidth;

        // For the last segment, use the remaining length
        if (i == numFullSegments && remainingLength > 0)
        {
            segmentWidth = remainingLength;
        }
        else
        {
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
        if (i == numFullSegments && remainingLength > 0)
        {
            u1 = remainingLength / bitmapWidth; // Partial texture width
        }
        else
        {
            u1 = 1.0f; // Full texture width
        }
        // - the code implemented in non-animated wall to adjust height to maxHeight may be needed here
        float v0 = 1.0f;
        float v1 = 0.0f;

        wallVerts.vertices.push_back(
            SwapYZ(makeWallVertex(segP0.mX, segP0.mY, lP0.mZ, u0, v0, textureAtlasId, rotationSpeed, rotationLength,
                                  i)));
        wallVerts.vertices.push_back(
            SwapYZ(makeWallVertex(segP0.mX, segP0.mY, lP1.mZ, u0, v1, textureAtlasId, rotationSpeed, rotationLength,
                                  i)));
        wallVerts.vertices.push_back(
            SwapYZ(makeWallVertex(segP1.mX, segP1.mY, lP0.mZ, u1, v0, textureAtlasId, rotationSpeed, rotationLength,
                                  i)));
        wallVerts.vertices.push_back(
            SwapYZ(makeWallVertex(segP1.mX, segP1.mY, lP1.mZ, u1, v1, textureAtlasId, rotationSpeed, rotationLength,
                                  i)));

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

std::unordered_map<int, VerticesData<FreeElementVertex>> GLLevelLoader::LoadGameFreeElements()
{
    std::unordered_map<int, VerticesData<FreeElementVertex>> result;
    auto electroCar = gObjectFactoryData->mResourceLib.GetActor(MR_ELECTRO_CAR);
    auto hitechCar = gObjectFactoryData->mResourceLib.GetActor(MR_HITECH_CAR);
    auto biturboCar = gObjectFactoryData->mResourceLib.GetActor(MR_BITURBO_CAR);
    auto powerUp = gObjectFactoryData->mResourceLib.GetActor(MR_PWRUP);
    auto mine = gObjectFactoryData->mResourceLib.GetActor(MR_MINE);
    auto bumperGate = gObjectFactoryData->mResourceLib.GetActor(MR_BUMPERGATE);
    auto missile = gObjectFactoryData->mResourceLib.GetActor(MR_MISSILE);
    std::array actors = {electroCar, hitechCar, biturboCar, powerUp, mine, bumperGate, missile};

    for (auto actor : actors)
    {
        VerticesData<FreeElementVertex> verts;
        for (int seqIdx = 0; seqIdx < actor->GetSequenceCount(); seqIdx++)
        {
            for (int frameIdx = 0; frameIdx < actor->GetFrameCount(seqIdx); frameIdx++)
            {
                auto patches = actor->GetPatches(seqIdx, frameIdx);
                for (auto patch : patches)
                {
                    int lBitmapXRes = patch->mBitmap->GetMaxXRes();
                    int lBitmapYRes = patch->mBitmap->GetMaxYRes();
                    int lURes = patch->GetURes();
                    int lVRes = patch->GetVRes();
                    float lBitmapRowInc = static_cast<float>(lBitmapXRes) / static_cast<float>(lVRes - 1);
                    float lBitmapColInc = static_cast<float>(lBitmapYRes) / static_cast<float>(lURes - 1);
                    const MR_3DCoordinate* lNodeList = patch->GetNodeList();
                    int textureId = glRenderer->LoadFreeElementTexture(patch->mBitmap->GetResourceId(), patch->mBitmap);
                    uint16_t startVertexIdx = verts.vertices.size();
                    for (int lV = 0; lV < lVRes; lV++)
                    {
                        float v = (lV * lBitmapRowInc) / lBitmapYRes;

                        for (int lU = 0; lU < lURes; lU++)
                        {
                            float u = (lU * lBitmapColInc) / lBitmapXRes;
                            int index = lV * lURes + lU;
                            auto node = lNodeList[index];

                            auto freeElementVertex = FreeElementVertex{
                                .vertex = SwapYZ(
                                    makeVertexWithTextureId(node.mX, node.mY, node.mZ, u, 1 - v, textureId)),
                                .sequence = seqIdx,
                                .frame = frameIdx
                            };
                            verts.vertices.push_back(freeElementVertex);
                        }
                    }

                    for (int lV = 0; lV < lVRes - 1; lV++)
                    {
                        for (int lU = 0; lU < lURes - 1; lU++)
                        {
                            // Calculate indices for the four corners of the current grid cell
                            // Offset by startVertexIdx to account for existing vertices
                            uint16_t bottomLeft = startVertexIdx + lV * lURes + lU;
                            uint16_t bottomRight = bottomLeft + 1;
                            uint16_t topLeft = bottomLeft + lURes;
                            uint16_t topRight = topLeft + 1;

                            verts.indices.push_back(bottomLeft);
                            verts.indices.push_back(bottomRight);
                            verts.indices.push_back(topLeft);

                            verts.indices.push_back(bottomRight);
                            verts.indices.push_back(topRight);
                            verts.indices.push_back(topLeft);
                        }
                    }
                }
            }
        }
        result[actor->GetResourceId()] = verts;
    }
    return result;
}

void GLLevelLoader::LoadGameSprites()
{
    const MR_ResSprite* missileSprite = gObjectFactoryData->mResourceLib.GetSprite(MR_MISSILE_STAT);
    const MR_ResSprite* mineSprite = gObjectFactoryData->mResourceLib.GetSprite(MR_MINE_STAT);
    const MR_ResSprite* powerUpSprite = gObjectFactoryData->mResourceLib.GetSprite(MR_PWRUP_STAT);
    std::array sprites = { missileSprite, mineSprite, powerUpSprite };

    for (auto sprite : sprites)
    {
        glRenderer->LoadSprite(sprite->GetResourceId(), sprite);
    }
}
