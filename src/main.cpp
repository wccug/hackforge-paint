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

    // Runtime state variables for the custom tooling colors
    static SDL_Color penColor = { 255, 255, 255, 255 };    // Default Pen: White
    static SDL_Color buttonColor = { 100, 100, 100, 255 }; // Default Button Background: Dark Gray

    // Interactive UI layout bounds
    static const SDL_FRect penBtnBounds = { 10.0f, 10.0f, 150.0f, 35.0f };
    static const SDL_FRect uiBtnBounds  = { 170.0f, 10.0f, 150.0f, 35.0f };
} // namespace hackforge

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[])
{
    if (!SDL_CreateWindowAndRenderer("Paint", hackforge::window_width, hackforge::window_height,
          0, &hackforge::window, &hackforge::renderer)) {
        SDL_Log("Couldn't create window and renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    return SDL_APP_CONTINUE;
}

/* This function runs when a new event occurs. */
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

            // 1. Check Pen Color Button Intersect
            if (mx >= hackforge::penBtnBounds.x && mx <= hackforge::penBtnBounds.x + hackforge::penBtnBounds.w &&
                my >= hackforge::penBtnBounds.y && my <= hackforge::penBtnBounds.y + hackforge::penBtnBounds.h) {
                
                hackforge::penColor = OpenNativeColorPicker(hackforge::window, hackforge::penColor);
            }
            // 2. Check UI Styling Color Intersect
            else if (mx >= hackforge::uiBtnBounds.x && mx <= hackforge::uiBtnBounds.x + hackforge::uiBtnBounds.w &&
                     my >= hackforge::uiBtnBounds.y && my <= hackforge::uiBtnBounds.y + hackforge::uiBtnBounds.h) {
                
                hackforge::buttonColor = OpenNativeColorPicker(hackforge::window, hackforge::buttonColor);
            }
            // 3. Canvas Drawing Area Intersect
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

/* This function runs once per frame. */
SDL_AppResult SDL_AppIterate(void* appstate)
{
    // Clear screen to black background canvas color
    SDL_SetRenderDrawColor(hackforge::renderer, 0, 0, 0, 255);
    SDL_RenderClear(hackforge::renderer);

    // Draw the pen stroke components matching chosen properties
    if (hackforge::penDown)
    {
        SDL_SetRenderScale(hackforge::renderer, 1, 1);
        SDL_FRect rect{};
        rect.x = hackforge::lastX;
        rect.y = hackforge::lastY;
        rect.w = 10;
        rect.h = 10;
        
        SDL_SetRenderDrawColor(hackforge::renderer, hackforge::penColor.r, hackforge::penColor.g, hackforge::penColor.b, hackforge::penColor.a);
        SDL_RenderFillRect(hackforge::renderer, &rect);
    }

    // --- UI Layout Render Pass ---
    SDL_SetRenderScale(hackforge::renderer, 1, 1);

    // Draw interactive context button geometry blocks using dynamic chosen colors
    SDL_SetRenderDrawColor(hackforge::renderer, hackforge::buttonColor.r, hackforge::buttonColor.g, hackforge::buttonColor.b, 255);
    SDL_RenderFillRect(hackforge::renderer, &hackforge::penBtnBounds);
    SDL_RenderFillRect(hackforge::renderer, &hackforge::uiBtnBounds);

    // Highlight the active pen color via its button structural border accent
    SDL_SetRenderDrawColor(hackforge::renderer, hackforge::penColor.r, hackforge::penColor.g, hackforge::penColor.b, 255);
    SDL_RenderRect(hackforge::renderer, &hackforge::penBtnBounds);

    // Render text layout panels
    SDL_SetRenderDrawColor(hackforge::renderer, 255, 255, 255, 255);
    SDL_RenderDebugText(hackforge::renderer, hackforge::penBtnBounds.x + 10, hackforge::penBtnBounds.y + 10, "Set Pen Color");
    SDL_RenderDebugText(hackforge::renderer, hackforge::uiBtnBounds.x + 10, hackforge::uiBtnBounds.y + 10, "Set UI Color");

    // Draw original project demo test message
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
}
