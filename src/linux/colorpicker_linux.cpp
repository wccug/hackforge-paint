#include "../colorpicker.hpp"

#include <random>

namespace hackforge {

SDL_Color OpenNativeColorPicker(SDL_Window *window, SDL_Color currentColor) {
  static std::random_device rd{};
  static std::mt19937 engine(rd());
  std::uniform_int_distribution<std::uint8_t> dist{0, 255};
  auto sample = [&]() { return dist(engine); };

  return {.r = sample(), .g = sample(), .b = sample(), .a = 255};
}

} // namespace hackforge
