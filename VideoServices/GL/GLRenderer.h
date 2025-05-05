#pragma once
#include "Camera.h"
#include "SokolState.h"
#include "SDL3/SDL.h"

struct Vertex
{
    int32_t position[3]; // x, y, z
    float color[4]; // r, g, b, a
};

inline Vertex makeVertex(int32_t x, int32_t y, int32_t z, float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.f)
{
    return Vertex{
        .position = {x, z, y,},
        .color = {r, g, b, a}
    };
    return Vertex{
        .position = {x, y, z},
        .color = {1.0f, 0.0f, 0.0f, 1.0f}
    };
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
