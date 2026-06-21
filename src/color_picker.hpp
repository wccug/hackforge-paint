#pragma once

#include <SDL3/SDL.h>

namespace hackforge {

/**
 * @brief Opens the OS-native color picker dialog (Win32 on Windows, XDG Portal on Linux).
 * @param window Pointer to the active SDL_Window context.
 * @param currentColor The initial color selection when the dialog loads.
 * @return The chosen SDL_Color, or currentColor if canceled.
 */
SDL_Color OpenNativeColorPicker(SDL_Window* window, SDL_Color currentColor);

} // namespace hackforge
