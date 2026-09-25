#include <SDL3/SDL.h>
#include <memory>

#include "Toolbar.hpp"
#include "AnglePen.hpp"
#include "uilayout.hpp"
#include "dialogbox.hpp"

namespace hackforge {

inline SDL_Window *window = nullptr;
inline SDL_Renderer *renderer = nullptr;
inline SDL_Texture *canvas = nullptr;
inline SDL_Texture *screenDoor = nullptr;

inline int window_width = sc_default_canvas_width;
inline int window_height = sc_default_canvas_height;
inline float currentPenX = 0;
inline float currentPenY = 0;
inline float previousPenX = 0;
inline float previousPenY = 0;
inline bool penDown = false;
inline bool shouldExit = false;
inline bool shouldClear = false;
inline bool doSave = false;
inline bool doHorizontalFlip = false;
inline bool doVerticalFlip = false;

// --- State variables for the Custom Tooling colors ---
inline SDL_Color penColor = {255, 255, 255, 255}; // Default Pen: White
inline SDL_Color buttonColor = {100, 100, 100,
                                255}; // Default Button Background: Dark Gray

inline Toolbar toolbar;
inline Tool currentTool;
inline AnglePen anglePen;
inline std::unique_ptr<SetCanvasSizeDialogBox> resizeDialog;

inline void NewDocument()
{
    hackforge::shouldClear = true;
}

} // namespace hackforge
