#include "GLRenderer.h"

#include <cstring>

#include "sokol_gfx.h"
#include "SDL3/SDL_video.h"

GLRenderer::GLRenderer(SDL_Window* glWindow, SDL_GLContext glContext)
    : glWindow(glWindow), glContext(glContext), state{}
{
    // float vertices[] = {
    //     // positions            colors
    //     -0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f,
    //     0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f,
    //     0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f,
    //     -0.5f, -0.5f, 0.5f, 1.0f, 1.0f, 0.0f, 1.0f,
    // };
    // Vertex vertices[] = {
    //    makeVertex(-10.5, 10.5,10.5, 1.0f,0.0f),
    //     makeVertex(10.5, 10.5,10.5),
    //    makeVertex(10.5, -10.5,10.5,0,0,1.0f),
    //    makeVertex(-10.5, -10.5,10.5,0,0,1.0f),
    // };
    // Vertex vertices[] = {
    //     makeVertex(364932, 40639,4000),
    //     makeVertex(364932, 40639,0),
    //     makeVertex(362498, 71283,4000),
    //     makeVertex(362498, 71283,0),
    //  };
    // sg_buffer_desc buf_desc = {
    //     .size = sizeof(vertices),
    //     .type = SG_BUFFERTYPE_VERTEXBUFFER,
    //     .usage = SG_USAGE_IMMUTABLE,
    //     .data = SG_RANGE(vertices),
    //     .label = "quad-vertices"
    // };
    //
    // state.bind.vertex_buffers[0] = sg_make_buffer(&buf_desc);
    //
    // uint16_t indices[] = {0, 1, 2, 1, 2, 3};
    // sg_buffer_desc index_buf_desc = {
    //     .type = SG_BUFFERTYPE_INDEXBUFFER,
    //     .data = SG_RANGE(indices),
    //     .label = "quad-indices"
    // };
    // state.bind.index_buffer = sg_make_buffer(&index_buf_desc);

    const sg_shader_desc* shader_desc = quad_shader_desc(sg_query_backend());
    sg_shader shd = sg_make_shader(shader_desc);

    sg_pipeline_desc pipeline_desc = {};
    pipeline_desc.index_type = SG_INDEXTYPE_UINT16;
    pipeline_desc.shader = shd;
    pipeline_desc.sample_count = 4;
    pipeline_desc.label = "quad-pipeline";
    pipeline_desc.layout.attrs[ATTR_quad_position].format = SG_VERTEXFORMAT_INT3;
    pipeline_desc.layout.attrs[ATTR_quad_color0].format = SG_VERTEXFORMAT_FLOAT4;
    pipeline_desc.cull_mode = SG_CULLMODE_BACK;
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
}

GLRenderer::~GLRenderer()
{
    sg_destroy_buffer(state.bind.vertex_buffers[0]);
    sg_destroy_buffer(state.bind.index_buffer);
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
