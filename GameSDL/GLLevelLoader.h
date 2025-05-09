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
    void LoadRoom(const MR_Level* pLevel, int pRoomId, const MR_PolygonShape* sectionShape,
                    VerticesData& verts) const;
    void LoadRoomFloor(const MR_PolygonShape* sectionShape, VerticesData& verts, MR_SurfaceElement* floorTexture) const;
    void LoadRoomCeiling(const MR_PolygonShape* roomShape, VerticesData& verts) const;
    void AddWallVertices(VerticesData& verts, MR_3DCoordinate lP0, MR_3DCoordinate lP1) const;

    GLRenderer* glRenderer;
};
