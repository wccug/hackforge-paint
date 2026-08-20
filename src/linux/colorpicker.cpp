#include "../colorpicker.hpp"

namespace hackforge {

SDL_Color OpenNativeColorPicker(SDL_Window *window, SDL_Color currentColor) {
  return {.r = 0, .g = 255, .b = 0, .a = 255};
}

} // namespace hackforge
