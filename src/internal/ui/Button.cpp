#include "Button.h"

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_log.h>
#include <cstdint>

hackforge::internal::ui::Button::Button(
    float x, float y, int width, int height
) :x{x}, y{y}, width{width}, height{height} {
    isPressed = false;
};

hackforge::internal::ui::Button::~Button() {
    SDL_DestroySurface(surface);
    SDL_DestroyTexture(texture);
}

int hackforge::internal::ui::Button::update(hackforge::internal::Pointer* pointer) {
    isPressed = (
        pointer->getLastX() >= x && pointer->getLastX() <= x + (float)width
    ) && (
        pointer->getLastY() >= y && pointer->getLastY() <= y + (float)height
    ) && pointer->isLeftButtonPressed();

    return SDL_APP_CONTINUE;
}

int hackforge::internal::ui::Button::draw(SDL_Renderer* renderer) {
    // NOTE: The texture needs to be renderer after the test message otherwise
    // the SDL_SetRenderScale call will also scale up the drawn texture. Also need
    // to set the scale back to "normal" by calling SDL_SetRenderScale again.
    SDL_SetRenderScale(renderer, 1, 1);

    SDL_FRect position = SDL_FRect{
        .x = x,
        .y = y,
        .w = (float)width,
        .h = (float)height
    };

    if (isPressed) {
        // Flipping the texture creates the button press effect
        SDL_RenderTextureRotated(
            renderer,
            texture,
            NULL,
            &position,
            0.0,
            NULL,
            SDL_FLIP_HORIZONTAL_AND_VERTICAL
        );
    } else {
        SDL_RenderTexture(renderer, texture, NULL, &position);
    }

    return SDL_APP_CONTINUE;
}

int hackforge::internal::ui::Button::initialize(SDL_Renderer* renderer) {
    // FOUR!!
    if (int error = createSurface() > 0) {
        return error;
    }

    std::uint32_t hilightColour = SDL_MapRGBA(SDL_GetPixelFormatDetails(SDL_PIXELFORMAT_RGBA32), NULL, 229, 229, 229, 255);
    std::uint32_t shadowColour = SDL_MapRGBA(SDL_GetPixelFormatDetails(SDL_PIXELFORMAT_RGBA32), NULL, 153, 153, 153, 225);

    SDL_Rect positions[] = {
        SDL_Rect{ .x = 0, .y = 0, .w = 1, .h = height },
        SDL_Rect{ .x = 0, .y = 0, .w = width, .h = 1 },
        SDL_Rect{ .x = width - 1, .y = 0, .w = 1, .h = height },
        SDL_Rect{ .x = 0, .y = height - 1, .w = width, .h = 1 },
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
            int success = SDL_FillSurfaceRect(
                surface,
                &position,
                colours[index]
            );
            if (!success) {
                SDL_Log("Failed to fill surface: %s", SDL_GetError());

                return SDL_APP_FAILURE;
            }
        }
    }

    return createTexture(renderer);
}

int hackforge::internal::ui::Button::createSurface() {
    // Is SDL_PIXELFORMAT_RGBA32 the correct/best pixel format to use?
    // Do we need to standize on a format for the entire project or can it be per surface?
    surface = SDL_CreateSurface(
        width,
        height,
        SDL_PIXELFORMAT_RGBA32
    );

    // Is this how we want to handle unexpected errors?
    if (surface == nullptr) {
        SDL_Log("Failed to create surface: %s", SDL_GetError());

        return SDL_APP_FAILURE;
    }

    std::uint32_t backgroudColor = SDL_MapRGBA(SDL_GetPixelFormatDetails(SDL_PIXELFORMAT_RGBA32), NULL, 192, 192, 192, 255);

    bool success = SDL_FillSurfaceRect(
        surface,
        NULL,
        backgroudColor
    );
    if (!success) {
        SDL_Log("Failed to fill surface: %s", SDL_GetError());

        return SDL_APP_FAILURE;
    }

    return SDL_APP_CONTINUE;
}

int hackforge::internal::ui::Button::createTexture(SDL_Renderer *renderer) {
    texture = SDL_CreateTextureFromSurface(
        renderer,
        surface
    );

    if (texture == nullptr) {
        SDL_Log("Failed to create texture from surface: %s", SDL_GetError());

        return SDL_APP_FAILURE;
    }

    return SDL_APP_CONTINUE;
}
