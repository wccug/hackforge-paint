#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */

#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_surface.h>
#include <SDL3_image/SDL_image.h>
#include "bucket.hpp"

#include <chrono>
#include <string>

#include "colorpicker.hpp"
#include "common.hpp"
#include "dialogbox.hpp"

void NewDocument()
{
    hackforge::shouldClear = true;
}

void AllocateScreenDoorTexture(SDL_Renderer* renderer) 
{
    // Create a tiny 2x2 texture with Alpha support
    hackforge::screenDoor = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, 2, 2);

    // Enable alpha blending so the screen underneath shows through
    SDL_SetTextureBlendMode(hackforge::screenDoor, SDL_BLENDMODE_BLEND);

    const SDL_PixelFormatDetails* details = SDL_GetPixelFormatDetails(SDL_PIXELFORMAT_RGBA32);
    Uint32 clear = SDL_MapRGBA(details, NULL, 0, 0, 0, 0x00);
    Uint32 translucent = SDL_MapRGBA(details, NULL, 0, 0, 0, 0xCC);

    Uint32 pixels[4] = { clear, translucent, translucent, translucent };
    SDL_UpdateTexture(hackforge::screenDoor, NULL, pixels, 2 * sizeof(Uint32));
}

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
    NewDocument();

    SDL_PixelFormat pixel_format = SDL_GetWindowPixelFormat(hackforge::window);
    hackforge::canvas = SDL_CreateTexture(hackforge::renderer, pixel_format, SDL_TEXTUREACCESS_TARGET, 800, 600);
    AllocateScreenDoorTexture(hackforge::renderer);

    hackforge::currentTool = hackforge::Tool::Pencil;

    return SDL_APP_CONTINUE;
}

void OnMouseMove(SDL_Event* event)
{
    if (hackforge::showResizeDialog)
    {
        hackforge::resizeDialog.OnMouseMove(event->motion.x, event->motion.y);
        return;
    }

    bool shouldContinueProcessingMouse = true;
    hackforge::toolbar.OnMouseMove(event->motion.x, event->motion.y, &shouldContinueProcessingMouse);
    if (!shouldContinueProcessingMouse)
        return;

    hackforge::previousPenX = hackforge::currentPenX;
    hackforge::previousPenY = hackforge::currentPenY;

    hackforge::currentPenX = event->motion.x;
    hackforge::currentPenY = event->motion.y;
}

void OnMouseLeftClick(SDL_Event* event)
{
    if (hackforge::showResizeDialog)
    {
        bool closeDialog = false;
        hackforge::resizeDialog.OnMouseClick(event->button.x, event->button.y, &closeDialog);
        if (closeDialog)
        {
            hackforge::showResizeDialog = false;
        }
        return;
    }

    bool shouldContinueProcessingClicks = true;
    hackforge::toolbar.OnMouseClick(&shouldContinueProcessingClicks);
    if (!shouldContinueProcessingClicks)
        return;

    float mx = event->button.x;
    float my = event->button.y;

    hackforge::penDown = true;
}

void ClearImpl()
{
    SDL_SetRenderTarget(hackforge::renderer, hackforge::canvas);
    SDL_SetRenderDrawColor(hackforge::renderer, 0, 0, 0, 255);
    SDL_RenderClear(hackforge::renderer);

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
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }
    else if (event->type == SDL_EVENT_MOUSE_MOTION)
    {
        OnMouseMove(event);
    }
    else if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN)
    {
        if (event->button.button == SDL_BUTTON_LEFT) {

            OnMouseLeftClick(event);
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

void DrawCopyOfCanvasWithFlipMode(SDL_Renderer* renderer, SDL_Texture* canvas, SDL_FlipMode flipMode)
{
    SDL_PropertiesID props = SDL_GetTextureProperties(canvas);
    SDL_PixelFormat format = (SDL_PixelFormat)SDL_GetNumberProperty(props, SDL_PROP_TEXTURE_FORMAT_NUMBER, SDL_PIXELFORMAT_UNKNOWN);

    // Create a new target texture
    SDL_Texture* scratch = SDL_CreateTexture(renderer, format, SDL_TEXTUREACCESS_TARGET, hackforge::window_width, hackforge::window_height);

    // Set properties and copy contents
    SDL_SetRenderTarget(renderer, scratch);
    SDL_RenderTexture(renderer, canvas, NULL, NULL); // Render the original texture onto the new texture

    // Set target to canvas, and draw the copy flipped
    SDL_SetRenderTarget(hackforge::renderer, hackforge::canvas);
    SDL_RenderTextureRotated(
        hackforge::renderer,
        scratch,
        NULL,        // Source rectangle (NULL for entire texture)
        NULL,        // Destination rectangle on screen
        0.0,         // Rotation angle
        NULL,        // Center of rotation (NULL defaults to center)
        flipMode
    );

    SDL_DestroyTexture(scratch);
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void* appstate)
{
    if (hackforge::shouldExit)
        return SDL_APP_SUCCESS;

    if (hackforge::shouldClear)
    {
        ClearImpl();
        hackforge::shouldClear = false;
    }

    if (hackforge::doSave) {
        
        // make up a filename
        const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        const auto filename = "hackforge-paint-file-" + std::to_string(ms) + ".png";
        
        SDL_Surface* surface = SDL_RenderReadPixels(hackforge::renderer, nullptr);
        IMG_SavePNG(surface,filename.c_str());
        SDL_DestroySurface(surface);
        hackforge::doSave = false;
    }

    if (hackforge::doHorizontalFlip)
    {
        DrawCopyOfCanvasWithFlipMode(hackforge::renderer, hackforge::canvas, SDL_FLIP_HORIZONTAL);
        hackforge::doHorizontalFlip = false;
    }

    if (hackforge::doVerticalFlip)
    {
        DrawCopyOfCanvasWithFlipMode(hackforge::renderer, hackforge::canvas, SDL_FLIP_VERTICAL);
        hackforge::doVerticalFlip = false;
    }

    // --- 1. Canvas Drawing Pass ---
    SDL_SetRenderTarget(hackforge::renderer, hackforge::canvas);
    if (hackforge::penDown)
    {
        SDL_SetRenderScale(hackforge::renderer, 1.0f, 1.0f);


        if (hackforge::currentTool == hackforge::Tool::Pencil)
        {
            SDL_SetRenderDrawColor(hackforge::renderer, hackforge::penColor.r, hackforge::penColor.g, hackforge::penColor.b, hackforge::penColor.a);
            SDL_RenderLine(hackforge::renderer, hackforge::previousPenX, hackforge::previousPenY, hackforge::currentPenX, hackforge::currentPenY);
        }
        else if (hackforge::currentTool == hackforge::Tool::Stamp)
        {
            SDL_FRect rect{};
            rect.x = hackforge::currentPenX;
            rect.y = hackforge::currentPenY;
            rect.w = 10.0f;
            rect.h = 10.0f;

            // Use dynamic active pen color chosen from picker
            SDL_SetRenderDrawColor(hackforge::renderer, hackforge::penColor.r, hackforge::penColor.g, hackforge::penColor.b, hackforge::penColor.a);
            SDL_RenderFillRect(hackforge::renderer, &rect);

        }
        else if (hackforge::currentTool == hackforge::Tool::AnglePen)
        {
            hackforge::anglePen.Render(hackforge::renderer, hackforge::previousPenX, hackforge::previousPenY, hackforge::currentPenX, hackforge::currentPenY, hackforge::penColor);
        }
        else if (hackforge::currentTool == hackforge::Tool::PaintBucket)
        {
            RenderBucket(hackforge::renderer, hackforge::window_width, hackforge::window_height, hackforge::canvas, hackforge::penColor, hackforge::currentPenX, hackforge::currentPenY);
            hackforge::penDown = false;
        }
    }

    // --- 2. UI Layout Render Pass (Drawn over the canvas) ---
    SDL_SetRenderTarget(hackforge::renderer, nullptr); // Sets the window as the target
    SDL_SetRenderScale(hackforge::renderer, 1.0f, 1.0f);
    SDL_RenderTexture(hackforge::renderer, hackforge::canvas, NULL, NULL);
    hackforge::toolbar.Render(hackforge::renderer, hackforge::buttonColor);

    if (hackforge::showResizeDialog)
    {
        SDL_RenderTextureTiled(hackforge::renderer, hackforge::screenDoor, NULL, 1.0f, NULL);
        hackforge::resizeDialog.ShowDialog(hackforge::renderer, hackforge::buttonColor);
    }

    SDL_RenderPresent(hackforge::renderer);
    return SDL_APP_CONTINUE;
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    // SDL3 automates window and renderer cleanup inside standard callback shutdown hooks
}
