#pragma once
#include <vector>

#include "Camera.h"
#include "SokolState.h"
#include "SDL3/SDL.h"
#include "../../ObjFacTools/ResBitmap.h"
#include "../VideoBuffer.h"

struct Vertex
{
    glm::i32vec3 position;
    glm::vec2 texcoord;
};

struct VertexWithTextureId
{
    Vertex vertex;
    int textureIdx;
};

struct WallVertex
{
    VertexWithTextureId vertex;
    int rotationSpeed;
    int rotationLength;
    int segment;
};

template <typename T>
struct VerticesData
{
    std::vector<T> vertices;
    std::vector<uint16_t> indices;
};

inline Vertex makeVertex(int32_t x, int32_t y, int32_t z, float u, float v)
{
    return Vertex{
        .position = glm::i32vec3(x, y, z),
        .texcoord = glm::vec2(u, v),
    };
}

inline VertexWithTextureId makeVertexWithTextureId(int32_t x, int32_t y, int32_t z, float u, float v,
                                                   int textureIdx)
{
    return VertexWithTextureId{
        .vertex = makeVertex(x, y, z, u, v),
        .textureIdx = textureIdx
    };
}

inline WallVertex makeWallVertex(int32_t x, int32_t y, int32_t z, float u, float v,
                                                   uint32_t textureIdx, int rotationSpeed = 0, int rotationLength = 0, int segment = 0)
{
    return WallVertex {
        .vertex = makeVertexWithTextureId(x, y, z, u, v, textureIdx),
        .rotationSpeed = rotationSpeed,
        .rotationLength = rotationLength,
        .segment = segment
    };
}

inline glm::i32vec3 SwapYZ(glm::i32vec3 vec)
{
    // -z means ahead, which was +y in the old coordinate system
    return glm::i32vec3(vec.x, vec.z, -vec.y);
}

inline Vertex SwapYZ(Vertex vertex)
{
    vertex.position = SwapYZ(vertex.position);
    return vertex;
}

inline VertexWithTextureId SwapYZ(VertexWithTextureId vertex)
{
    vertex.vertex = SwapYZ(vertex.vertex);
    return vertex;
}

inline WallVertex SwapYZ(WallVertex vertex)
{
    vertex.vertex = SwapYZ(vertex.vertex);
    return vertex;
}

struct AtlasCoords
{
    float u1, v1; // Top-left
    float u2, v2; // Bottom-right
};

struct TextureData
{
    MR_UInt32 id;   // high bit for 'b' animation frame

    sg_image img;
    //   sg_sampler sampler;
    int width;
    int height;
    AtlasCoords atlas_coords;
    uint32_t* pixels;
};


class GLRenderer
{
    std::vector<TextureData> textures;
    MR_VideoBuffer* videoBuffer;

    uint32_t* ConvertTextureToRGBA8(const MR_ResBitmap* bitmap);
    uint32_t* ConvertBackgroundToRGBA8(const MR_UInt8* backImage);

public:
    GLRenderer(SDL_Window* glWindow, SDL_GLContext glContext, MR_VideoBuffer* videoBuffer);
    ~GLRenderer();
    Sokol_State state{};
    SDL_Window* glWindow;
    SDL_GLContext glContext;

    void BindWorldTextures();
    void BindWorldVertices(const VerticesData<VertexWithTextureId>& vertices);
    void BindWallVertices(const VerticesData<WallVertex>& vertices);
    void BindFreeElementVertices(const VerticesData<VertexWithTextureId>& vertices);
    unsigned long LoadTexture(MR_UInt32 id, const MR_ResBitmap* bitmap);
    void BindBackgroundVertices(const VerticesData<Vertex>& vertices);
    void BindBackgroundTexture(const MR_UInt8* backImage);
    void Render() const;
};
