#pragma once

#include "../VideoServices/GL/GLRenderer.h"
#include "../Model/Level.h"

class GLLevelLoader
{
public:
    GLLevelLoader(GLRenderer* glRenderer);
    void LoadLevel(const MR_Level* level, const MR_UInt8* backImage);

private:
    void LoadBackground(const MR_Level* level, const MR_UInt8* backImage);
    void LoadRoomWalls(const MR_Level* level, int roomId);
    void LoadRoomFloor(const MR_Level* level, int roomId);
    void LoadRoomCeiling(const MR_Level* level, int roomId);
    void AddWallVertices(MR_3DCoordinate lP0, MR_3DCoordinate lP1, MR_SurfaceElement* surfaceElement);

    VerticesData verts;
    VerticesData bkgVerts;
    GLRenderer* glRenderer;
};
