#pragma once
#include <unordered_map>

#include "Camera.h"
#include "SokolState.h"
#include "SDL3/SDL.h"
#include "../../ObjFacTools/ResBitmap.h"
#include "../ColorPaletteEntry.h"


struct Vertex
{
    glm::i32vec3 position;
    float color[4];
    glm::vec2 texcoord;
    uint32_t textureIdx;
};

struct VerticesData {
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;
};

inline Vertex makeVertex(
    int32_t x, int32_t y, int32_t z,
    float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f,
    float u = 0.0f, float v = 0.0f, uint32_t tex_idx = 0)
{
    return Vertex{
        .position = glm::i32vec3(x, y, z),
        .color = {r, g, b, a},
        .texcoord = glm::vec2(u, v),
        .textureIdx = tex_idx
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

struct TextureData {
    sg_image img;
 //   sg_sampler sampler;
    int width;
    int height;
};


class GLRenderer
{
    std::unordered_map<MR_UInt16, TextureData> textures;
    NoMFC::PALETTEENTRY* colorPalette;

private:
    uint32_t* ConvertTextureToRGBA8(const MR_ResBitmap* bitmap);

public:
    GLRenderer(SDL_Window* glWindow, SDL_GLContext glContext, NoMFC::PALETTEENTRY* colorPalette);
    ~GLRenderer();
    Sokol_State state;
    SDL_Window* glWindow;
    SDL_GLContext glContext;

    void BindTextures();
    void BindVertices(const VerticesData& vertices);
    void LoadTexture(MR_UInt16 id, const MR_ResBitmap* bitmap);
    void Render() const;
};
