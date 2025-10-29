#include "GLRenderer.h"
#include "../../ObjFac1/ObjFac1Res.h"
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
#include "sokol_gfx.h"

GLRenderer::GLRenderer(SDL_Window* glWindow, SDL_GLContext glContext, VideoPalette* pVideoPalette)
    : glWindow(glWindow), glContext(glContext), videoPalette(pVideoPalette)
{
    int width, height;
    SDL_GetWindowSize(glWindow, &width, &height);

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

    const sg_shader_desc* floor_shdr_desc = floor_shader_desc(sg_query_backend());
    sg_shader floor_shader = sg_make_shader(floor_shdr_desc);
    sg_pipeline_desc floor_pipeline_desc = {};
    floor_pipeline_desc.index_type = SG_INDEXTYPE_UINT16;
    floor_pipeline_desc.shader = floor_shader;
    floor_pipeline_desc.sample_count = 16;
    floor_pipeline_desc.label = "floor-pipeline";
    floor_pipeline_desc.layout.attrs[ATTR_floor_position].format = SG_VERTEXFORMAT_INT3;
    floor_pipeline_desc.layout.attrs[ATTR_floor_texcoord0].format = SG_VERTEXFORMAT_FLOAT2;
    floor_pipeline_desc.layout.attrs[ATTR_floor_textureIdx].format = SG_VERTEXFORMAT_INT;
    floor_pipeline_desc.cull_mode = SG_CULLMODE_BACK;
    floor_pipeline_desc.depth.write_enabled = true;
    floor_pipeline_desc.depth.compare = SG_COMPAREFUNC_LESS_EQUAL;
    state.floor_pipeline = sg_make_pipeline(&floor_pipeline_desc);
    state.floor_uniforms.textureScale = 1.0f;
    state.floor_minimap_uniforms.textureScale = 2.0f;

    const sg_shader_desc* water_shdr_desc = floor_shader_desc(sg_query_backend());
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
    free_element_pipeline_desc.layout.attrs[ATTR_free_element_is_variant_texture].format = SG_VERTEXFORMAT_INT;
    free_element_pipeline_desc.layout.attrs[ATTR_free_element_is_variant_texture].buffer_index = 0;
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
    free_element_pipeline_desc.layout.attrs[ATTR_free_element_instance_variant].format = SG_VERTEXFORMAT_INT;
    free_element_pipeline_desc.layout.attrs[ATTR_free_element_instance_variant].buffer_index = 1;
    free_element_pipeline_desc.layout.buffers[0].step_func = SG_VERTEXSTEP_PER_VERTEX;
    free_element_pipeline_desc.layout.buffers[0].step_rate = 1;
    free_element_pipeline_desc.layout.buffers[1].step_func = SG_VERTEXSTEP_PER_INSTANCE;
    free_element_pipeline_desc.layout.buffers[1].step_rate = 1;

    free_element_pipeline_desc.cull_mode = SG_CULLMODE_BACK;
    free_element_pipeline_desc.depth.write_enabled = true;
    free_element_pipeline_desc.depth.compare = SG_COMPAREFUNC_LESS_EQUAL;
    state.free_element_pipeline = sg_make_pipeline(&free_element_pipeline_desc);
    state.free_element_uniforms.scale = 1.0f;
    state.free_element_minimap_uniforms.scale = 10.0f;

    state.pass_action.colors[0] = {
        .load_action = SG_LOADACTION_CLEAR,
        .clear_value = {0.0f, 0.0f, 0.0f, 1.0f}
    };

    sg_sampler_desc wrap_sampler_desc = {};
    wrap_sampler_desc.min_filter = SG_FILTER_LINEAR;
    wrap_sampler_desc.mag_filter = SG_FILTER_LINEAR;
    wrap_sampler_desc.mipmap_filter = SG_FILTER_LINEAR;  // Enable trilinear filtering with mipmaps
    wrap_sampler_desc.wrap_u = SG_WRAP_REPEAT;
    wrap_sampler_desc.wrap_v = SG_WRAP_REPEAT;
    state.wrap_sampler = sg_make_sampler(&wrap_sampler_desc);
    state.floor_bindings.samplers[0] = state.wrap_sampler;
    state.water_bindings.samplers[0] = state.wrap_sampler;
    state.wall_bindings.samplers[0] = state.wrap_sampler;

    sg_sampler_desc edge_sampler_desc = {};
    edge_sampler_desc.min_filter = SG_FILTER_LINEAR;
    edge_sampler_desc.mag_filter = SG_FILTER_LINEAR;
    edge_sampler_desc.wrap_u = SG_WRAP_CLAMP_TO_EDGE;
    edge_sampler_desc.wrap_v = SG_WRAP_CLAMP_TO_EDGE;
    state.edge_sampler = sg_make_sampler(&edge_sampler_desc);
    state.bkg_bindings.samplers[0] = state.edge_sampler;

    state.swapchain = {
        .width = width,
        .height = height,
        .sample_count = 16,
        .color_format = SG_PIXELFORMAT_RGBA8,
        .depth_format = SG_PIXELFORMAT_DEPTH,
    };

    simgui_desc_t simgui_desc = {};
    simgui_setup(&simgui_desc);
    ImGuiIO& io = ImGui::GetIO();
    ImFontConfig config;
    config.GlyphExtraAdvanceX = 12.0f;
    config.SizePixels = 57.0f;
    ImFont* font = io.Fonts->AddFontFromFileTTF("Roboto-MediumItalic.ttf", 57.0f, &config);
    io.FontDefault = font;
    
    io.FontGlobalScale = CalculateFontScale(height);
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
    sg_destroy_buffer(state.floor_bindings.vertex_buffers[0]);
    sg_destroy_buffer(state.floor_bindings.index_buffer);
    sg_destroy_buffer(state.water_bindings.vertex_buffers[0]);
    sg_destroy_buffer(state.water_bindings.index_buffer);
    sg_destroy_image(state.floor_bindings.images[0]);
    sg_destroy_sampler(state.floor_bindings.samplers[0]);
    sg_destroy_pipeline(state.free_element_pipeline);
    sg_destroy_pipeline(state.floor_pipeline);
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

    sg_apply_pipeline(state.floor_pipeline);
    sg_apply_uniforms(0, SG_RANGE(state.floor_uniforms));
    sg_apply_uniforms(1, SG_RANGE(state.floor_atlas_coords));
    sg_apply_bindings(&state.floor_bindings);
    sg_draw(0, state.floor_count, 1);

    sg_apply_pipeline(state.wall_pipeline);
    sg_apply_uniforms(0, SG_RANGE(state.wall_uniforms));
    sg_apply_uniforms(1, SG_RANGE(state.wall_atlas_coords));
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
        sg_apply_uniforms(1, SG_RANGE(state.floor_atlas_coords));
        sg_apply_bindings(&state.water_bindings);
        sg_draw(0, state.water_count, 1);
    }
}

void GLRenderer::EndRender() const
{
    sg_end_pass();
}

float GetFreeElementMiniMapScale(int freeElementType)
{
    switch (freeElementType)
    {
        case MR_BUMPERGATE:
            return 2.5f;
        case MR_MINE:
            return 7.5f;
        default:
            return 10.0f;
    }
}

void GLRenderer::RenderMiniMap(glm::ivec4 size)
{
    sg_color_attachment_action loadColorAction = { .load_action = SG_LOADACTION_LOAD };
    sg_depth_attachment_action clearDepthAction = { .load_action = SG_LOADACTION_CLEAR, .clear_value = 1.0f };
    sg_pass_action pass_action = {};
    pass_action.colors[0] = loadColorAction;
    pass_action.depth = clearDepthAction;

    sg_pass pass = {
        .action = pass_action,
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
        // Map is taller than it is wide
        viewportWidth = static_cast<int>(maxMinimapSize * mapRatio);
        viewportHeight = maxMinimapSize;
    }
    sg_apply_viewport(margin, margin, viewportWidth, viewportHeight, true);

    sg_apply_pipeline(state.floor_pipeline);
    sg_apply_uniforms(0, SG_RANGE(state.floor_minimap_uniforms));
    sg_apply_uniforms(1, SG_RANGE(state.floor_atlas_coords));
    sg_apply_bindings(&state.floor_bindings);
    sg_draw(0, state.floor_count, 1);

    sg_apply_pipeline(state.free_element_pipeline);
    sg_apply_uniforms(1, SG_RANGE(state.free_element_atlas_coords));
    for (auto& [freeElementType, binding] : state.free_element_bindings)
    {
        state.free_element_minimap_uniforms.scale = GetFreeElementMiniMapScale(freeElementType);
        sg_apply_uniforms(0, SG_RANGE(state.free_element_minimap_uniforms));
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

void GLRenderer::ChangeResolution(int width, int height)
{
    state.swapchain.width = width;
    state.swapchain.height = height;
    
    ImGuiIO& io = ImGui::GetIO();
    io.FontGlobalScale = CalculateFontScale(height);
}

float GLRenderer::CalculateFontScale(int height)
{
    const float targetScale = static_cast<float>(height) / 1000.0f;
    return std::max(0.3f, std::min(3.0f, targetScale));
}

template<size_t NumLevels>
void GLRenderer::PadMipmapLevelsWithUpscaling(TextureData<NumLevels>& textureData, int sourceLevel, int destStartLevel)
{
    for (int i = destStartLevel; i >= 0; i--)
    {
        // Calculate how many times to upscale (2x for each level)
        int scaleSteps = sourceLevel - i;
        int scaleFactor = 1 << scaleSteps; // 2^scaleSteps

        int sourceWidth = textureData.levels[sourceLevel].width;
        int sourceHeight = textureData.levels[sourceLevel].height;
        int scaledWidth = sourceWidth * scaleFactor;
        int scaledHeight = sourceHeight * scaleFactor;

        // Upscale by nearest-neighbor
        auto scaledPixels = new uint32_t[scaledWidth * scaledHeight];
        for (int y = 0; y < scaledHeight; y++)
        {
            for (int x = 0; x < scaledWidth; x++)
            {
                int srcX = x / scaleFactor;
                int srcY = y / scaleFactor;
                scaledPixels[y * scaledWidth + x] =
                    textureData.levels[sourceLevel].pixels[srcY * sourceWidth + srcX];
            }
        }

        textureData.levels[i].width = scaledWidth;
        textureData.levels[i].height = scaledHeight;
        textureData.levels[i].pixels = scaledPixels;
    }
}

void GLRenderer::CopyTextureToAtlasWithPadding(uint32_t* atlas_pixels, int atlas_width,
                                                const MipmapLevel& level, int rect_x, int rect_y, int mipLevel, int padding)
{

    // Calculate the actual rect position for this mip level
    int mipRectX = rect_x >> mipLevel;
    int mipRectY = rect_y >> mipLevel;
    int texWidth = level.width;
    int texHeight = level.height;

    // Copy the main texture to the center of the padded region
    for (int y = 0; y < texHeight; y++)
    {
        for (int x = 0; x < texWidth; x++)
        {
            int atlas_idx = (mipRectY + padding + y) * atlas_width + (mipRectX + padding + x);
            int tex_idx = y * texWidth + x;
            atlas_pixels[atlas_idx] = level.pixels[tex_idx];
        }
    }

    // Extrude edges to fill padding and prevent bleeding
    // Top and bottom edges
    for (int x = 0; x < texWidth; x++)
    {
        uint32_t top_pixel = level.pixels[x];
        uint32_t bottom_pixel = level.pixels[(texHeight - 1) * texWidth + x];
        for (int p = 0; p < padding; p++)
        {
            // Top padding
            int top_idx = (mipRectY + p) * atlas_width + (mipRectX + padding + x);
            atlas_pixels[top_idx] = top_pixel;
            // Bottom padding
            int bottom_idx = (mipRectY + padding + texHeight + p) * atlas_width + (mipRectX + padding + x);
            atlas_pixels[bottom_idx] = bottom_pixel;
        }
    }

    // Left and right edges
    for (int y = 0; y < texHeight; y++)
    {
        uint32_t left_pixel = level.pixels[y * texWidth];
        uint32_t right_pixel = level.pixels[y * texWidth + texWidth - 1];
        for (int p = 0; p < padding; p++)
        {
            // Left padding
            int left_idx = (mipRectY + padding + y) * atlas_width + (mipRectX + p);
            atlas_pixels[left_idx] = left_pixel;
            // Right padding
            int right_idx = (mipRectY + padding + y) * atlas_width + (mipRectX + padding + texWidth + p);
            atlas_pixels[right_idx] = right_pixel;
        }
    }

    // Corner pixels
    uint32_t top_left = level.pixels[0];
    uint32_t top_right = level.pixels[texWidth - 1];
    uint32_t bottom_left = level.pixels[(texHeight - 1) * texWidth];
    uint32_t bottom_right = level.pixels[(texHeight - 1) * texWidth + texWidth - 1];

    for (int py = 0; py < padding; py++)
    {
        for (int px = 0; px < padding; px++)
        {
            // Top-left corner
            atlas_pixels[(mipRectY + py) * atlas_width + (mipRectX + px)] = top_left;
            // Top-right corner
            atlas_pixels[(mipRectY + py) * atlas_width + (mipRectX + padding + texWidth + px)] = top_right;
            // Bottom-left corner
            atlas_pixels[(mipRectY + padding + texHeight + py) * atlas_width + (mipRectX + px)] = bottom_left;
            // Bottom-right corner
            atlas_pixels[(mipRectY + padding + texHeight + py) * atlas_width + (mipRectX + padding + texWidth + px)] = bottom_right;
        }
    }
}

template<size_t N, size_t M>
std::tuple<sg_image, std::array<glm::vec4, N>> GLRenderer::BindTexturesInternal(std::vector<TextureData<M>>& collection)
{
    std::vector<stbrp_rect> rects;
    rects.reserve(collection.size());

    // Calculate padding for level 0 (maximum padding)
    int level0Padding = ATLAS_PADDING << (M - 1);

    int max_dim = 0;
    for (const auto& texture : collection)
    {
        stbrp_rect rect;
        rect.id = texture.id;
        rect.w = texture.levels[0].width + (level0Padding * 2);
        rect.h = texture.levels[0].height + (level0Padding * 2);
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

    // Calculate atlas UV coordinates for each texture (based on level 0 packing)
    for (auto [rectIt, texIt] = std::tuple{rects.begin(), collection.begin()};
         rectIt != rects.end();
         ++rectIt, ++texIt)
    {
        const auto& rect = *rectIt;
        auto& texture = *texIt;

        // UV coordinates map to the actual texture region (excluding padding)
        texture.atlas_coords.u1 = static_cast<float>(rect.x + level0Padding) / atlas_width;
        texture.atlas_coords.v1 = static_cast<float>(rect.y + level0Padding) / atlas_height;
        texture.atlas_coords.u2 = static_cast<float>(rect.x + level0Padding + texture.levels[0].width) / atlas_width;
        texture.atlas_coords.v2 = static_cast<float>(rect.y + level0Padding + texture.levels[0].height) / atlas_height;
    }

    // Determine maximum number of mipmap levels to create
    int maxMipmapLevels = M;  // Use the template parameter

    // Create atlas for each mipmap level
    std::vector<uint32_t*> mipmap_atlases;
    mipmap_atlases.reserve(maxMipmapLevels);

    for (int mipLevel = 0; mipLevel < maxMipmapLevels; mipLevel++)
    {
        int mipAtlasWidth = std::max(1, atlas_width >> mipLevel);
        int mipAtlasHeight = std::max(1, atlas_height >> mipLevel);
        auto mip_atlas_pixels = new uint32_t[mipAtlasWidth * mipAtlasHeight]{};

        // Calculate padding for this mip level (level 0 has most padding, scales down with each level)
        int padding = ATLAS_PADDING << (maxMipmapLevels - 1 - mipLevel);

        // Copy textures to this mipmap level atlas
        for (auto [rectIt, texIt] = std::tuple{rects.begin(), collection.begin()};
             rectIt != rects.end();
             ++rectIt, ++texIt)
        {
            const auto& rect = *rectIt;
            auto& texture = *texIt;

            // Skip if texture doesn't have this mipmap level
            if (mipLevel >= static_cast<int>(M) || texture.levels[mipLevel].pixels == nullptr)
                continue;

            // Copy texture to atlas with edge extrusion padding
            CopyTextureToAtlasWithPadding(mip_atlas_pixels, mipAtlasWidth,
                                          texture.levels[mipLevel], rect.x, rect.y, mipLevel, padding);
        }

        mipmap_atlases.push_back(mip_atlas_pixels);
    }

    // Clean up texture pixels after copying to atlas
    for (auto& texture : collection)
    {
        for (size_t i = 0; i < M; i++)
        {
            delete[] texture.levels[i].pixels;
            texture.levels[i].pixels = nullptr;
        }
    }

    // Create the atlas texture with all mipmap levels
    sg_image_desc img_desc = {};
    img_desc.width = atlas_width;
    img_desc.height = atlas_height;
    img_desc.pixel_format = SG_PIXELFORMAT_RGBA8;
    img_desc.num_mipmaps = maxMipmapLevels;

    for (int mipLevel = 0; mipLevel < maxMipmapLevels; mipLevel++)
    {
        int mipAtlasWidth = std::max(1, atlas_width >> mipLevel);
        int mipAtlasHeight = std::max(1, atlas_height >> mipLevel);
        img_desc.data.subimage[0][mipLevel].ptr = mipmap_atlases[mipLevel];
        img_desc.data.subimage[0][mipLevel].size = mipAtlasWidth * mipAtlasHeight * 4;
    }

    auto atlas_texture = sg_make_image(&img_desc);

    // Clean up all atlas pixel arrays
    for (auto* atlas : mipmap_atlases)
    {
        delete[] atlas;
    }

    std::array<glm::vec4, N> atlas_coords = {};
    int i = 0;
    for (const auto& texture : collection)
    {
        if (i >= N)
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

// Explicit template instantiation for the sizes we use
template std::tuple<sg_image, std::array<glm::vec4, 32>> GLRenderer::BindTexturesInternal<32, 6>(std::vector<TextureData<6>>&);
template std::tuple<sg_image, std::array<glm::vec4, 32>> GLRenderer::BindTexturesInternal<32, 1>(std::vector<TextureData<1>>&);
template std::tuple<sg_image, std::array<glm::vec4, 64>> GLRenderer::BindTexturesInternal<64, 1>(std::vector<TextureData<1>>&);

void GLRenderer::BindFloorTextures()
{
    auto [atlas_texture, atlas_coords] = BindTexturesInternal<32, 6>(floor_textures);
    state.floor_bindings.images[0] = atlas_texture;
    state.water_bindings.images[0] = atlas_texture;
    state.floor_atlas_coords = atlas_coords;
}

void GLRenderer::BindWallTextures()
{
    auto [atlas_texture, atlas_coords] = BindTexturesInternal<32, 1>(wall_textures);
    state.wall_bindings.images[0] = atlas_texture;
    state.wall_atlas_coords = atlas_coords;
}

void GLRenderer::BindFloorVertices(const VerticesData<VertexWithTextureId>& vertices)
{
    sg_buffer_desc buf_desc = {
        .usage= {
            .vertex_buffer = true,
            .immutable = true
        },
        .data = make_sg_range(vertices.vertices),
        .label = "floor-vertices"
    };
    state.floor_bindings.vertex_buffers[0] = sg_make_buffer(&buf_desc);

    sg_buffer_desc index_buf_desc = {
        .usage= {
            .index_buffer = true,
        },
        .data = make_sg_range(vertices.indices),
        .label = "floor-indices"
    };
    state.floor_bindings.index_buffer = sg_make_buffer(&index_buf_desc);

    state.floor_count = static_cast<uint32_t>(vertices.indices.size());
}

void GLRenderer::BindWaterVertices(const VerticesData<VertexWithTextureId>& vertices)
{
    if (vertices.vertices.empty())
    {
        return;
    }

    sg_buffer_desc buf_desc = {
        .usage= {
            .vertex_buffer = true,
            .immutable = true
        },
        .data = make_sg_range(vertices.vertices),
        .label = "water-vertices"
    };
    state.water_bindings.vertex_buffers[0] = sg_make_buffer(&buf_desc);

    sg_buffer_desc index_buf_desc = {
        .usage= {
            .index_buffer = true,
        },
        .data = make_sg_range(vertices.indices),
        .label = "water-indices"
    };
    state.water_bindings.index_buffer = sg_make_buffer(&index_buf_desc);

    state.water_count = static_cast<uint32_t>(vertices.indices.size());
}

void GLRenderer::BindWallVertices(const VerticesData<WallVertex>& vertices)
{
    sg_buffer_desc buf_desc = {
        .usage= {
            .vertex_buffer = true,
            .immutable = true
        },
        .data = make_sg_range(vertices.vertices),
        .label = "wall-vertices"
    };
    state.wall_bindings.vertex_buffers[0] = sg_make_buffer(&buf_desc);

    sg_buffer_desc index_buf_desc = {
        .usage= {
            .index_buffer = true,
        },
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
            .usage= {
                .vertex_buffer = true,
                .immutable = true
            },
            .data = make_sg_range(vertices.vertices),
            .label = "free_element-vertices"
        };
        sg_buffer_desc index_buf_desc = {
            .usage= {
                .index_buffer = true,
            },
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
                .usage= {
                    .vertex_buffer = true,
                    .dynamic_update = true
                },
                .label = "free_element-instances",
            };
            state.free_element_bindings[elementId].vertex_buffers[1] = sg_make_buffer(&instance_buf_desc);
        }
        sg_update_buffer(state.free_element_bindings[elementId].vertex_buffers[1], make_sg_range(instances));
        state.free_element_instance_count[elementId] = instances.size();
    }
    this->freeElementInstances = updatedFreeElementInstances;
}

template<size_t NumLevels>
unsigned long GLRenderer::LoadTextureInternal(std::vector<TextureData<NumLevels>>& collection, MR_UInt32 id,
                                              const MR_ResBitmap* bitmap, uint8_t alpha)
{
    auto it = std::ranges::find_if(collection, [=](const auto& t) { return t.id == id; });
    if (it == collection.end())
    {
        TextureData<NumLevels> textureData = {};
        textureData.id = id;

        // Get number of available mipmap levels from bitmap
        int nbSubBitmaps = bitmap->GetNbSubBitmap();
        int levelsToLoad = std::min(static_cast<int>(NumLevels), nbSubBitmaps);

        // Load available mipmap levels into the worst (highest numbered) slots
        int destStartLevel = NumLevels - levelsToLoad;
        for (int destLevel = destStartLevel; destLevel < NumLevels; destLevel++)
        {
            int srcLevel = destLevel - destStartLevel;
            textureData.levels[destLevel].width = bitmap->GetXRes(srcLevel);
            textureData.levels[destLevel].height = bitmap->GetYRes(srcLevel);
            textureData.levels[destLevel].pixels = ConvertTextureToRGBA8(bitmap, alpha, srcLevel);
        }

        // Pad remaining levels with upscaled versions of the best quality available level (source level 0)
        if (levelsToLoad > 0 && destStartLevel > 0)
        {
            PadMipmapLevelsWithUpscaling(textureData, destStartLevel, destStartLevel - 1);
        }

        collection.push_back(textureData);
        return collection.size() - 1;
    }
    return std::distance(collection.begin(), it);
}

// Explicit template instantiation
template unsigned long GLRenderer::LoadTextureInternal<1>(std::vector<TextureData<1>>&, MR_UInt32, const MR_ResBitmap*, uint8_t);
template unsigned long GLRenderer::LoadTextureInternal<6>(std::vector<TextureData<6>>&, MR_UInt32, const MR_ResBitmap*, uint8_t);
template void GLRenderer::PadMipmapLevelsWithUpscaling<1>(TextureData<1>&, int, int);
template void GLRenderer::PadMipmapLevelsWithUpscaling<6>(TextureData<6>&, int, int);

unsigned long GLRenderer::LoadFloorTexture(MR_UInt32 id, const MR_ResBitmap* bitmap, uint8_t alpha)
{
    return LoadTextureInternal(floor_textures, id, bitmap, alpha);
}

unsigned long GLRenderer::LoadWallTexture(MR_UInt32 id, const MR_ResBitmap* bitmap)
{
    return LoadTextureInternal(wall_textures, id, bitmap);
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
        TextureData<1> textureData = {};
        textureData.id = id;
        textureData.levels[0].width = sprite->GetItemWidth(),
        textureData.levels[0].height = sprite->GetItemHeight() * sprite->GetNbItem();
        textureData.levels[0].pixels = ConvertSpriteToRGBA8(sprite);
        sprites.push_back(textureData);
        return sprites.size() - 1;
    }
    return std::distance(sprites.begin(), it);
}

unsigned long GLRenderer::GetSpriteAtlasIndex(MR_UInt32 id) const
{
    auto it = std::ranges::find_if(sprites, [id](const auto& t) { return t.id == id; });
    if (it == sprites.end()) {
        throw std::out_of_range("Sprite ID " + std::to_string(id) + " not found in atlas");
    }
    return std::distance(sprites.begin(), it);
}

void GLRenderer::BindFreeElementTextures()
{
    auto [atlas_texture, atlas_coords] = BindTexturesInternal<64, 1>(free_element_textures);
    for (auto& [_, binding] : state.free_element_bindings)
    {
        binding.images[0] = atlas_texture;
    }
    state.free_element_atlas_coords = atlas_coords;
}

void GLRenderer::BindSpriteTextures()
{
    auto [atlas_texture, atlas_coords] = BindTexturesInternal<32, 1>(sprites);
    state.sprites_image = atlas_texture;
    state.sprite_atlas_coords = atlas_coords;
}

void GLRenderer::BindBackgroundVertices(const VerticesData<Vertex>& vertices)
{
    sg_buffer_desc buf_desc = {
        .usage= {
            .vertex_buffer = true,
            .immutable = true
        },
        .data = make_sg_range(vertices.vertices),
        .label = "background-vertices"
    };
    state.bkg_bindings.vertex_buffers[0] = sg_make_buffer(&buf_desc);

    sg_buffer_desc index_buf_desc = {
        .usage= {
            .index_buffer = true,
        },
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
    auto palette = videoPalette->GetPalette();
    int width = sprite->GetItemWidth();
    int height = sprite->GetItemHeight() * sprite->GetNbItem();
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

uint32_t* GLRenderer::ConvertTextureToRGBA8(const MR_ResBitmap* bitmap, uint8_t alpha, int mipmapLevel)
{
    auto palette = videoPalette->GetPalette();
    int width = bitmap->GetXRes(mipmapLevel);
    int height = bitmap->GetYRes(mipmapLevel);
    bool isSquare = (width == height);

    MR_UInt8* lSrc = bitmap->GetBuffer(mipmapLevel);
    auto lDest = new uint32_t[width * height];

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int pixelIdx = y * width + x;
            MR_UInt8 pixelColorPaletteIdx = lSrc[pixelIdx];
            NoMFC::PALETTEENTRY& paletteEntry = palette[pixelColorPaletteIdx];
            uint32_t color =  (alpha << 24) | (paletteEntry.peBlue << 16) | (paletteEntry.peGreen << 8) | paletteEntry.peRed;

            // Square textures appear to be rotated 90 degrees, which this code corrects
            // Non-square textures are used as-is
            if (isSquare)
            {
                int rotated_x = y;
                int rotated_y = width - 1 - x;
                int rotated_pixelIdx = rotated_y * width + rotated_x;
                lDest[rotated_pixelIdx] = color;
            }
            else
            {
                lDest[pixelIdx] = color;
            }
        }
    }
    return lDest;
}

uint32_t* GLRenderer::ConvertBackgroundToRGBA8(const MR_UInt8* backImage)
{
    auto palette = videoPalette->GetPalette();
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
