#include "Pointer.h"

hackforge::internal::Pointer::Pointer() {
    lastX = 0.f;
    lastY = 0.f;
    leftButtonPressed = false;
}

void hackforge::internal::Pointer::handleEvent(SDL_Event* event) {
    if (event->type == SDL_EVENT_MOUSE_MOTION)
    {
        lastX = event->motion.x;
        lastY = event->motion.y;
    }
    else if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN)
    {
        leftButtonPressed = true;
    }
    else if (event->type == SDL_EVENT_MOUSE_BUTTON_UP)
    {
        leftButtonPressed = false;
    }
}

float hackforge::internal::Pointer::getLastX() {
    return lastX;
}

float hackforge::internal::Pointer::getLastY() {
    return lastY;
}

bool hackforge::internal::Pointer::isLeftButtonPressed() {
    return leftButtonPressed;
}
