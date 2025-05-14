#pragma once
#include <vector>

#include "sokol_gfx.h"
#include "quad-sapp.h"

struct Sokol_State {
    sg_pipeline pip;
    sg_bindings world_bindings;
    sg_bindings bkg_bindings;
    sg_pass_action pass_action;
    sg_swapchain swapchain;

    Uniforms_t uniforms;
    glm::vec4 atlas_coords[32]{};

    int world_count;
    int bkg_count;
};

template<typename T>
sg_range make_sg_range(const std::vector<T>& vec) {
    return { .ptr = vec.data(), .size = vec.size() * sizeof(T) };
}