#pragma once

#include "../VideoServices/GL/GLRenderer.h"
#include "../Model/Shapes.h"
#include "../Model/Level.h"

class GLLevelLoader
{
public:
    GLLevelLoader(GLRenderer* glRenderer);
    void LoadLevel(const MR_Level* level);

private:
    void LoadRoomWalls(const MR_Level* level, int roomId, VerticesData& verts) const;
    void LoadRoomFloor(const MR_Level* level, int roomId, VerticesData& verts) const;
    void LoadRoomCeiling(const MR_Level* level, int roomId, VerticesData& verts) const;
    void AddWallVertices(VerticesData& verts, MR_3DCoordinate lP0, MR_3DCoordinate lP1, MR_SurfaceElement* surfaceElement) const;

    GLRenderer* glRenderer;
};
