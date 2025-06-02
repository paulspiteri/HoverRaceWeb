#pragma once

#include <optional>
#include <unordered_map>

#include "../VideoServices/GL/GLRenderer.h"
#include "../Model/Level.h"

class GLLevelLoader
{
public:
    explicit GLLevelLoader(GLRenderer* glRenderer);
    void LoadLevel(const MR_Level* level, const MR_UInt8* backImage);
    std::unordered_map<int, std::vector<FreeElementInstance>> GetFreeElementInstances(const MR_Level* level);

private:
    void LoadBackground(const MR_UInt8* backImage);
    void LoadRoomWalls(const MR_Level* level, int roomId);
    void LoadRoomFloor(const MR_Level* level, int roomId);
    void LoadRoomCeiling(const MR_Level* level, int roomId);
    void LoadFeatureWalls(const MR_Level* level, int featureId);
    void LoadFeatureFloor(const MR_Level* level, int featureId);
    void LoadFeatureCeiling(const MR_Level* level, int featureId);
    void LoadFloor(MR_PolygonShape* shape, MR_SurfaceElement* surfaceElement, bool upsideDown = false);
    void LoadWater(MR_PolygonShape* shape, MR_SurfaceElement* surfaceElement, MR_Int32 waterLevel);
    void LoadCeiling(MR_PolygonShape* shape, MR_SurfaceElement* surfaceElement, bool upsideDown = false);
    void AddWall(MR_3DCoordinate lP0, MR_3DCoordinate lP1, MR_SurfaceElement* surfaceElement);
    void AddRegularWall(MR_3DCoordinate lP0, MR_3DCoordinate lP1, int textureAtlasId, float bitmapWidth, float bitmapHeight, std::optional<int> maxHeight);
    void AddAnimatedWall(MR_3DCoordinate lP0, MR_3DCoordinate lP1, int textureAtlasId, float bitmapWidth, float bitmapHeight, std::optional<int> maxHeight, int rotationSpeed, int rotationLength);
    std::unordered_map<int, VerticesData<FreeElementVertex>> LoadGameFreeElements();
    void LoadGameSprites();

    VerticesData<Vertex> bkgVerts;
    VerticesData<VertexWithTextureId> worldVerts;
    VerticesData<VertexWithTextureId> waterVerts;
    VerticesData<WallVertex> wallVerts;
    GLRenderer* glRenderer;
};
