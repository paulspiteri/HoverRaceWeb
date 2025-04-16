#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "SDLGameApp.h"
#include <bitset>

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;
static MR_SDLGameApp *game = NULL;
static int lControlState = 0;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    if (!SDL_CreateWindowAndRenderer("HoverRace SDL", 640, 400, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        SDL_Log("Couldn't create window and renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    texture = SDL_CreateTexture(renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        640, 400);

    game = new MR_SDLGameApp( texture );
    game->InitGame();
    game->NewLocalSession();
    
    return SDL_APP_CONTINUE;
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
                case SDLK_SPACE:
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
            std::clog << "DOWN " << std::bitset<16>(lControlState) << std::endl;
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
                case SDLK_SPACE:
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
            std::clog << "UP   " << std::bitset<16>(lControlState) << std::endl;
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
