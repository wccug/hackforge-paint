#pragma once

#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_render.h>

#include "../Pointer.h"

// Is this going too far ?? Makes for a lovely pyramid tho
namespace hackforge {
    namespace internal {
        namespace ui {
            class Button;
        }
    }
}

class hackforge::internal::ui::Button {
    public:
        Button(float x, float y, int width, int height);
        ~Button();
        int initialize(SDL_Renderer* renderer);
        int update(hackforge::internal::Pointer* pointer);
        int draw(SDL_Renderer* renderer);
    private:
        SDL_Texture* texture;
        SDL_Surface* surface;
        float x;
        float y;
        int width;
        int height;
        bool isPressed;

        int createSurface();
        int createTexture(SDL_Renderer* renderer);
};
