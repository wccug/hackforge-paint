#pragma once

#include <SDL3/SDL_events.h>

namespace hackforge {
    namespace internal {
        class Interactable;
    }
}

class hackforge::internal::Interactable {
    public:
        virtual void handleEvent(SDL_Event* event);
};
