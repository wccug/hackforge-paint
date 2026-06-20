#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_pixels.h>

#include <cstdint>
#include <string>

namespace hackforge {

static SDL_Window* window = nullptr;
static SDL_Renderer* renderer = nullptr;

static constexpr int window_width = 800;
static constexpr int window_height = 600;

static float lastX = 0;
static float lastY = 0;
static bool penDown = false;

// I doubt creating a namespace in a namespace is the best idea here ...
namespace button {
    static SDL_Surface* surface = nullptr;
    static SDL_Texture* texture = nullptr;
    static int width = 100;
    static int height = 25;
} // namespace hackforge::button

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

    // Initialize the target to black
    SDL_SetRenderDrawColor(hackforge::renderer, 0, 0, 0, 255);
    SDL_RenderClear(hackforge::renderer);

    // Is SDL_PIXELFORMAT_RGBA32 the correct/best pixel format to use?
    // Do we need to standize on a format for the entire project or can it be per surface?
    hackforge::button::surface = SDL_CreateSurface(
        hackforge::button::width,
        hackforge::button::height,
        SDL_PIXELFORMAT_RGBA32
    );

    // Is this how we want to handle unexpected errors?
    if (hackforge::button::surface == nullptr) {
        SDL_Log("Failed to create surface: %s", SDL_GetError());

        return SDL_APP_FAILURE;
    }

    std::uint32_t backgroudColor = SDL_MapRGBA(SDL_GetPixelFormatDetails(SDL_PIXELFORMAT_RGBA32), NULL, 192, 192, 192, 255);

    bool success = SDL_FillSurfaceRect(
        hackforge::button::surface,
        NULL,
        backgroudColor
    );
    if (!success) {
        SDL_Log("Failed to fill surface: %s", SDL_GetError());

        return SDL_APP_FAILURE;
    }

    // Creates the "buttons" border bevel
    {
        std::uint32_t hilightColour = SDL_MapRGBA(SDL_GetPixelFormatDetails(SDL_PIXELFORMAT_RGBA32), NULL, 229, 229, 229, 255);
        std::uint32_t shadowColour = SDL_MapRGBA(SDL_GetPixelFormatDetails(SDL_PIXELFORMAT_RGBA32), NULL, 153, 153, 153, 225);

        // TODO: This should probably be a class but I don't know how to define one correctly (yet)
        // Need to read Bjarne Stroustrup's book: A Tour of C++
        SDL_Rect positions[] = {
            SDL_Rect{ .x = 0, .y = 0, .w = 1, .h = hackforge::button::height },
            SDL_Rect{ .x = 0, .y = 0, .w = hackforge::button::width, .h = 1 },
            SDL_Rect{ .x = hackforge::button::width - 1, .y = 0, .w = 1, .h = hackforge::button::height },
            SDL_Rect{ .x = 0, .y = hackforge::button::height - 1, .w = hackforge::button::width, .h = 1 },
        };

        std::uint32_t colours[] = {
            hilightColour,
            hilightColour,
            shadowColour,
            shadowColour
        };

        // idk what to call this :E
        SDL_Rect factors[] = {
            SDL_Rect{ .x = 1, .y = 1, .w = 0, .h = -2 },
            SDL_Rect{ .x = 1, .y = 1, .w = -2, .h = 0 },
            SDL_Rect{ .x = -1, .y = 1, .w = 0, .h = -2 },
            SDL_Rect{ .x = 1, .y = -1, .w = -2, .h = 0 },
        };

        for (int multiplier = 0; multiplier < 2; ++multiplier) {
            for (int index = 0; index < 4; ++index) {
                SDL_Rect currentPosition = positions[index];
                SDL_Rect currentFactor = factors[index];
                SDL_Rect position = SDL_Rect{
                    .x = currentPosition.x + (currentFactor.x * multiplier),
                    .y = currentPosition.y + (currentFactor.y * multiplier),
                    .w = currentPosition.w + (currentFactor.w * multiplier),
                    .h = currentPosition.h + (currentFactor.h * multiplier)
                };
                success = SDL_FillSurfaceRect(
                    hackforge::button::surface,
                    &position,
                    colours[index]
                );
                if (!success) {
                    SDL_Log("Failed to fill surface: %s", SDL_GetError());

                    return SDL_APP_FAILURE;
                }
            }
        }
    }

    hackforge::button::texture = SDL_CreateTextureFromSurface(
        hackforge::renderer,
        hackforge::button::surface
    );
    if (hackforge::button::texture == nullptr) {
        SDL_Log("Failed to create texture from surface: %s", SDL_GetError());

        return SDL_APP_FAILURE;
    }

    // The surface isn't needed once the texture is created so it *should* be
    // safe to destroy
    SDL_DestroySurface(hackforge::button::surface);
    hackforge::button::surface = nullptr;

    return SDL_APP_CONTINUE;
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }
    else if (event->type == SDL_EVENT_MOUSE_MOTION)
    {
        hackforge::lastX = event->motion.x;
        hackforge::lastY = event->motion.y;
    }
    else if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN)
    {
        hackforge::penDown = true;
    }
    else if (event->type == SDL_EVENT_MOUSE_BUTTON_UP)
    {
        hackforge::penDown = false;
    }
    return SDL_APP_CONTINUE;
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void* appstate)
{
    // Draw the pen
    if (hackforge::penDown)
    {
        SDL_SetRenderScale(hackforge::renderer, 1, 1);
        SDL_FRect rect{};
        rect.x = hackforge::lastX;
        rect.y = hackforge::lastY;
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

    {
        // NOTE: The texture needs to be renderer after the test message otherwise
        // the SDL_SetRenderScale call will also scale up the drawn texture. Also need
        // to set the scale back to "normal" by calling SDL_SetRenderScale again.
        SDL_SetRenderScale(hackforge::renderer, 1, 1);

        SDL_FRect position = SDL_FRect{
            .x = 10.f,
            .y = 10.f,
            // Is this the correct way to cast this values to a float?
            .w = (float)hackforge::button::width,
            .h = (float)hackforge::button::height
        };

        bool isOverTexture = (
            hackforge::lastX >= position.x && hackforge::lastX <= position.x + position.w
        ) && (
            hackforge::lastY >= position.y && hackforge::lastY <= position.y + position.h
        );

        if (hackforge::penDown && isOverTexture) {
            // Flipping the texture creates the button press effect
            SDL_RenderTextureRotated(
                hackforge::renderer,
                hackforge::button::texture,
                NULL,
                &position,
                0.0,
                NULL,
                SDL_FLIP_HORIZONTAL_AND_VERTICAL
            );
        } else {
            SDL_RenderTexture(hackforge::renderer, hackforge::button::texture, NULL, &position);
        }
    }

    SDL_RenderPresent(hackforge::renderer);

    return SDL_APP_CONTINUE;
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    SDL_DestroyTexture(hackforge::button::texture);
}
