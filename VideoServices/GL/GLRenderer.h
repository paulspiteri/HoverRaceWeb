#pragma once
#include "Camera.h"
#include "SokolState.h"
#include "SDL3/SDL.h"

struct Vertex
{
    glm::i32vec3 position;
    float color[4];
    glm::vec2 texcoord;
};

struct VerticesData {
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;
};

inline Vertex makeVertex(
    int32_t x, int32_t y, int32_t z,
    float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f,
    float u = 0.0f, float v = 0.0f)
{
    return Vertex{
        .position = glm::i32vec3(x, y, z),
        .color = {r, g, b, a},
        .texcoord = glm::vec2(u, v)
    };
}

inline glm::i32vec3 SwapYZ(glm::i32vec3 vec)
{
    // negative z means ahead, which was the intent from the old coordinate system
    return glm::i32vec3(vec.x, vec.z, -vec.y);
}

inline Vertex SwapYZ(Vertex vertex)
{
    vertex.position = SwapYZ(vertex.position);
    return vertex;
}


class GLRenderer
{
public:
    GLRenderer(SDL_Window* glWindow, SDL_GLContext glContext);
    ~GLRenderer();
    Sokol_State state;
    SDL_Window* glWindow;
    SDL_GLContext glContext;

    void SetVertices(const VerticesData& vertices);
    void Render() const;
};
