#include "GLRenderer.h"
#include <cstring>

#include "sokol_gfx.h"
#include "SDL3/SDL_video.h"

#define STB_RECT_PACK_IMPLEMENTATION
#include <algorithm>

#include "stb_rect_pack.h"


GLRenderer::GLRenderer(SDL_Window* glWindow, SDL_GLContext glContext, NoMFC::PALETTEENTRY* colorPalette)
    : glWindow(glWindow), glContext(glContext), state{}, colorPalette(colorPalette)
{
    const sg_shader_desc* shader_desc = quad_shader_desc(sg_query_backend());
    sg_shader shd = sg_make_shader(shader_desc);

    sg_pipeline_desc pipeline_desc = {};
    pipeline_desc.index_type = SG_INDEXTYPE_UINT16;
    pipeline_desc.shader = shd;
    pipeline_desc.sample_count = 4;
    pipeline_desc.label = "quad-pipeline";
    pipeline_desc.layout.attrs[ATTR_quad_position].format = SG_VERTEXFORMAT_INT3;
    pipeline_desc.layout.attrs[ATTR_quad_color0].format = SG_VERTEXFORMAT_FLOAT4;
    pipeline_desc.layout.attrs[ATTR_quad_texcoord0].format = SG_VERTEXFORMAT_FLOAT2;
    pipeline_desc.layout.attrs[ATTR_quad_textureIdx].format = SG_VERTEXFORMAT_UINT;
    pipeline_desc.cull_mode = SG_CULLMODE_BACK;
    pipeline_desc.depth.write_enabled = true;
    pipeline_desc.depth.compare = SG_COMPAREFUNC_LESS_EQUAL;
    state.pip = sg_make_pipeline(&pipeline_desc);

    state.pass_action.colors[0] = {
        .load_action = SG_LOADACTION_CLEAR,
        .clear_value = {0.0f, 0.0f, 0.0f, 1.0f}
    };

    sg_sampler_desc smp_desc = {};
    smp_desc.min_filter = SG_FILTER_LINEAR;
    smp_desc.mag_filter = SG_FILTER_LINEAR;
    smp_desc.wrap_u = SG_WRAP_REPEAT;
    smp_desc.wrap_v = SG_WRAP_REPEAT;
    sg_sampler smp = sg_make_sampler(&smp_desc);
    state.bind.samplers[3] = smp;

    state.swapchain = {
        .width = 640,
        .height = 400,
        .sample_count = 4,
        .color_format = SG_PIXELFORMAT_RGBA8,
        .depth_format = SG_PIXELFORMAT_DEPTH,
    };
}

GLRenderer::~GLRenderer()
{
    sg_destroy_buffer(state.bind.vertex_buffers[0]);
    sg_destroy_buffer(state.bind.index_buffer);
    //  sg_destroy_image(state.bind.images[1]);
    sg_destroy_sampler(state.bind.samplers[3]);
    sg_destroy_pipeline(state.pip);

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
    sg_apply_pipeline(state.pip);
    sg_apply_bindings(&state.bind);
    sg_apply_uniforms(0, SG_RANGE(state.uniforms));
    sg_apply_uniforms(1, SG_RANGE(state.atlas_coords));
    sg_draw(0, state.wallVertexCount, 1);
    sg_end_pass();
    sg_commit();
    SDL_GL_SwapWindow(glWindow);
}


void GLRenderer::BindTextures()
{
    std::vector<stbrp_rect> rects;
    rects.reserve(textures.size());

    int max_dim = 0;
    for (const auto& texture : textures)
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
    for (auto [rectIt, texIt] = std::tuple{rects.begin(), textures.begin()};
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

        texture.atlas_coords.u1 = static_cast<float>(rect.x) / atlas_width;
        texture.atlas_coords.v1 = static_cast<float>(rect.y) / atlas_height;
        texture.atlas_coords.u2 = static_cast<float>(rect.x + rect.w) / atlas_width;
        texture.atlas_coords.v2 = static_cast<float>(rect.y + rect.h) / atlas_height;

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

    state.bind.images[2] = atlas_texture;

    int i = 0;
    for (const auto& texture : textures)
    {
        if (i >= 64) break;
        state.atlas_coords[i] = glm::vec4(
            texture.atlas_coords.u1,
            texture.atlas_coords.v1,
            texture.atlas_coords.u2,
            texture.atlas_coords.v2
        );
        std::cout << "Texture " << " (index " << i << "): "
            << "(" << texture.atlas_coords.u1 << ", " << texture.atlas_coords.v1 << ") to "
            << "(" << texture.atlas_coords.u2 << ", " << texture.atlas_coords.v2 << ")\n";

        i++;
    }
}

void GLRenderer::BindVertices(const VerticesData& vertices)
{
    sg_buffer_desc buf_desc = {
        .type = SG_BUFFERTYPE_VERTEXBUFFER,
        .usage = SG_USAGE_IMMUTABLE,
        .data = make_sg_range(vertices.vertices),
        .label = "wall-vertices"
    };

    state.bind.vertex_buffers[0] = sg_make_buffer(&buf_desc);

    sg_buffer_desc index_buf_desc = {
        .type = SG_BUFFERTYPE_INDEXBUFFER,
        .data = make_sg_range(vertices.indices),
        .label = "wall-indices"
    };
    state.bind.index_buffer = sg_make_buffer(&index_buf_desc);

    state.wallVertexCount = static_cast<uint32_t>(vertices.indices.size());

    for (const auto& vertex : vertices.vertices)
    {
        std::cout << "UV: (" << vertex.texcoord.x << ", " << vertex.texcoord.y
            << ") texIdx: " << vertex.textureIdx << std::endl;
    }
}

unsigned long GLRenderer::LoadTexture(MR_UInt16 id, const MR_ResBitmap* bitmap)
{
    auto it = std::ranges::find_if(textures, [=](const auto& t) { return t.id == id; });
    if (it == textures.end())
    {
        TextureData textureData = {};
        textureData.id = id;
        textureData.width = bitmap->GetMaxXRes();
        textureData.height = bitmap->GetMaxYRes();
        textureData.pixels = ConvertTextureToRGBA8(bitmap);
        textures.push_back(textureData);
        return textures.size() - 1;
    }
    return std::distance(textures.begin(), it);
}

uint32_t* GLRenderer::ConvertTextureToRGBA8(const MR_ResBitmap* bitmap)
{
    int width = bitmap->GetMaxXRes();
    int height = bitmap->GetMaxYRes();
    MR_UInt8* lSrc = bitmap->GetBuffer(0);
    auto lDest = new uint32_t[width * height];

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int pixelIdx = y * width + x;
            MR_UInt8 pixelColorPaletteIdx = lSrc[pixelIdx];
            NoMFC::PALETTEENTRY& paletteEntry = colorPalette[pixelColorPaletteIdx];
            uint32_t color = 0xFF000000 | (paletteEntry.peRed << 16) | (paletteEntry.peGreen << 8) | paletteEntry.
                peBlue;
            lDest[pixelIdx] = color;
        }
    }

    return lDest;
}
