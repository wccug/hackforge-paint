#pragma once

#include <SDL3/SDL.h>

namespace hackforge {
    static constexpr int window_width = 800;
    static constexpr int window_height = 600;
    static constexpr int toolbar_height = 32;
    static constexpr int toolbar_margin = 16;
    static constexpr int toolbar_line_height = 32;
    static constexpr float toolbar_top_level_menu_horizontal_spacing = 32;
    static constexpr float toolbar_text_scaling = 2;

    bool IsInBounds(float x, float y, SDL_FRect const& bounds);

} // namespace hackforge