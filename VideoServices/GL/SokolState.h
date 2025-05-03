#pragma once
#include "sokol_gfx.h"
#include "quad-sapp.h"

struct Sokol_State {
    sg_pipeline pip;
    sg_bindings bind;
    sg_pass_action pass_action;
    sg_swapchain swapchain;

    Uniforms_t uniforms;
};
