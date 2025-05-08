#include "GLRenderer.h"

#include <cstring>

#include "sokol_gfx.h"
#include "SDL3/SDL_video.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

GLRenderer::GLRenderer(SDL_Window* glWindow, SDL_GLContext glContext)
    : glWindow(glWindow), glContext(glContext), state{}
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
    pipeline_desc.cull_mode = SG_CULLMODE_BACK;
    pipeline_desc.depth.write_enabled = true;
    pipeline_desc.depth.compare = SG_COMPAREFUNC_LESS_EQUAL;
    state.pip = sg_make_pipeline(&pipeline_desc);

    state.pass_action.colors[0] = {
        .load_action = SG_LOADACTION_CLEAR,
        .clear_value = {0.0f, 0.0f, 0.0f, 1.0f}
    };

    state.swapchain = {
        .width = 640,
        .height = 400,
        .sample_count = 4,
        .color_format = SG_PIXELFORMAT_RGBA8,
        .depth_format = SG_PIXELFORMAT_DEPTH,
    };

    int width, height;
    unsigned char* brickTexture = stbi_load("metal_plate.bmp", &width, &height, nullptr, STBI_rgb_alpha);
    sg_image_desc img_desc = {};
    img_desc.width = width;
    img_desc.height = height;
    img_desc.pixel_format = SG_PIXELFORMAT_RGBA8;
    img_desc.data.subimage[0][0].ptr = brickTexture;
    img_desc.data.subimage[0][0].size = width * height * 4;
    sg_image texture = sg_make_image(&img_desc);
    state.bind.images[1] = texture;
    stbi_image_free(brickTexture);

    sg_sampler_desc smp_desc = {};
    smp_desc.min_filter = SG_FILTER_LINEAR;
    smp_desc.mag_filter = SG_FILTER_LINEAR;
    smp_desc.wrap_u = SG_WRAP_REPEAT;
    smp_desc.wrap_v = SG_WRAP_REPEAT;
    sg_sampler smp = sg_make_sampler(&smp_desc);
    state.bind.samplers[2] = smp;
}

GLRenderer::~GLRenderer()
{
    sg_destroy_buffer(state.bind.vertex_buffers[0]);
    sg_destroy_buffer(state.bind.index_buffer);
    sg_destroy_image(state.bind.images[1]);
    sg_destroy_sampler(state.bind.samplers[2]);
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
    sg_draw(0, state.wallVertexCount, 1);
    sg_end_pass();
    sg_commit();
    SDL_GL_SwapWindow(glWindow);
}


void GLRenderer::SetVertices(const VerticesData& vertices)
{
    sg_buffer_desc buf_desc = {
        .type = SG_BUFFERTYPE_VERTEXBUFFER,
        .usage = SG_USAGE_IMMUTABLE,
        .data = make_sg_range(vertices.vertices),
        .label = "wall-vertices"
    };

    this->state.bind.vertex_buffers[0] = sg_make_buffer(&buf_desc);

    sg_buffer_desc index_buf_desc = {
        .type = SG_BUFFERTYPE_INDEXBUFFER,
        .data = make_sg_range(vertices.indices),
        .label = "wall-indices"
    };
    this->state.bind.index_buffer = sg_make_buffer(&index_buf_desc);

    this->state.wallVertexCount = static_cast<uint32_t>(vertices.indices.size());
}
