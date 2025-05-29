#include "GLRenderer.h"
#include <cstring>

#include "sokol_gfx.h"
#include "SDL3/SDL_video.h"

#define STB_RECT_PACK_IMPLEMENTATION
#include <algorithm>

#include "stb_rect_pack.h"
#include "../3DViewport.h"

#define SOKOL_DEBUG
#ifdef __EMSCRIPTEN__
    #define SOKOL_GLES3
#else
#define SOKOL_GLCORE
#endif

#define SOKOL_LOG_IMPL
#include "sokol_log.h"
#define SOKOL_GFX_IMPL
#include "sokol_gfx.h"

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

    if (!sg_isvalid()) {
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
    free_element_pipeline_desc.layout.attrs[ATTR_free_element_instancePosition].format = SG_VERTEXFORMAT_INT3;
    free_element_pipeline_desc.layout.attrs[ATTR_free_element_instancePosition].buffer_index = 1;
    free_element_pipeline_desc.layout.attrs[ATTR_free_element_type].format = SG_VERTEXFORMAT_INT;
    free_element_pipeline_desc.layout.attrs[ATTR_free_element_type].buffer_index = 1;
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
    wrap_sampler_desc.min_filter =SG_FILTER_LINEAR;
    wrap_sampler_desc.mag_filter = SG_FILTER_LINEAR;
    wrap_sampler_desc.wrap_u = SG_WRAP_REPEAT;
    wrap_sampler_desc.wrap_v = SG_WRAP_REPEAT;
    state.wrap_sampler = sg_make_sampler(&wrap_sampler_desc);
    state.world_bindings.samplers[0] = state.wrap_sampler;
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
}

GLRenderer::~GLRenderer()
{
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
    sg_destroy_image(state.world_bindings.images[0]);
    sg_destroy_sampler(state.world_bindings.samplers[0]);
    sg_destroy_pipeline(state.free_element_pipeline);
    sg_destroy_pipeline(state.world_pipeline);
    sg_destroy_pipeline(state.wall_pipeline);

    sg_shutdown();
}

void GLRenderer::Render() const
{
    sg_pass pass = {
        .action = state.pass_action,
        .swapchain = state.swapchain
    };

    SDL_GL_MakeCurrent(glWindow, glContext);
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
        int freeElementInstanceCount = state.free_element_instance_count.at(freeElementType);
        if (freeElementInstanceCount > 0)
        {
            sg_apply_bindings(&binding);
            sg_draw(0, freeElementVertexCount, freeElementInstanceCount);
        }
    }

    sg_end_pass();
    sg_commit();
    SDL_GL_SwapWindow(glWindow);
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
        if (i >= 32) break;
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

void GLRenderer::BindFreeElementVertices(const std::unordered_map<MR_UInt16, VerticesData<VertexWithTextureId>>& freeElements)
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
    const std::unordered_map<MR_UInt16, std::vector<FreeElementInstance>> updatedFreeElementInstances)
{
    if (this->freeElementInstances == updatedFreeElementInstances)
    {
        // no change, nothing to do
        return;
    }

    // first make current (probably only necessary while SDL version is running
    SDL_GL_MakeCurrent(glWindow, glContext);

    // delete any buffers for element types which now no longer exist
    for (const auto& [elementId, instances] : this->freeElementInstances)
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

        if (this->freeElementInstances.empty() || instances.size() != this->freeElementInstances.at(elementId).size())
        {
            if (state.free_element_bindings[elementId].vertex_buffers[1].id != SG_INVALID_ID)
            {
                sg_destroy_buffer(state.free_element_bindings[elementId].vertex_buffers[1]);
            }
            sg_buffer_desc instance_buf_desc = {
                .type = SG_BUFFERTYPE_VERTEXBUFFER,
                .usage = SG_USAGE_DYNAMIC,
                .size = instances.size() * sizeof(FreeElementInstance),
                .label = "free_element-instances",
            };
            state.free_element_bindings[elementId].vertex_buffers[1] =  sg_make_buffer(&instance_buf_desc);
        }
        sg_update_buffer(state.free_element_bindings[elementId].vertex_buffers[1], make_sg_range(instances));
        state.free_element_instance_count[elementId] = instances.size();
    }
    this->freeElementInstances = updatedFreeElementInstances;
}

unsigned long GLRenderer::LoadTextureInternal(std::vector<TextureData>& collection, MR_UInt32 id, const MR_ResBitmap* bitmap)
{
    auto it = std::ranges::find_if(collection, [=](const auto& t) { return t.id == id; });
    if (it == collection.end())
    {
        TextureData textureData = {};
        textureData.id = id;
        textureData.width = bitmap->GetMaxXRes();
        textureData.height = bitmap->GetMaxYRes();
        textureData.pixels = ConvertTextureToRGBA8(bitmap);
        collection.push_back(textureData);
        return collection.size() - 1;
    }
    return std::distance(collection.begin(), it);
}

unsigned long GLRenderer::LoadTexture(MR_UInt32 id, const MR_ResBitmap* bitmap)
{
    return LoadTextureInternal(textures, id, bitmap);
}

unsigned long GLRenderer::LoadFreeElementTexture(MR_UInt32 id, const MR_ResBitmap* bitmap)
{
    return LoadTextureInternal(free_element_textures, id, bitmap);
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
    delete[] rgbaBackImg;   // todo
}

uint32_t* GLRenderer::ConvertTextureToRGBA8(const MR_ResBitmap* bitmap)
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
            uint32_t color = (paletteEntry.peBlue << 16) | (paletteEntry.peGreen << 8) | paletteEntry.peRed;

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
    int srcWidth = MR_BACK_Y_RES;   // image is rotated 90 degrees
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
