#pragma once

#include <SDL3/SDL_events.h>
#include "Interactable.h"

namespace hackforge {
    namespace internal {
        class Pointer;
    }
}

class hackforge::internal::Pointer : public hackforge::internal::Interactable {
    public:
        Pointer();
        void handleEvent(SDL_Event* event) override;
        float getLastX();
        float getLastY();
        bool isLeftButtonPressed();
    private:
        float lastX;
        float lastY;
        bool leftButtonPressed;
};
