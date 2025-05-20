#pragma once

#include <optional>

#include "../VideoServices/GL/GLRenderer.h"
#include "../Model/Level.h"

class GLLevelLoader
{
public:
    GLLevelLoader(GLRenderer* glRenderer);
    void LoadLevel(const MR_Level* level, const MR_UInt8* backImage);

private:
    void LoadBackground(const MR_UInt8* backImage);
    void LoadRoomWalls(const MR_Level* level, int roomId);
    void LoadRoomFloor(const MR_Level* level, int roomId);
    void LoadRoomCeiling(const MR_Level* level, int roomId);
    void LoadFeatureWalls(const MR_Level* level, int featureId);
    void LoadFeatureFloor(const MR_Level* level, int featureId);
    void LoadFeatureCeiling(const MR_Level* level, int featureId);
    void LoadFloor(MR_PolygonShape* shape, MR_SurfaceElement* surfaceElement, bool upsideDown = false);
    void LoadCeiling(MR_PolygonShape* shape, MR_SurfaceElement* surfaceElement, bool upsideDown = false);
    void AddWall(MR_3DCoordinate lP0, MR_3DCoordinate lP1, MR_SurfaceElement* surfaceElement);
    void AddRegularWall(MR_3DCoordinate lP0, MR_3DCoordinate lP1, int textureAtlasId, float bitmapWidth, float bitmapHeight, std::optional<int> maxHeight);
    void AddAnimatedWall(MR_3DCoordinate lP0, MR_3DCoordinate lP1, int textureAtlasId, float bitmapWidth, float bitmapHeight, std::optional<int> maxHeight, int rotationSpeed, int rotationLength);

    VerticesData<Vertex> bkgVerts;
    VerticesData<VertexWithTextureId> worldVerts;
    VerticesData<WallVertex> wallVerts;

    GLRenderer* glRenderer;
};
