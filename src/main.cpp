#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>
#include <string>
#include "win32_colorpicker.h"

namespace hackforge {
    static SDL_Window* window = nullptr;
    static SDL_Renderer* renderer = nullptr;
    static constexpr int window_width = 800;
    static constexpr int window_height = 600;

    static float lastX = 0;
    static float lastY = 0;
    static bool penDown = false;

    // --- State variables for the Custom Tooling colors ---
    static SDL_Color penColor = { 255, 255, 255, 255 };   // Default Pen: White
    static SDL_Color buttonColor = { 100, 100, 100, 255 }; // Default Button Background: Dark Gray

    // Interactive interface button geometry bounding regions
    static const SDL_FRect penBtnBounds = { 10.0f, 10.0f, 150.0f, 35.0f };
    static const SDL_FRect uiBtnBounds = { 170.0f, 10.0f, 150.0f, 35.0f };
} // namespace hackforge

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[])
{
    /* Create the window and renderer */
    if (!SDL_CreateWindowAndRenderer("Paint", hackforge::window_width, hackforge::window_height,
        0, &hackforge::window, &hackforge::renderer)) {
        SDL_Log("Couldn't create window and renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // Initialize the canvas target background color to Black once at startup
    SDL_SetRenderDrawColor(hackforge::renderer, 0, 0, 0, 255);
    SDL_RenderClear(hackforge::renderer);

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
        if (event->button.button == SDL_BUTTON_LEFT) {
            float mx = event->button.x;
            float my = event->button.y;

            // 1. Check if user clicked the "Set Pen Color" button bounding frame
            if (mx >= hackforge::penBtnBounds.x && mx <= hackforge::penBtnBounds.x + hackforge::penBtnBounds.w &&
                my >= hackforge::penBtnBounds.y && my <= hackforge::penBtnBounds.y + hackforge::penBtnBounds.h) {

                hackforge::penColor = OpenNativeColorPicker(hackforge::window, hackforge::penColor);
            }
            // 2. Check if user clicked the "Set Button Color" button bounding frame
            else if (mx >= hackforge::uiBtnBounds.x && mx <= hackforge::uiBtnBounds.x + hackforge::uiBtnBounds.w &&
                my >= hackforge::uiBtnBounds.y && my <= hackforge::uiBtnBounds.y + hackforge::uiBtnBounds.h) {

                hackforge::buttonColor = OpenNativeColorPicker(hackforge::window, hackforge::buttonColor);
            }
            // 3. Otherwise, they are drawing on the canvas space below the menu layout (y > 55)
            else if (my > 55.0f) {
                hackforge::penDown = true;
            }
        }
    }
    else if (event->type == SDL_EVENT_MOUSE_BUTTON_UP)
    {
        if (event->button.button == SDL_BUTTON_LEFT) {
            hackforge::penDown = false;
        }
    }
    return SDL_APP_CONTINUE;
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void* appstate)
{
    // NOTE: SDL_RenderClear() removed so your pen brush strokes persist on the canvas!

    // --- 1. Canvas Drawing Pass ---
    if (hackforge::penDown)
    {
        SDL_SetRenderScale(hackforge::renderer, 1.0f, 1.0f);
        SDL_FRect rect{};
        rect.x = hackforge::lastX;
        rect.y = hackforge::lastY;
        rect.w = 10.0f;
        rect.h = 10.0f;

        // Use dynamic active pen color chosen from picker
        SDL_SetRenderDrawColor(hackforge::renderer, hackforge::penColor.r, hackforge::penColor.g, hackforge::penColor.b, hackforge::penColor.a);
        SDL_RenderFillRect(hackforge::renderer, &rect);
    }

    // --- 2. UI Layout Render Pass (Drawn over the canvas) ---
    SDL_SetRenderScale(hackforge::renderer, 1.0f, 1.0f);

    // Clear the menu header zone so drawings don't bleed into your UI text/buttons
    SDL_FRect menuBarZone = { 0.0f, 0.0f, (float)hackforge::window_width, 55.0f };
    SDL_SetRenderDrawColor(hackforge::renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(hackforge::renderer, &menuBarZone);

    // Draw interactive context button geometry blocks using dynamic chosen UI colors
    SDL_SetRenderDrawColor(hackforge::renderer, hackforge::buttonColor.r, hackforge::buttonColor.g, hackforge::buttonColor.b, 255);
    SDL_RenderFillRect(hackforge::renderer, &hackforge::penBtnBounds);
    SDL_RenderFillRect(hackforge::renderer, &hackforge::uiBtnBounds);

    // Highlight the active pen color via its button structural border accent
    SDL_SetRenderDrawColor(hackforge::renderer, hackforge::penColor.r, hackforge::penColor.g, hackforge::penColor.b, 255);
    SDL_RenderRect(hackforge::renderer, &hackforge::penBtnBounds);

    // Draw tool visual label text markers
    SDL_SetRenderDrawColor(hackforge::renderer, 255, 255, 255, 255);
    SDL_RenderDebugText(hackforge::renderer, hackforge::penBtnBounds.x + 10.0f, hackforge::penBtnBounds.y + 10.0f, "Set Pen Color");
    SDL_RenderDebugText(hackforge::renderer, hackforge::uiBtnBounds.x + 10.0f, hackforge::uiBtnBounds.y + 10.0f, "Set UI Color");

    // Draw original project demo test message (Centered)
    {
        const std::string message = "Hello, Hackforge Paint!";
        int w = 0, h = 0;
        float x, y;
        const float scale = 4.0f;

        SDL_GetRenderOutputSize(hackforge::renderer, &w, &h);
        SDL_SetRenderScale(hackforge::renderer, scale, scale);
        x = ((w / scale) - SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE * message.size()) / 2;
        y = ((h / scale) - SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE) / 2;

        SDL_SetRenderDrawColor(hackforge::renderer, 255, 255, 255, 255);
        SDL_RenderDebugText(hackforge::renderer, x, y, message.c_str());
    }

    SDL_RenderPresent(hackforge::renderer);
    return SDL_APP_CONTINUE;
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    // SDL3 automates window and renderer cleanup inside standard callback shutdown hooks
}
