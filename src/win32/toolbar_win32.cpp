#include "../common.hpp"
#include "../toolbar.hpp"
#include "../colorpicker.hpp"
#include "../actions.hpp"

#include <Windows.h>
#include <Shlobj.h>

namespace hackforge {

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

  SaveCanvasToFile(destFilename.c_str());
}

} // namespace hackforge
