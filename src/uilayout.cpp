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
}