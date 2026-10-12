#include "actions.hpp"
#include "common.hpp"

/*
 * Actions should be:
 * 1) Tied to user intent. EG saving a file is an user action. Copying textures to a buffer is an internal utility
 * 2) UI agnostic. EG saving a file should not care whether it is being done via Toolbar or UI
 * 3) Atomic. IE calling any one function in this file should create a new, valid state for the user
 */

void SaveCanvasToFile(const char* path) {
  SDL_SetRenderTarget(hackforge::renderer, hackforge::canvas);
  SDL_Surface *surface = SDL_RenderReadPixels(hackforge::renderer, nullptr);
  SDL_SavePNG(surface, path);
  SDL_DestroySurface(surface);
}
