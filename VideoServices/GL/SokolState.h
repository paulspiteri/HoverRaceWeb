#pragma once
#include <vector>

#include "sokol_gfx.h"
#include "shaders/background.h"
#include "shaders/world.h"

struct Sokol_State {
    sg_pass_action pass_action;
    sg_swapchain swapchain;

    WorldUniforms_t uniforms;
    glm::vec4 atlas_coords[32]{};

    sg_pipeline bkg_pipeline;
    sg_bindings bkg_bindings;
    int bkg_count;

    sg_pipeline world_pipeline;
    sg_bindings world_bindings;
    int world_count;
};

template<typename T>
sg_range make_sg_range(const std::vector<T>& vec) {
    return { .ptr = vec.data(), .size = vec.size() * sizeof(T) };
}