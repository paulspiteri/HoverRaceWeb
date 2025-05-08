#include "GLViewport.h"
#include <cstring>

GLViewport::GLViewport(): sizeX(0), sizeY(0), camera(90)
{
}

void GLViewport::Setup(GLRenderer* pGlRenderer, int pSizeX, int pSizeY)
{
    glRenderer = pGlRenderer;
    sizeX = pSizeX;
    sizeY = pSizeY;
}

void GLViewport::SetCameraPosition(const MR_3DCoordinate& pPosition, MR_Angle pOrientation)
{
    camera.setPosition(SwapYZ(glm::vec3(pPosition.mX, pPosition.mY, pPosition.mZ)));

    float orientationYaw = MR_ANGLE_TO_DEGREES(pOrientation);
    orientationYaw = -orientationYaw; // requires negation due to coordinate system mapping, and depth being negative in GL
    camera.rotate(orientationYaw);

    float aspect = static_cast<float>(sizeX) / static_cast<float>(sizeY);
    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 projection = camera.getProjectionMatrix(aspect);
    std::memcpy(glRenderer->state.uniforms.view, &view, sizeof(view));
    std::memcpy(glRenderer->state.uniforms.proj, &projection, sizeof(projection));
}
