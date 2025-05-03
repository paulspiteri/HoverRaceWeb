#pragma once
#include "Camera.h"
#include "SokolState.h"
#include "SDL3/SDL.h"

class GLRenderer
{
    Camera camera = Camera(90);

public:
    GLRenderer(SDL_Window* glWindow, SDL_GLContext glContext);
    ~GLRenderer();
    Sokol_State state;
    SDL_Window* glWindow;
    SDL_GLContext glContext;

    void Render();
};
