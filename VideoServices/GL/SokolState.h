#pragma once
#include <vector>

#include "sokol_gfx.h"
#include "shaders/background.h"
#include "shaders/world.h"
#include "shaders/wall.h"
#include "shaders/free_element.h"

struct Sokol_State {
    sg_pass_action pass_action;
    sg_swapchain swapchain;

    sg_pipeline bkg_pipeline;
    BackgroundUniforms_t bkg_uniforms;
    sg_bindings bkg_bindings;
    int bkg_count;

    sg_pipeline world_pipeline;
    WorldUniforms_t world_uniforms;
    sg_bindings world_bindings;
    int world_count;

    sg_pipeline wall_pipeline;
    WallUniforms_t wall_uniforms;
    sg_bindings wall_bindings;
    int wall_count;

    sg_pipeline free_element_pipeline;
    FreeElementUniforms_t free_element_uniforms;
    sg_bindings free_element_bindings;
    int free_element_count;

    glm::vec4 atlas_coords[32]{};
};

template<typename T>
sg_range make_sg_range(const std::vector<T>& vec) {
    return { .ptr = vec.data(), .size = vec.size() * sizeof(T) };
}