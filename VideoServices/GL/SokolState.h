#pragma once
#include <vector>

#include "sokol_gfx.h"
#include "quad-sapp.h"

struct Sokol_State {
    sg_pipeline pip;
    sg_bindings bind;
    sg_pass_action pass_action;
    sg_swapchain swapchain;

    Uniforms_t uniforms;

    size_t wallVertexCount;
};

template<typename T>
sg_range make_sg_range(const std::vector<T>& vec) {
    return { .ptr = vec.data(), .size = vec.size() * sizeof(T) };
}