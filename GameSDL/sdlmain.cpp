#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "SDLGameApp.h"
#include <bitset>
#include <future>
#include <optional>
#include <filesystem>

#include "backends/imgui_impl_sdl3.h"

SDL_Window *sdlWindow = nullptr;
SDL_Renderer *renderer = nullptr;
SDL_Texture *texture = nullptr;
SDL_Window *glWindow = nullptr;
SDL_GLContext glContext = nullptr;
MR_SDLGameApp *game = nullptr;
int lControlState = 0;

extern "C" {
void ChangeToTrack(const char *trackFile) {
    printf("ChangeToTrack: %s\n", trackFile);
    if (game != nullptr) {
        game->Clean();
        game->LoadSelectedTrack(trackFile);
    }
}
}

std::optional<std::string> GetTrack() {
    std::string defaultTrackFile = "Steeplechase.trk";
    if (std::filesystem::exists(defaultTrackFile)) {
        std::cout << "Selected default track: " << defaultTrackFile << std::endl;
        return defaultTrackFile.c_str();
    } else {
        std::cout << "Attempting to select a track... " << std::endl;
        std::promise<std::optional<std::string> > dialogPromise;
        std::future<std::optional<std::string> > dialogFuture = dialogPromise.get_future();

        SDL_ShowOpenFileDialog([](void *userdata, const char *const*filelist, int filter) {
            auto dialogPromise = static_cast<std::promise<std::optional<std::string> > *>(userdata);
            if (filelist && *filelist && filelist[0][0] != '\0') {
                std::cout << "Track selected " << filelist[0] << std::endl;
                dialogPromise->set_value(filelist[0]);
            } else {
                std::cout << "Did not select a track file." << std::endl;
                dialogPromise->set_value(std::nullopt);
            }
        }, &dialogPromise, NULL, NULL, 0, NULL, FALSE);

        return dialogFuture.get();
    }
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    MR_SoundServer::Init();
    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");

#ifndef __EMSCRIPTEN__
    sdlWindow = SDL_CreateWindow("HoverRace SDL",
                                 640, 400,
                                 SDL_WINDOW_RESIZABLE);
    if (!sdlWindow) {
        SDL_Log("Couldn't create window =: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    SDL_SetWindowMinimumSize(sdlWindow, 640, 400);
    renderer = SDL_CreateRenderer(sdlWindow, NULL);
    if (!renderer) {
        SDL_Log("Couldn't create renderer =: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    texture = SDL_CreateTexture(renderer,
                                SDL_PIXELFORMAT_ARGB8888,
                                SDL_TEXTUREACCESS_STREAMING,
                                640, 400);
#endif

    glWindow = SDL_CreateWindow("GLHoverRace",
                                640, 400,
                                SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!glWindow) {
        SDL_Log("Couldn't create gl window =: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    SDL_SetWindowMinimumSize(glWindow, 640, 400);
    std::cout << "Created Windows and Renderer" << std::endl;

#ifdef __EMSCRIPTEN__
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
#endif
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 16);
    glContext = SDL_GL_CreateContext(glWindow);
    SDL_GL_SetSwapInterval(1); // VSync
    SDL_GL_MakeCurrent(glWindow, glContext);

    game = new MR_SDLGameApp(glWindow, glContext);
    game->InitGame();
    std::cout << "Init Game completed" << std::endl;

    auto track = GetTrack();
    if (!track.has_value())
    {
        return SDL_APP_FAILURE;
    }

    game->LoadSelectedTrack(track.value().c_str());
    ImGui_ImplSDL3_InitForOther(glWindow);
    return SDL_APP_CONTINUE;

}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    ImGui_ImplSDL3_ProcessEvent(event);

    if (
        event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS; /* end the program, reporting success to the OS. */
    }

    if (event->window.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
        return SDL_APP_SUCCESS;
    }

    if (event->type == SDL_EVENT_KEY_DOWN || event->type == SDL_EVENT_KEY_UP) {
        if (event->key.repeat) {
            return SDL_APP_CONTINUE;
        }

        if (event->type == SDL_EVENT_KEY_DOWN) {
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
        } else if (event->type == SDL_EVENT_KEY_UP) {
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
    } else if (event->type == SDL_EVENT_WINDOW_RESIZED) {
        if (event->window.windowID == SDL_GetWindowID(sdlWindow)) {
            std::cout << "SDL Window resized " << event->window.data1 << " x " << event->window.data2 << std::endl;
            SDL_DestroyTexture(texture);
            texture = SDL_CreateTexture(renderer,
                                        SDL_PIXELFORMAT_ARGB8888,
                                        SDL_TEXTUREACCESS_STREAMING,
                                        event->window.data1, event->window.data2);

            game->SetResolution(event->window.data1, event->window.data2);
        } else if (event->window.windowID == SDL_GetWindowID(glWindow)) {
            std::cout << "GL Window resized " << event->window.data1 << " x " << event->window.data2 << std::endl;
            game->SetOpenGLResolution(event->window.data1, event->window.data2);
        }
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    // necessary to re-set the control state on each frame because the existing code polled in the game loop
    game->SetControlState(lControlState);
    game->Simulate();
    game->RefreshView(texture);
    SDL_RenderTexture(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    delete game;
    game = nullptr;

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(sdlWindow);

    SDL_GL_DestroyContext(glContext);
    SDL_DestroyWindow(glWindow);
}
