#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "Toolbar.hpp"
#include "AnglePen.hpp"
#include "LayoutConstants.hpp"

namespace hackforge {

inline SDL_Window *window = nullptr;
inline SDL_Renderer *renderer = nullptr;
inline SDL_Texture *canvas = nullptr;

inline float currentPenX = 0;
inline float currentPenY = 0;
inline float previousPenX = 0;
inline float previousPenY = 0;
inline bool penDown = false;
inline bool shouldExit = false;
inline bool shouldClear = false;

// --- State variables for the Custom Tooling colors ---
inline SDL_Color penColor = {255, 255, 255, 255}; // Default Pen: White
inline SDL_Color buttonColor = {100, 100, 100,
                                255}; // Default Button Background: Dark Gray

inline Toolbar toolbar;
inline Tool currentTool;
inline AnglePen anglePen;

inline void NewDocument()
{
    hackforge::shouldClear = true;
}

} // namespace hackforge
