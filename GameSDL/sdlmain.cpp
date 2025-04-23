#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "SDLGameApp.h"
#include <bitset>
#include <future>
#include <optional>
#include <filesystem>

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;
static MR_SDLGameApp *game = NULL;
static int lControlState = 0;

std::optional<std::string> GetTrack() 
{
    std::string defaultTrackFile = "ClassicH.trk";
    if (std::filesystem::exists(defaultTrackFile))
    {
       std::cout << "Selected default track: " << defaultTrackFile << std::endl;
       return defaultTrackFile.c_str();
    } 
    else 
    {
        std::cout << "Attempting to choose a track... " << std::endl;
        std::promise<std::optional<std::string>> dialogPromise;
        std::future<std::optional<std::string>> dialogFuture = dialogPromise.get_future();
    
        SDL_ShowOpenFileDialog([](void* userdata, const char* const* filelist, int filter) 
        {
            auto dialogPromise = static_cast<std::promise<std::optional<std::string>>*>(userdata);
            if (filelist && *filelist && filelist[0][0] != '\0')
            {
                std::cout << "Chose track " << filelist[0] << std::endl;
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
    if (!SDL_CreateWindowAndRenderer("HoverRace SDL", 640, 400, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        SDL_Log("Couldn't create window and renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    texture = SDL_CreateTexture(renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        640, 400);

    std::cout << "Created Window and Renderer" << std::endl;
    game = new MR_SDLGameApp( texture );
    game->InitGame();
    std::cout << "Init Game completed" << std::endl;
    auto track = GetTrack();
    if(track.has_value()) 
    {
        game->LoadSelectedTrack(track.value().c_str());
        std::cout << "New Local Session created" << std::endl;
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
    SDL_RenderTexture(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    delete game;
    game = nullptr;
}
