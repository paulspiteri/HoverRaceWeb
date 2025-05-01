#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "SDLGameApp.h"
#include <bitset>
#include <future>
#include <optional>
#include <filesystem>

#ifdef __EMSCRIPTEN__
    #define SOKOL_GLES3
#else
    #define SOKOL_GLCORE
#endif
#define SOKOL_LOG_IMPL
#include "sokol_log.h"
#define SOKOL_GFX_IMPL
#include "sokol_gfx.h"
#include "triangle-sapp.h"

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
SDL_Texture* texture = nullptr;
SDL_Window* glWindow = nullptr;
SDL_GLContext glContext = nullptr;
MR_SDLGameApp* game = nullptr;
int lControlState = 0;

static struct {
    sg_pipeline pip;
    sg_bindings bind;
    sg_pass_action pass_action;
} state;

extern "C" 
{
    void ChangeToTrack(const char* trackFile) 
    {
        printf("ChangeToTrack: %s\n", trackFile);
        if (game != nullptr)
        {
            game->Clean();
            game->LoadSelectedTrack(trackFile);
        }
    }
}

std::optional<std::string> GetTrack() 
{
    std::string defaultTrackFile = "Steeplechase.trk";
    if (std::filesystem::exists(defaultTrackFile))
    {
       std::cout << "Selected default track: " << defaultTrackFile << std::endl;
       return defaultTrackFile.c_str();
    } 
    else 
    {
        std::cout << "Attempting to select a track... " << std::endl;
        std::promise<std::optional<std::string>> dialogPromise;
        std::future<std::optional<std::string>> dialogFuture = dialogPromise.get_future();
    
        SDL_ShowOpenFileDialog([](void* userdata, const char* const* filelist, int filter) 
        {
            auto dialogPromise = static_cast<std::promise<std::optional<std::string>>*>(userdata);
            if (filelist && *filelist && filelist[0][0] != '\0')
            {
                std::cout << "Track selected " << filelist[0] << std::endl;
                dialogPromise->set_value(filelist[0]);
            } else
            {
                std::cout << "Did not select a track file." << std::endl;
                dialogPromise->set_value(std::nullopt);
            }
        }, &dialogPromise, NULL, NULL, 0, NULL, FALSE);
 
        return dialogFuture.get();
    }
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    MR_SoundServer::Init();
    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    window = SDL_CreateWindow("HoverRace SDL",
        640, 400,
        SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_Log("Couldn't create window =: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    SDL_SetWindowMinimumSize(window, 640, 400);
    renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        SDL_Log("Couldn't create renderer =: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    glWindow = SDL_CreateWindow("GLHoverRace",
    640, 400,
    SDL_WINDOW_OPENGL);
    if (!glWindow) {
        SDL_Log("Couldn't create gl window =: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    //SDL_SetWindowMinimumSize(glWindow, 640, 400);
    glContext = SDL_GL_CreateContext(glWindow);
    SDL_GL_SetSwapInterval(1); // VSync
    SDL_GL_MakeCurrent(glWindow, glContext);

    std::cout << "Created Windows and Renderer" << std::endl;

    sg_logger logger = {
        .func = slog_func
    };

    sg_desc desc = {
        .environment.defaults = {
            .color_format = SG_PIXELFORMAT_RGBA8,
            .depth_format = SG_PIXELFORMAT_DEPTH,
            .sample_count = 1,
        },
        .logger = logger,
        .disable_validation = false
    };
    sg_setup(&desc);

    if (!sg_isvalid()) {
        std::cout << "Failed to initialize sokol_gfx" << std::endl;
        return SDL_APP_FAILURE;
    }
    std::cout << "Initialized sokol_gfx" << std::endl;

    float vertices[] = {
        // positions            // colors
        0.0f,  0.5f, 0.5f,     1.0f, 0.0f, 0.0f, 1.0f,
        0.5f, -0.5f, 0.5f,     0.0f, 1.0f, 0.0f, 1.0f,
       -0.5f, -0.5f, 0.5f,     0.0f, 0.0f, 1.0f, 1.0f
   };

    sg_buffer_desc buf_desc = {
        .data = SG_RANGE(vertices),
        .label = "triangle-vertices"
    };
    state.bind.vertex_buffers[0] = sg_make_buffer(&buf_desc);

    // create shader from code-generated sg_shader_desc
    sg_shader shd = sg_make_shader(triangle_shader_desc(sg_query_backend()));

    sg_pipeline_desc pipeline_desc {};
    pipeline_desc.shader = shd;
    pipeline_desc.label = "triangle-pipeline";
    pipeline_desc.layout.attrs[ATTR_triangle_position].format = SG_VERTEXFORMAT_FLOAT3;
    pipeline_desc.layout.attrs[ATTR_triangle_color0].format = SG_VERTEXFORMAT_FLOAT4;

    state.pip = sg_make_pipeline(&pipeline_desc);

    state.pass_action.colors[0] = {
        .load_action=SG_LOADACTION_CLEAR,
        .clear_value={0.0f, 0.0f, 0.0f, 1.0f }
    };

    texture = SDL_CreateTexture(renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        640, 400);

    game = new MR_SDLGameApp( texture );
    game->InitGame();
    std::cout << "Init Game completed" << std::endl;

    #ifdef __EMSCRIPTEN__
        // don't attempt to default a track on web
        return SDL_APP_CONTINUE;
    #endif

    auto track = GetTrack();
    if(track.has_value()) 
    {
        game->LoadSelectedTrack(track.value().c_str());
        return SDL_APP_CONTINUE;
    }
    return SDL_APP_FAILURE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (
        event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
    }

    if (event->type == SDL_EVENT_KEY_DOWN || event->type == SDL_EVENT_KEY_UP)
    {
        if (event->key.repeat) 
        {
            return SDL_APP_CONTINUE;
        }

        if (event->type == SDL_EVENT_KEY_DOWN)
        {
            switch (event->key.key) {
                case SDLK_LSHIFT:
                case SDLK_RSHIFT:
                    lControlState |= MR_MainCharacter::eMotorOn;
                    break;
                case SDLK_RIGHT:
                    lControlState |= MR_MainCharacter::eRight;
                    break;
                case SDLK_LEFT:
                    lControlState |= MR_MainCharacter::eLeft;
                    break;
                case SDLK_DOWN:
                    lControlState |= MR_MainCharacter::eBreakDirection;
                    break;
                case SDLK_UP:
                    lControlState |= MR_MainCharacter::eJump;
                    break;
                case SDLK_LCTRL:
                case SDLK_RCTRL:
                    lControlState |= MR_MainCharacter::eFire;
                    break;
                case SDLK_TAB:
                    lControlState |= MR_MainCharacter::eSelectWeapon;
                    break;
            }
        }
        else if (event->type == SDL_EVENT_KEY_UP)
        {
            switch (event->key.key) {
                case SDLK_LSHIFT:
                case SDLK_RSHIFT:
                    lControlState &= ~MR_MainCharacter::eMotorOn;
                    break;
                case SDLK_RIGHT:
                    lControlState &= ~MR_MainCharacter::eRight;
                    break;
                case SDLK_LEFT:
                    lControlState &= ~MR_MainCharacter::eLeft;
                    break;
                case SDLK_DOWN:
                    lControlState &= ~MR_MainCharacter::eBreakDirection;
                    break;
                case SDLK_UP:
                    lControlState &= ~MR_MainCharacter::eJump;
                    break;
                case SDLK_LCTRL:
                case SDLK_RCTRL:
                    lControlState &= ~MR_MainCharacter::eFire;
                    break;
                case SDLK_TAB:
                    lControlState &= ~MR_MainCharacter::eSelectWeapon;
                    break;
            }
        }
        game->SetControlState(lControlState);
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    // necessary to re-set the control state on each frame because the existing code polled in the game loop
    game->SetControlState(lControlState);
    game->Simulate();
    game->RefreshView();
    SDL_RenderTexture(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);

    sg_swapchain swapchain = {
        .width = 640,
        .height = 400,
        .sample_count = 1,
        .color_format = SG_PIXELFORMAT_RGBA8,
        .depth_format = SG_PIXELFORMAT_DEPTH,
    };
    sg_pass pass = {
        .action = state.pass_action,
        .swapchain = swapchain
    };
    SDL_GL_MakeCurrent(glWindow, glContext);
    sg_begin_pass(&pass);
    sg_apply_pipeline(state.pip);
    sg_apply_bindings(&state.bind);
    sg_draw(0, 3, 1);
    sg_end_pass();
    sg_commit();
    SDL_GL_SwapWindow(glWindow);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    delete game;
    game = nullptr;

    sg_shutdown();
}
