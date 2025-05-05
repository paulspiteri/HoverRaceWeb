#pragma once
#include "Camera.h"
#include "SokolState.h"
#include "SDL3/SDL.h"

struct Vertex
{
    glm::i32vec3 position;
    glm::vec4 color;
};

inline Vertex makeVertex(int32_t x, int32_t y, int32_t z, float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.f)
{
    return Vertex{
        .position = glm::i32vec3(x, y, z),
        .color = glm::vec4(r, g, b, a)
    };
}

inline glm::i32vec3 SwapYZ(glm::i32vec3 vec)
{
    return glm::i32vec3(vec.x, vec.z, vec.y);
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

    void Render() const;
};
