#pragma once

#include <SDL3/SDL_events.h>

namespace hackforge {
    namespace internal {
        class Pointer;
    }
}

class hackforge::internal::Pointer {
    public:
        Pointer();
        void handleEvent(SDL_Event* event);
        float getLastX();
        float getLastY();
        bool isLeftButtonPressed();
    private:
        float lastX;
        float lastY;
        bool leftButtonPressed;
};
