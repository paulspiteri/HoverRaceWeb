#pragma once

#include "Camera.h"
#include "GLRenderer.h"

class GLViewport
{
    int sizeX, sizeY;
    Camera camera;
    Camera bkg_camera;
    Camera map_camera;
    GLRenderer* glRenderer{};

public:
    GLViewport();
    void Setup(GLRenderer* pGlRenderer, int pSizeX, int pSizeY);
    void SetCameraPosition(const MR_3DCoordinate& pPosition, MR_Angle pOrientation);
    void SetMapSize(const glm::ivec4& size);
    void SetSimulationTime(const MR_SimulationTime pTime);
};
