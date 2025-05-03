#pragma once
#include "Camera.h"
#include "GLRenderer.h"
#include "../../Util/WorldCoordinates.h"

class GLViewport
{
    int sizeX, sizeY;
    Camera camera;
    GLRenderer* glRenderer{};

public:
    GLViewport();
    void Setup(GLRenderer* pGlRenderer, int pSizeX, int pSizeY);
    void SetCameraPosition(const MR_3DCoordinate& pPosition, MR_Angle pOrientation);
};
