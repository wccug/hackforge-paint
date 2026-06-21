// Author Mackenzie Stewart
// Color Picker for Windows and Linux
// ============================================================================
// WINDOWS PLATFORM NATIVE CALL
// ============================================================================
#if defined(_WIN32) || defined(WIN32)

#include "color_picker.hpp"

#include <windows.h>
#include <commdlg.h>

#pragma comment(lib, "comdlg32.lib")

namespace hackforge {

SDL_Color OpenNativeColorPicker(SDL_Window* window, SDL_Color currentColor) {
    CHOOSECOLOR cc;
    static COLORREF acrCustClr[16];

    HWND hwndOwner = (HWND)SDL_GetPointerProperty(
        SDL_GetWindowProperties(window),
        SDL_PROP_WINDOW_WIN32_HWND_POINTER,
        NULL
    );

    ZeroMemory(&cc, sizeof(cc));
    cc.lStructSize = sizeof(cc);
    cc.hwndOwner = hwndOwner;
    cc.lpCustColors = (LPDWORD)acrCustClr;
    cc.rgbResult = RGB(currentColor.r, currentColor.g, currentColor.b);
    cc.Flags = CC_FULLOPEN | CC_RGBINIT | CC_ANYCOLOR;

    if (ChooseColor(&cc) == TRUE) {
        return SDL_Color{ GetRValue(cc.rgbResult), GetGValue(cc.rgbResult), GetBValue(cc.rgbResult), 255 };
    }
    return currentColor;
}

} // namespace hackforge

#endif
