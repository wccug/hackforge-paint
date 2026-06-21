// Author Mackenzie Stewart
// Color Picker for Windows and Linux

#if defined(__linux__)

#include "color_picker.hpp"

namespace hackforge {

// For now, leave this the same as the generic unimplemented version.
SDL_Color OpenNativeColorPicker(SDL_Window* window, SDL_Color currentColor) {
    return currentColor;
}

} // namespace hackforge

#endif
