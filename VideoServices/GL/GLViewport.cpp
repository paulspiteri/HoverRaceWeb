#include "GLViewport.h"
#include <cstring>

GLViewport::GLViewport(): sizeX(0), sizeY(0), camera(74.0f), bkg_camera(74.0f), map_camera(90.0f)
{
    map_camera.setFront(glm::vec3(0.0f, -1.0f, 0.0f));
    map_camera.setUp(glm::vec3(0.0f, 0.0f, -1.0f));
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

    float orientationYaw = MR_ANGLE_TO_RADIANS(pOrientation);
    orientationYaw = -orientationYaw; // requires negation due to coordinate system mapping, and depth being negative in GL
    camera.rotate(orientationYaw);
    bkg_camera.rotate(orientationYaw);

    float aspect = static_cast<float>(sizeX) / static_cast<float>(sizeY);
    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 projection = camera.getProjectionMatrix(aspect);
    std::memcpy(glRenderer->state.world_uniforms.view, &view, sizeof(view));
    std::memcpy(glRenderer->state.world_uniforms.proj, &projection, sizeof(projection));
    std::memcpy(glRenderer->state.water_uniforms.view, &view, sizeof(view));
    std::memcpy(glRenderer->state.water_uniforms.proj, &projection, sizeof(projection));
    std::memcpy(glRenderer->state.wall_uniforms.view, &view, sizeof(view));
    std::memcpy(glRenderer->state.wall_uniforms.proj, &projection, sizeof(projection));
    std::memcpy(glRenderer->state.free_element_uniforms.view, &view, sizeof(view));
    std::memcpy(glRenderer->state.free_element_uniforms.proj, &projection, sizeof(projection));


    glm::mat4 bkg_view = bkg_camera.getViewMatrix();
    glm::mat4 bkg_projection = bkg_camera.getProjectionMatrix(aspect);
    std::memcpy(glRenderer->state.bkg_uniforms.view, &bkg_view, sizeof(view));
    std::memcpy(glRenderer->state.bkg_uniforms.proj, &bkg_projection, sizeof(projection));
}

void GLViewport::SetMapSize(const glm::ivec4& size)
{
    int centerX = sizeX + (size.x + size.z) / 2;
    int centerY = sizeY + (size.y + size.w) / 2;
    // int width = size.z - size.x;
    // int height = size.w - size.y;
    // int z = std::max(width, height) / 1.5f;
    map_camera.setPosition(SwapYZ(glm::vec3(centerX, centerY, 200000)));

    glm::mat4 map_view = map_camera.getViewMatrix();
    glm::mat4 map_projection = map_camera.getProjectionMatrix(1.0f);
    std::memcpy(glRenderer->state.world_minimap_uniforms.view, &map_view, sizeof(map_view));
    std::memcpy(glRenderer->state.world_minimap_uniforms.proj, &map_projection, sizeof(map_projection));
}

void GLViewport::SetSimulationTime(const MR_SimulationTime pTime)
{
    glRenderer->state.wall_uniforms.time = pTime;
    glRenderer->state.free_element_uniforms.time = pTime;
}
