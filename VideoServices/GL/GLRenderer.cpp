#include "GLRenderer.h"
#include <cstring>

#include "SDL3/SDL_video.h"

#define STB_RECT_PACK_IMPLEMENTATION
#include <algorithm>

#include "stb_rect_pack.h"
#include "../3DViewport.h"

#include "sokol_log.h"
#include "imgui.h"
#define SOKOL_IMGUI_NO_SOKOL_APP
#include <chrono>

#include "util/sokol_imgui.h"

GLRenderer::GLRenderer(SDL_Window* glWindow, SDL_GLContext glContext, MR_VideoBuffer* videoBuffer)
    : glWindow(glWindow), glContext(glContext), videoBuffer(videoBuffer)
{
    sg_logger logger = {
        .func = slog_func
    };

    sg_desc desc = {};
    desc.environment.defaults.color_format = SG_PIXELFORMAT_RGBA8;
    desc.environment.defaults.depth_format = SG_PIXELFORMAT_DEPTH;
    desc.environment.defaults.sample_count = 16;
    desc.logger = logger;
    sg_setup(&desc);

    if (!sg_isvalid())
    {
        std::cout << "Failed to initialize sokol_gfx" << std::endl;
        throw std::runtime_error("Failed to initialize sokol_gfx");
    }
    std::cout << "Initialized sokol_gfx" << std::endl;

    const sg_shader_desc* bkg_shader_desc = background_shader_desc(sg_query_backend());
    sg_shader bkg_shader = sg_make_shader(bkg_shader_desc);
    sg_pipeline_desc bkg_pipeline_desc = {};
    bkg_pipeline_desc.index_type = SG_INDEXTYPE_UINT16;
    bkg_pipeline_desc.shader = bkg_shader;
    bkg_pipeline_desc.sample_count = 16;
    bkg_pipeline_desc.label = "background-pipeline";
    bkg_pipeline_desc.layout.attrs[ATTR_background_position].format = SG_VERTEXFORMAT_INT3;
    bkg_pipeline_desc.layout.attrs[ATTR_background_texcoord0].format = SG_VERTEXFORMAT_FLOAT2;
    bkg_pipeline_desc.cull_mode = SG_CULLMODE_BACK;
    state.bkg_pipeline = sg_make_pipeline(&bkg_pipeline_desc);

    const sg_shader_desc* world_shdr_desc = world_shader_desc(sg_query_backend());
    sg_shader world_shader = sg_make_shader(world_shdr_desc);
    sg_pipeline_desc world_pipeline_desc = {};
    world_pipeline_desc.index_type = SG_INDEXTYPE_UINT16;
    world_pipeline_desc.shader = world_shader;
    world_pipeline_desc.sample_count = 16;
    world_pipeline_desc.label = "world-pipeline";
    world_pipeline_desc.layout.attrs[ATTR_world_position].format = SG_VERTEXFORMAT_INT3;
    world_pipeline_desc.layout.attrs[ATTR_world_texcoord0].format = SG_VERTEXFORMAT_FLOAT2;
    world_pipeline_desc.layout.attrs[ATTR_world_textureIdx].format = SG_VERTEXFORMAT_INT;
    world_pipeline_desc.cull_mode = SG_CULLMODE_BACK;
    world_pipeline_desc.depth.write_enabled = true;
    world_pipeline_desc.depth.compare = SG_COMPAREFUNC_LESS_EQUAL;
    state.world_pipeline = sg_make_pipeline(&world_pipeline_desc);

    const sg_shader_desc* water_shdr_desc = world_shader_desc(sg_query_backend());
    sg_shader water_shader = sg_make_shader(water_shdr_desc);
    sg_pipeline_desc water_pipeline_desc = {};
    water_pipeline_desc.index_type = SG_INDEXTYPE_UINT16;
    water_pipeline_desc.shader = water_shader;
    water_pipeline_desc.sample_count = 16;
    water_pipeline_desc.label = "water-pipeline";
    water_pipeline_desc.layout.attrs[ATTR_water_position].format = SG_VERTEXFORMAT_INT3;
    water_pipeline_desc.layout.attrs[ATTR_water_texcoord0].format = SG_VERTEXFORMAT_FLOAT2;
    water_pipeline_desc.layout.attrs[ATTR_water_textureIdx].format = SG_VERTEXFORMAT_INT;
    water_pipeline_desc.cull_mode = SG_CULLMODE_BACK;
    water_pipeline_desc.depth.write_enabled = true;
    water_pipeline_desc.depth.compare = SG_COMPAREFUNC_LESS_EQUAL;
    water_pipeline_desc.colors[0].blend.enabled = true;
    water_pipeline_desc.colors[0].blend.src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA;
    water_pipeline_desc.colors[0].blend.dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    water_pipeline_desc.colors[0].blend.src_factor_alpha = SG_BLENDFACTOR_ONE;
    water_pipeline_desc.colors[0].blend.dst_factor_alpha = SG_BLENDFACTOR_ZERO;
    state.water_pipeline = sg_make_pipeline(&water_pipeline_desc);

    const sg_shader_desc* wall_shdr_desc = wall_shader_desc(sg_query_backend());
    sg_shader wall_shader = sg_make_shader(wall_shdr_desc);
    sg_pipeline_desc wall_pipeline_desc = {};
    wall_pipeline_desc.index_type = SG_INDEXTYPE_UINT16;
    wall_pipeline_desc.shader = wall_shader;
    wall_pipeline_desc.sample_count = 16;
    wall_pipeline_desc.label = "wall-pipeline";
    wall_pipeline_desc.layout.attrs[ATTR_wall_position].format = SG_VERTEXFORMAT_INT3;
    wall_pipeline_desc.layout.attrs[ATTR_wall_texcoord0].format = SG_VERTEXFORMAT_FLOAT2;
    wall_pipeline_desc.layout.attrs[ATTR_wall_textureIdx].format = SG_VERTEXFORMAT_INT;
    wall_pipeline_desc.layout.attrs[ATTR_wall_rotationSpeed].format = SG_VERTEXFORMAT_INT;
    wall_pipeline_desc.layout.attrs[ATTR_wall_rotationLength].format = SG_VERTEXFORMAT_INT;
    wall_pipeline_desc.layout.attrs[ATTR_wall_segment].format = SG_VERTEXFORMAT_INT;
    wall_pipeline_desc.cull_mode = SG_CULLMODE_BACK;
    wall_pipeline_desc.depth.write_enabled = true;
    wall_pipeline_desc.depth.compare = SG_COMPAREFUNC_LESS_EQUAL;
    state.wall_pipeline = sg_make_pipeline(&wall_pipeline_desc);

    const sg_shader_desc* free_element_shdr_desc = free_element_shader_desc(sg_query_backend());
    sg_shader free_element_shader = sg_make_shader(free_element_shdr_desc);
    sg_pipeline_desc free_element_pipeline_desc = {};
    free_element_pipeline_desc.index_type = SG_INDEXTYPE_UINT16;
    free_element_pipeline_desc.shader = free_element_shader;
    free_element_pipeline_desc.sample_count = 16;
    free_element_pipeline_desc.label = "free_element-pipeline";
    free_element_pipeline_desc.layout.attrs[ATTR_free_element_position].format = SG_VERTEXFORMAT_INT3;
    free_element_pipeline_desc.layout.attrs[ATTR_free_element_position].buffer_index = 0;
    free_element_pipeline_desc.layout.attrs[ATTR_free_element_texcoord0].format = SG_VERTEXFORMAT_FLOAT2;
    free_element_pipeline_desc.layout.attrs[ATTR_free_element_texcoord0].buffer_index = 0;
    free_element_pipeline_desc.layout.attrs[ATTR_free_element_textureIdx].format = SG_VERTEXFORMAT_INT;
    free_element_pipeline_desc.layout.attrs[ATTR_free_element_textureIdx].buffer_index = 0;
    free_element_pipeline_desc.layout.attrs[ATTR_free_element_sequence].format = SG_VERTEXFORMAT_INT;
    free_element_pipeline_desc.layout.attrs[ATTR_free_element_sequence].buffer_index = 0;
    free_element_pipeline_desc.layout.attrs[ATTR_free_element_frame].format = SG_VERTEXFORMAT_INT;
    free_element_pipeline_desc.layout.attrs[ATTR_free_element_frame].buffer_index = 0;
    free_element_pipeline_desc.layout.attrs[ATTR_free_element_instancePosition].format = SG_VERTEXFORMAT_INT3;
    free_element_pipeline_desc.layout.attrs[ATTR_free_element_instancePosition].buffer_index = 1;
    free_element_pipeline_desc.layout.attrs[ATTR_free_element_type].format = SG_VERTEXFORMAT_INT;
    free_element_pipeline_desc.layout.attrs[ATTR_free_element_type].buffer_index = 1;
    free_element_pipeline_desc.layout.attrs[ATTR_free_element_orientation].format = SG_VERTEXFORMAT_INT;
    free_element_pipeline_desc.layout.attrs[ATTR_free_element_orientation].buffer_index = 1;
    free_element_pipeline_desc.layout.attrs[ATTR_free_element_instance_sequence].format = SG_VERTEXFORMAT_INT;
    free_element_pipeline_desc.layout.attrs[ATTR_free_element_instance_sequence].buffer_index = 1;
    free_element_pipeline_desc.layout.attrs[ATTR_free_element_instance_frame].format = SG_VERTEXFORMAT_INT;
    free_element_pipeline_desc.layout.attrs[ATTR_free_element_instance_frame].buffer_index = 1;
    free_element_pipeline_desc.layout.buffers[0].step_func = SG_VERTEXSTEP_PER_VERTEX;
    free_element_pipeline_desc.layout.buffers[0].step_rate = 1;
    free_element_pipeline_desc.layout.buffers[1].step_func = SG_VERTEXSTEP_PER_INSTANCE;
    free_element_pipeline_desc.layout.buffers[1].step_rate = 1;

    free_element_pipeline_desc.cull_mode = SG_CULLMODE_BACK;
    free_element_pipeline_desc.depth.write_enabled = true;
    free_element_pipeline_desc.depth.compare = SG_COMPAREFUNC_LESS_EQUAL;
    state.free_element_pipeline = sg_make_pipeline(&free_element_pipeline_desc);

    state.pass_action.colors[0] = {
        .load_action = SG_LOADACTION_CLEAR,
        .clear_value = {0.0f, 0.0f, 0.0f, 1.0f}
    };

    sg_sampler_desc wrap_sampler_desc = {};
    wrap_sampler_desc.min_filter = SG_FILTER_LINEAR;
    wrap_sampler_desc.mag_filter = SG_FILTER_LINEAR;
    wrap_sampler_desc.wrap_u = SG_WRAP_REPEAT;
    wrap_sampler_desc.wrap_v = SG_WRAP_REPEAT;
    state.wrap_sampler = sg_make_sampler(&wrap_sampler_desc);
    state.world_bindings.samplers[0] = state.wrap_sampler;
    state.water_bindings.samplers[0] = state.wrap_sampler;
    state.wall_bindings.samplers[0] = state.wrap_sampler;

    sg_sampler_desc edge_sampler_desc = {};
    wrap_sampler_desc.min_filter = SG_FILTER_LINEAR;
    wrap_sampler_desc.mag_filter = SG_FILTER_LINEAR;
    wrap_sampler_desc.wrap_u = SG_WRAP_CLAMP_TO_EDGE;
    wrap_sampler_desc.wrap_v = SG_WRAP_CLAMP_TO_EDGE;
    state.edge_sampler = sg_make_sampler(&edge_sampler_desc);
    state.bkg_bindings.samplers[0] = state.edge_sampler;

    state.swapchain = {
        .width = 640,
        .height = 400,
        .sample_count = 16,
        .color_format = SG_PIXELFORMAT_RGBA8,
        .depth_format = SG_PIXELFORMAT_DEPTH,
    };

    simgui_desc_t simgui_desc = {};
    simgui_setup(&simgui_desc);
}

GLRenderer::~GLRenderer()
{
    simgui_shutdown();

    sg_destroy_buffer(state.bkg_bindings.vertex_buffers[0]);
    sg_destroy_buffer(state.bkg_bindings.index_buffer);
    sg_destroy_image(state.bkg_bindings.images[0]);
    sg_destroy_sampler(state.bkg_bindings.samplers[0]);
    sg_destroy_pipeline(state.bkg_pipeline);

    for (auto& [_, binding] : state.free_element_bindings)
    {
        sg_destroy_buffer(binding.vertex_buffers[0]);
        sg_destroy_buffer(binding.index_buffer);
        if (binding.vertex_buffers[1].id != SG_INVALID_ID)
        {
            sg_destroy_buffer(binding.vertex_buffers[1]);
        }
    }

    sg_destroy_buffer(state.wall_bindings.vertex_buffers[0]);
    sg_destroy_buffer(state.wall_bindings.index_buffer);
    sg_destroy_buffer(state.world_bindings.vertex_buffers[0]);
    sg_destroy_buffer(state.world_bindings.index_buffer);
    sg_destroy_buffer(state.water_bindings.vertex_buffers[0]);
    sg_destroy_buffer(state.water_bindings.index_buffer);
    sg_destroy_image(state.world_bindings.images[0]);
    sg_destroy_sampler(state.world_bindings.samplers[0]);
    sg_destroy_pipeline(state.free_element_pipeline);
    sg_destroy_pipeline(state.world_pipeline);
    sg_destroy_pipeline(state.water_pipeline);
    sg_destroy_pipeline(state.wall_pipeline);

    sg_shutdown();
}

void GLRenderer::BeginRender() const
{
    sg_pass pass = {
        .action = state.pass_action,
        .swapchain = state.swapchain
    };

    sg_begin_pass(&pass);

    sg_apply_pipeline(state.bkg_pipeline);
    sg_apply_uniforms(0, SG_RANGE(state.bkg_uniforms));
    sg_apply_bindings(&state.bkg_bindings);
    sg_draw(0, state.bkg_count, 1);

    sg_apply_pipeline(state.world_pipeline);
    sg_apply_uniforms(0, SG_RANGE(state.world_uniforms));
    sg_apply_uniforms(1, SG_RANGE(state.world_atlas_coords));
    sg_apply_bindings(&state.world_bindings);
    sg_draw(0, state.world_count, 1);

    sg_apply_pipeline(state.wall_pipeline);
    sg_apply_uniforms(0, SG_RANGE(state.wall_uniforms));
    sg_apply_uniforms(1, SG_RANGE(state.world_atlas_coords));
    sg_apply_bindings(&state.wall_bindings);
    sg_draw(0, state.wall_count, 1);

    sg_apply_pipeline(state.free_element_pipeline);
    sg_apply_uniforms(0, SG_RANGE(state.free_element_uniforms));
    sg_apply_uniforms(1, SG_RANGE(state.free_element_atlas_coords));
    for (auto& [freeElementType, binding] : state.free_element_bindings)
    {
        int freeElementVertexCount = state.free_element_vertex_count.at(freeElementType);
        int freeElementInstanceCount = state.free_element_instance_count.contains(freeElementType)
                                           ? state.free_element_instance_count.at(freeElementType)
                                           : 0;
        if (freeElementInstanceCount > 0)
        {
            sg_apply_bindings(&binding);
            sg_draw(0, freeElementVertexCount, freeElementInstanceCount);
        }
    }

    if (state.water_count > 0)
    {
        // water should be last as it has alpha
        sg_apply_pipeline(state.water_pipeline);
        sg_apply_uniforms(0, SG_RANGE(state.water_uniforms));
        sg_apply_uniforms(1, SG_RANGE(state.world_atlas_coords));
        sg_apply_bindings(&state.water_bindings);
        sg_draw(0, state.water_count, 1);
    }
}

void GLRenderer::EndRender() const
{
    sg_end_pass();
}

void GLRenderer::RenderMiniMap(glm::ivec4 size) const
{
    sg_pass pass = {
        .action = {
            .colors[0] = { .load_action = SG_LOADACTION_LOAD },
            .depth = { .load_action = SG_LOADACTION_CLEAR, .clear_value = 1.0f }
        },
        .swapchain = state.swapchain
    };
    sg_begin_pass(&pass);

    int margin = state.swapchain.height / 16;
    int maxMinimapSize = state.swapchain.height / 4;
    int mapWidth = size.z - size.x;
    int mapHeight = size.w - size.y;
    float mapRatio = static_cast<float>(mapWidth) / static_cast<float>(mapHeight);
    int viewportWidth, viewportHeight;
    if (mapRatio > 1.0f) {
        // Map is wider than it is tall
        viewportWidth = maxMinimapSize;
        viewportHeight = static_cast<int>(maxMinimapSize / mapRatio);
    } else {
        // Map is taller than it is wide (or square)
        viewportWidth = static_cast<int>(maxMinimapSize * mapRatio);
        viewportHeight = maxMinimapSize;
    }
    sg_apply_viewport(margin, margin, viewportWidth, viewportHeight, true);

    sg_apply_pipeline(state.world_pipeline);
    sg_apply_uniforms(0, SG_RANGE(state.world_minimap_uniforms));
    sg_apply_uniforms(1, SG_RANGE(state.world_atlas_coords));
    sg_apply_bindings(&state.world_bindings);
    sg_draw(0, state.world_count, 1);

    sg_end_pass();
    sg_commit();
}

void GLRenderer::BeginImguiFrame() const
{
    static auto last_time = std::chrono::high_resolution_clock::now();
    auto current_time = std::chrono::high_resolution_clock::now();
    float actual_delta = std::chrono::duration<float>(current_time - last_time).count();
    last_time = current_time;

    simgui_frame_desc_t frame_desc = {
        .width = state.swapchain.width,
        .height = state.swapchain.height,
        .delta_time = actual_delta,
        .dpi_scale = 1.0f
    };
    simgui_new_frame(&frame_desc);
}

void GLRenderer::EndImguiFrame() const
{
    simgui_render();
}

std::tuple<sg_image, std::array<glm::vec4, 32>> GLRenderer::BindTexturesInternal(std::vector<TextureData>& collection)
{
    std::vector<stbrp_rect> rects;
    rects.reserve(collection.size());

    int max_dim = 0;
    for (const auto& texture : collection)
    {
        stbrp_rect rect;
        rect.id = texture.id;
        rect.w = texture.width;
        rect.h = texture.height;
        max_dim = std::max(max_dim, std::max(rect.w, rect.h));
        rects.push_back(rect);
    }

    int atlas_width = max_dim;
    int atlas_height = max_dim;
    bool success = false;
    while (!success)
    {
        std::vector<stbrp_node> nodes(atlas_width);
        stbrp_context context;
        stbrp_init_target(&context, atlas_width, atlas_height, nodes.data(), atlas_width);
        if (stbrp_pack_rects(&context, rects.data(), rects.size()))
        {
            success = true;
        }
        else
        {
            // If packing failed, double both dimensions
            atlas_width *= 2;
            atlas_height *= 2;
        }
    }

    // Create the atlas texture
    auto atlas_pixels = new uint32_t[atlas_width * atlas_height]{};

    // Copy all textures to their positions in the atlas
    for (auto [rectIt, texIt] = std::tuple{rects.begin(), collection.begin()};
         rectIt != rects.end();
         ++rectIt, ++texIt)
    {
        const auto& rect = *rectIt;
        auto& texture = *texIt;

        for (int y = 0; y < rect.h; y++)
        {
            for (int x = 0; x < rect.w; x++)
            {
                int atlas_idx = (rect.y + y) * atlas_width + (rect.x + x);
                int tex_idx = y * texture.width + x;
                atlas_pixels[atlas_idx] = texture.pixels[tex_idx];
            }
        }

        float bleed_padding = 0.5f;
        texture.atlas_coords.u1 = (rect.x + bleed_padding) / atlas_width;
        texture.atlas_coords.v1 = (rect.y + bleed_padding) / atlas_height;
        texture.atlas_coords.u2 = (rect.x + rect.w - bleed_padding) / atlas_width;
        texture.atlas_coords.v2 = (rect.y + rect.h - bleed_padding) / atlas_height;

        delete[] texture.pixels; // todo
        texture.pixels = nullptr;
    }

    // Create the atlas texture
    sg_image_desc img_desc = {};
    img_desc.width = atlas_width;
    img_desc.height = atlas_height;
    img_desc.pixel_format = SG_PIXELFORMAT_RGBA8;
    img_desc.data.subimage[0][0].ptr = atlas_pixels;
    img_desc.data.subimage[0][0].size = atlas_width * atlas_height * 4;
    auto atlas_texture = sg_make_image(&img_desc);
    delete[] atlas_pixels;

    std::array<glm::vec4, 32> atlas_coords = {};
    int i = 0;
    for (const auto& texture : collection)
    {
        if (i >= 32)
        {
            throw std::runtime_error("Too many textures in atlas");
        }
        atlas_coords[i] = glm::vec4(
            texture.atlas_coords.u1,
            texture.atlas_coords.v1,
            texture.atlas_coords.u2,
            texture.atlas_coords.v2
        );
        i++;
    }

    return std::make_tuple(atlas_texture, atlas_coords);
}

void GLRenderer::BindWorldTextures()
{
    auto [atlas_texture, atlas_coords] = BindTexturesInternal(textures);
    state.world_bindings.images[0] = atlas_texture;
    state.water_bindings.images[0] = atlas_texture;
    state.wall_bindings.images[0] = atlas_texture;
    state.world_atlas_coords = atlas_coords;
}

void GLRenderer::BindWorldVertices(const VerticesData<VertexWithTextureId>& vertices)
{
    sg_buffer_desc buf_desc = {
        .type = SG_BUFFERTYPE_VERTEXBUFFER,
        .usage = SG_USAGE_IMMUTABLE,
        .data = make_sg_range(vertices.vertices),
        .label = "floor-vertices"
    };
    state.world_bindings.vertex_buffers[0] = sg_make_buffer(&buf_desc);

    sg_buffer_desc index_buf_desc = {
        .type = SG_BUFFERTYPE_INDEXBUFFER,
        .data = make_sg_range(vertices.indices),
        .label = "floor-indices"
    };
    state.world_bindings.index_buffer = sg_make_buffer(&index_buf_desc);

    state.world_count = static_cast<uint32_t>(vertices.indices.size());
}

void GLRenderer::BindWaterVertices(const VerticesData<VertexWithTextureId>& vertices)
{
    if (vertices.vertices.empty())
    {
        return;
    }

    sg_buffer_desc buf_desc = {
        .type = SG_BUFFERTYPE_VERTEXBUFFER,
        .usage = SG_USAGE_IMMUTABLE,
        .data = make_sg_range(vertices.vertices),
        .label = "water-vertices"
    };
    state.water_bindings.vertex_buffers[0] = sg_make_buffer(&buf_desc);

    sg_buffer_desc index_buf_desc = {
        .type = SG_BUFFERTYPE_INDEXBUFFER,
        .data = make_sg_range(vertices.indices),
        .label = "water-indices"
    };
    state.water_bindings.index_buffer = sg_make_buffer(&index_buf_desc);

    state.water_count = static_cast<uint32_t>(vertices.indices.size());
}

void GLRenderer::BindWallVertices(const VerticesData<WallVertex>& vertices)
{
    sg_buffer_desc buf_desc = {
        .type = SG_BUFFERTYPE_VERTEXBUFFER,
        .usage = SG_USAGE_IMMUTABLE,
        .data = make_sg_range(vertices.vertices),
        .label = "wall-vertices"
    };
    state.wall_bindings.vertex_buffers[0] = sg_make_buffer(&buf_desc);

    sg_buffer_desc index_buf_desc = {
        .type = SG_BUFFERTYPE_INDEXBUFFER,
        .data = make_sg_range(vertices.indices),
        .label = "wall-indices"
    };
    state.wall_bindings.index_buffer = sg_make_buffer(&index_buf_desc);

    state.wall_count = static_cast<uint32_t>(vertices.indices.size());
}

void GLRenderer::BindFreeElementVertices(const std::unordered_map<int, VerticesData<FreeElementVertex>>& freeElements)
{
    for (const auto& [elementId, vertices] : freeElements)
    {
        sg_buffer_desc buf_desc = {
            .type = SG_BUFFERTYPE_VERTEXBUFFER,
            .usage = SG_USAGE_IMMUTABLE,
            .data = make_sg_range(vertices.vertices),
            .label = "free_element-vertices"
        };
        sg_buffer_desc index_buf_desc = {
            .type = SG_BUFFERTYPE_INDEXBUFFER,
            .data = make_sg_range(vertices.indices),
            .label = "free_element-indices"
        };
        state.free_element_bindings[elementId].vertex_buffers[0] = sg_make_buffer(&buf_desc);
        state.free_element_bindings[elementId].index_buffer = sg_make_buffer(&index_buf_desc);
        state.free_element_bindings[elementId].samplers[0] = state.edge_sampler;
        state.free_element_vertex_count[elementId] = static_cast<int>(vertices.indices.size());
    }
}

void GLRenderer::BindFreeElementInstances(
    const std::unordered_map<int, std::vector<FreeElementInstance>>& updatedFreeElementInstances)
{
    if (freeElementInstances == updatedFreeElementInstances)
    {
        // no change, nothing to do
        return;
    }

    // delete any buffers for element types which now no longer exist
    for (const auto& [elementId, instances] : freeElementInstances)
    {
        if (!updatedFreeElementInstances.contains(elementId))
        {
            if (state.free_element_bindings[elementId].vertex_buffers[1].id != SG_INVALID_ID)
            {
                sg_destroy_buffer(state.free_element_bindings[elementId].vertex_buffers[1]);
            }
            state.free_element_instance_count[elementId] = 0;
        }
    }

    // update buffers for each element type. delete+recreate if necessary (when size changed)
    for (const auto& [elementId, instances] : updatedFreeElementInstances)
    {
        if (!state.free_element_bindings.contains(elementId))
        {
            throw std::runtime_error("Free element type not bound");
        }

        int previousElementInstanceCount = freeElementInstances.contains(elementId)
                                               ? freeElementInstances.at(elementId).size()
                                               : 0;
        if (freeElementInstances.empty() || instances.size() != previousElementInstanceCount)
        {
            if (state.free_element_bindings[elementId].vertex_buffers[1].id != SG_INVALID_ID)
            {
                sg_destroy_buffer(state.free_element_bindings[elementId].vertex_buffers[1]);
            }
            sg_buffer_desc instance_buf_desc = {
                .size = instances.size() * sizeof(FreeElementInstance),
                .type = SG_BUFFERTYPE_VERTEXBUFFER,
                .usage = SG_USAGE_DYNAMIC,
                .label = "free_element-instances",
            };
            state.free_element_bindings[elementId].vertex_buffers[1] = sg_make_buffer(&instance_buf_desc);
        }
        sg_update_buffer(state.free_element_bindings[elementId].vertex_buffers[1], make_sg_range(instances));
        state.free_element_instance_count[elementId] = instances.size();
    }
    this->freeElementInstances = updatedFreeElementInstances;
}

unsigned long GLRenderer::LoadTextureInternal(std::vector<TextureData>& collection, MR_UInt32 id,
                                              const MR_ResBitmap* bitmap, u_int8_t alpha)
{
    auto it = std::ranges::find_if(collection, [=](const auto& t) { return t.id == id; });
    if (it == collection.end())
    {
        TextureData textureData = {};
        textureData.id = id;
        textureData.width = bitmap->GetMaxXRes();
        textureData.height = bitmap->GetMaxYRes();
        textureData.pixels = ConvertTextureToRGBA8(bitmap, alpha);
        collection.push_back(textureData);
        return collection.size() - 1;
    }
    return std::distance(collection.begin(), it);
}

unsigned long GLRenderer::LoadTexture(MR_UInt32 id, const MR_ResBitmap* bitmap, u_int8_t alpha)
{
    return LoadTextureInternal(textures, id, bitmap, alpha);
}

unsigned long GLRenderer::LoadFreeElementTexture(MR_UInt32 id, const MR_ResBitmap* bitmap)
{
    return LoadTextureInternal(free_element_textures, id, bitmap);
}

unsigned long GLRenderer::LoadSprite(MR_UInt32 id, const MR_Sprite* sprite)
{
    auto it = std::ranges::find_if(sprites, [=](const auto& t) { return t.id == id; });
    if (it == sprites.end())
    {
        TextureData textureData = {};
        textureData.id = id;
        textureData.width = sprite->GetItemWidth(),
        textureData.height = sprite->GetItemHeight();
        textureData.pixels = ConvertSpriteToRGBA8(sprite);
        sprites.push_back(textureData);
        return sprites.size() - 1;
    }
    return std::distance(sprites.begin(), it);
}

void GLRenderer::BindFreeElementTextures()
{
    auto [atlas_texture, atlas_coords] = BindTexturesInternal(free_element_textures);
    for (auto& [_, binding] : state.free_element_bindings)
    {
        binding.images[0] = atlas_texture;
    }
    state.free_element_atlas_coords = atlas_coords;
}

void GLRenderer::BindSpriteTextures()
{
    auto [atlas_texture, atlas_coords] = BindTexturesInternal(sprites);
    state.sprites_image = atlas_texture;
    state.sprite_atlas_coords = atlas_coords;
}

void GLRenderer::BindBackgroundVertices(const VerticesData<Vertex>& vertices)
{
    sg_buffer_desc buf_desc = {
        .type = SG_BUFFERTYPE_VERTEXBUFFER,
        .usage = SG_USAGE_IMMUTABLE,
        .data = make_sg_range(vertices.vertices),
        .label = "background-vertices"
    };
    state.bkg_bindings.vertex_buffers[0] = sg_make_buffer(&buf_desc);

    sg_buffer_desc index_buf_desc = {
        .type = SG_BUFFERTYPE_INDEXBUFFER,
        .data = make_sg_range(vertices.indices),
        .label = "background-indices"
    };
    state.bkg_bindings.index_buffer = sg_make_buffer(&index_buf_desc);

    state.bkg_count = static_cast<uint32_t>(vertices.indices.size());
}

void GLRenderer::BindBackgroundTexture(const MR_UInt8* backImage)
{
    auto rgbaBackImg = ConvertBackgroundToRGBA8(backImage);

    sg_image_desc img_desc = {};
    img_desc.width = MR_BACK_X_RES;
    img_desc.height = MR_BACK_Y_RES;
    img_desc.pixel_format = SG_PIXELFORMAT_RGBA8;
    img_desc.data.subimage[0][0].ptr = rgbaBackImg;
    img_desc.data.subimage[0][0].size = MR_BACK_X_RES * MR_BACK_Y_RES * 4;
    auto bkg_texture = sg_make_image(&img_desc);

    state.bkg_bindings.images[0] = bkg_texture;
    delete[] rgbaBackImg; // todo
}

void GLRenderer::MakeGLContextCurrent() const
{
    SDL_GL_MakeCurrent(glWindow, glContext);
}

uint32_t* GLRenderer::ConvertSpriteToRGBA8(const MR_Sprite* sprite)
{
    auto palette = videoBuffer->GetPalette();
    int width = sprite->GetItemWidth();
    int height = sprite->GetItemHeight();
    MR_UInt8* lSrc = sprite->GetData();
    auto lDest = new uint32_t[width * height];
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int pixelIdx = y * width + x;
            MR_UInt8 pixelColorPaletteIdx = lSrc[pixelIdx];
            NoMFC::PALETTEENTRY& paletteEntry = palette[pixelColorPaletteIdx];
            uint32_t color =  ((pixelColorPaletteIdx == 0 ? 0 : 0xFF) << 24) | (paletteEntry.peBlue << 16) | (paletteEntry.peGreen << 8) | paletteEntry.peRed;
            lDest[pixelIdx] = color;
        }
    }
    return lDest;
}

uint32_t* GLRenderer::ConvertTextureToRGBA8(const MR_ResBitmap* bitmap, u_int8_t alpha)
{
    auto palette = videoBuffer->GetPalette();
    int width = bitmap->GetMaxXRes();
    int height = bitmap->GetMaxYRes();
    if (width != height)
    {
        throw std::runtime_error("Only square textures are supported");
    }
    MR_UInt8* lSrc = bitmap->GetBuffer(0);
    auto lDest = new uint32_t[width * height];
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int pixelIdx = y * width + x;
            MR_UInt8 pixelColorPaletteIdx = lSrc[pixelIdx];
            NoMFC::PALETTEENTRY& paletteEntry = palette[pixelColorPaletteIdx];
            uint32_t color =  (alpha << 24) | (paletteEntry.peBlue << 16) | (paletteEntry.peGreen << 8) | paletteEntry.peRed;

            // textures appear to be rotated 90 degrees, which this code corrects
            int rotated_x = y;
            int rotated_y = width - 1 - x;
            int rotated_pixelIdx = rotated_y * width + rotated_x;

            lDest[rotated_pixelIdx] = color;
        }
    }
    return lDest;
}

uint32_t* GLRenderer::ConvertBackgroundToRGBA8(const MR_UInt8* backImage)
{
    auto palette = videoBuffer->GetPalette();
    int srcWidth = MR_BACK_Y_RES; // image is rotated 90 degrees
    int srcHeight = MR_BACK_X_RES;
    int destWidth = srcHeight;
    int destHeight = srcWidth;

    auto lDest = new uint32_t[destWidth * destHeight];
    for (int y = 0; y < srcHeight; y++)
    {
        for (int x = 0; x < srcWidth; x++)
        {
            int srcIdx = y * srcWidth + x;
            MR_UInt8 pixelColorPaletteIdx = backImage[srcIdx];
            NoMFC::PALETTEENTRY& paletteEntry = palette[pixelColorPaletteIdx];
            uint32_t color = (paletteEntry.peBlue << 16) | (paletteEntry.peGreen << 8) | paletteEntry.peRed;

            // Apply 90-degree clockwise rotation
            int destX = y;
            int destY = (srcWidth - 1) - x;
            int destIdx = destY * destWidth + destX;

            lDest[destIdx] = color;
        }
    }

    return lDest;
}
