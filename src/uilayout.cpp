#include "uilayout.hpp"

namespace hackforge
{
    bool IsInBounds(float x, float y, SDL_FRect const& bounds) {
        if (x > bounds.x && x < bounds.x + bounds.w && y > bounds.y &&
            y < bounds.y + bounds.h) {
            return true;
        }
        else {
            return false;
        }
    }

    float GetRenderedTextWidthInPixels(size_t stringLength)
    {
        return stringLength * SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE* hackforge::toolbar_text_scaling;
    }

    SDL_FColor OpaqueUnormColorToOpaqueFloatColor(SDL_Color c)
    {
        SDL_FColor r;
        r.a = 1.0f;
        r.r = static_cast<float>(c.r) / 255.0f;
        r.g = static_cast<float>(c.g) / 255.0f;
        r.b = static_cast<float>(c.b) / 255.0f;
        return r;
    }
}