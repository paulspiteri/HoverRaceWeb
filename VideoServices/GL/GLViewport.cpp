#include "GLViewport.h"
#include <cstring>

inline float MR_ANGLE_TO_DEGREES(MR_Angle angle)
{
    float degrees = (angle * 360.0f) / (float)MR_2PI;
    return degrees;
}

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
    camera.setPosition(glm::vec3(pPosition.mX, pPosition.mZ, pPosition.mY));

    float orientationYaw = MR_ANGLE_TO_DEGREES(pOrientation);
    camera.rotate(orientationYaw);

    float aspect = static_cast<float>(sizeX) / static_cast<float>(sizeY);
    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 projection = camera.getProjectionMatrix(aspect);
    std::memcpy(glRenderer->state.uniforms.view, &view, sizeof(view));
    std::memcpy(glRenderer->state.uniforms.proj, &projection, sizeof(projection));
}

void GLViewport::SetWallVertices(const std::vector<Vertex>& vertices, const std::vector<uint16_t>& vertexIdxs) const
{
    static bool firstTime = true;
    if (!firstTime)
    {
        return;
    }
    sg_buffer_desc buf_desc = {
        .type = SG_BUFFERTYPE_VERTEXBUFFER,
        .usage = SG_USAGE_IMMUTABLE,
        .data = make_sg_range(vertices),
        .label = "wall-vertices"
    };

    glRenderer->state.bind.vertex_buffers[0] = sg_make_buffer(&buf_desc);

    sg_buffer_desc index_buf_desc = {
        .type = SG_BUFFERTYPE_INDEXBUFFER,
        .data = make_sg_range(vertexIdxs),
        .label = "wall-indices"
    };
    glRenderer->state.bind.index_buffer = sg_make_buffer(&index_buf_desc);

    glRenderer->state.wallVertexCount = static_cast<uint32_t>(vertices.size());
    firstTime = false;
}
