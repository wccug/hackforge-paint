#include <Windows.h>
#include <Shlobj.h>

#include "common.hpp"
#include "colorpicker.hpp"

namespace hackforge {

void OnToolbarNew() {
  NewDocument();
}

void OnToolbarSaveAs() {
  SDL_PropertiesID props = SDL_GetWindowProperties(hackforge::window);
  void *hWndData =
      SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
  HWND hWnd = reinterpret_cast<HWND>(hWndData);

  char documentsPath[MAX_PATH];

  if (FAILED((SHGetFolderPathA(NULL, CSIDL_PERSONAL | CSIDL_FLAG_CREATE, NULL,
                               0, documentsPath)))) {
    return;
  }

  OPENFILENAMEA ofn;
  ZeroMemory(&ofn, sizeof(ofn));

  char szFile[MAX_PATH];
  strcpy_s(szFile, "image.png");

  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = hWnd;
  ofn.lpstrFile = szFile;
  ofn.nMaxFile = sizeof(szFile);
  ofn.lpstrFilter = "PNG File\0";
  ofn.nFilterIndex = 1;
  ofn.lpstrFileTitle = NULL;
  ofn.nMaxFileTitle = 0;
  ofn.lpstrInitialDir = documentsPath;
  ofn.lpstrDefExt = "png";
  ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

  if (GetSaveFileNameA(&ofn) == 0)
    return;

  std::string destFilename = ofn.lpstrFile;

  // In SDL, textures are GPU-side, surfaces are CPU-side.
  // We have to copy the GPU-side texture to CPU to save to disk.
  SDL_SetRenderTarget(hackforge::renderer, hackforge::canvas);
  SDL_Rect rect{};
  rect.w = hackforge::window_width;
  rect.h = hackforge::window_height;
  SDL_Surface *surface = SDL_RenderReadPixels(hackforge::renderer, &rect);
  SDL_CreateSurfaceFrom(hackforge::window_width, hackforge::window_height,
                        hackforge::canvas->format, nullptr, 0);
  SDL_SavePNG(surface, destFilename.c_str());
  SDL_DestroySurface(surface);
}

void OnToolbarExit() {
  hackforge::shouldExit = true;
}

void OnToolbarSetPenColor() {
  hackforge::penColor =
      hackforge::OpenNativeColorPicker(hackforge::window, hackforge::penColor);
}

void OnToolbarSetUIColor() {
  hackforge::buttonColor = hackforge::OpenNativeColorPicker(
      hackforge::window, hackforge::buttonColor);
}

void OnToolbarSetStampTool() {
  hackforge::currentTool = Tool::Stamp;
  hackforge::toolbar.SetChildMenuItemCheckedState(1, 0, true);
  hackforge::toolbar.SetChildMenuItemCheckedState(1, 1, false);
}

void OnToolbarSetAnglePenTool() {
  hackforge::currentTool = Tool::AnglePen;
  hackforge::toolbar.SetChildMenuItemCheckedState(1, 0, false);
  hackforge::toolbar.SetChildMenuItemCheckedState(1, 1, true);
}

} // namespace hackforge
