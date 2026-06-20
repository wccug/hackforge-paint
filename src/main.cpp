#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_pixels.h>

#include <string>

#include "internal/Pointer.h"
#include "internal/ui/Button.h"

namespace hackforge {

static SDL_Window* window = nullptr;
static SDL_Renderer* renderer = nullptr;

static constexpr int window_width = 800;
static constexpr int window_height = 600;

static hackforge::internal::Pointer* pointer = nullptr;
static hackforge::internal::ui::Button* button = nullptr;

} // namespace hackforge

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[])
{
    /* Create the window */
    if (!SDL_CreateWindowAndRenderer("Paint", hackforge::window_width,
          hackforge::window_height,
          0, &hackforge::window, &hackforge::renderer)) {
        SDL_Log("Couldn't create window and renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    hackforge::pointer = new hackforge::internal::Pointer();

    // Initialize the target to black
    SDL_SetRenderDrawColor(hackforge::renderer, 0, 0, 0, 255);
    SDL_RenderClear(hackforge::renderer);

    hackforge::button = new hackforge::internal::ui::Button(10.f, 10.f, 100, 25);
    if (hackforge::button->initialize(hackforge::renderer) > 0) {
        SDL_Log("Failed to initialize button: %s", SDL_GetError());

        return SDL_APP_FAILURE;
    }

    return SDL_APP_CONTINUE;
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }
    hackforge::pointer->handleEvent(event);
    return SDL_APP_CONTINUE;
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void* appstate)
{
    // Draw the pen
    if (hackforge::pointer->isLeftButtonPressed())
    {
        SDL_SetRenderScale(hackforge::renderer, 1, 1);
        SDL_FRect rect{};
        rect.x = hackforge::pointer->getLastX();
        rect.y = hackforge::pointer->getLastY();
        rect.w = 10;
        rect.h = 10;
        SDL_SetRenderDrawColor(hackforge::renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(hackforge::renderer, &rect);
    }

    // Draw a test message
    {
        const std::string message = "Hello, Hackforge Paint!";
        int w = 0, h = 0;
        float x, y;
        const float scale = 4.0f;

        /* Center the message and scale it up */
        SDL_GetRenderOutputSize(hackforge::renderer, &w, &h);
        SDL_SetRenderScale(hackforge::renderer, scale, scale);
        x = ((w / scale) - SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE * message.size()) / 2;
        y = ((h / scale) - SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE) / 2;

        /* Draw the message */
        SDL_SetRenderDrawColor(hackforge::renderer, 255, 255, 255, 255);
        SDL_RenderDebugText(hackforge::renderer, x, y, message.c_str());
    }

    if (hackforge::button->update(hackforge::pointer) > 0) {
        SDL_Log("Failed to update button: %s", SDL_GetError());

        return SDL_APP_FAILURE;
    };

    if (hackforge::button->draw(hackforge::renderer) > 0) {
        SDL_Log("Failed to draw button: %s", SDL_GetError());

        return SDL_APP_FAILURE;
    };

    SDL_RenderPresent(hackforge::renderer);

    return SDL_APP_CONTINUE;
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    delete hackforge::button;
    delete hackforge::pointer;
}
