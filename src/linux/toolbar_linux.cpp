#include <cstdio>
#include <cstdlib>
#include <dlfcn.h>
#include <string>
#include <unistd.h>
#include <SDL3/SDL_filesystem.h>
#include <limits.h>
#include "../actions.hpp"

namespace hackforge {

  // dylib function definitions
  typedef char* (*fn_save_as)();
  typedef void (*fn_drop_rstring)(char*);
  
  void OnToolbarSaveAs() {

    // Build the path to our dylib, relative to the executable
    // TODO: will eventually be better to load the dylib once in a more central way
    const char* lib_name = "libxdg_portal_wrapper.so";
    char lib_path[PATH_MAX] = "";
    const char * base_path = SDL_GetBasePath();
    size_t remaining = sizeof(lib_path) - strlen(base_path) - 1;
    strncat(lib_path, base_path, remaining);
    remaining = sizeof(lib_path) - strlen(lib_name) - 1;
    strncat(lib_path, lib_name, remaining);

    // Load the dylib
    void* handler{};
    handler = dlopen(lib_path, RTLD_LAZY);
    if (!handler) {
      fprintf(stderr, "Failed to save: xdg portal wrapper not found at path: %s\n!", lib_path);
      return;
    }

    // load our functions
    fn_save_as save_as = reinterpret_cast<fn_save_as>(dlsym(handler, "save_as"));
    if (!save_as) {
      fprintf(stderr, "Failed to save: dynamic `save_as()` symbol not found! (Is your dylib up to date?)\n");
      return;
    }
    fn_drop_rstring drop_rstring = reinterpret_cast<fn_drop_rstring>(dlsym(handler, "drop_rstring"));
    if (!drop_rstring) {
      fprintf(stderr, "Failed to save: dynamic `drop_rstring()` symbol not found! (Is your dylib up to date?)\n");
      return;
    }

    // open save dialog    
    char* path = save_as();
    if (strcmp(path, "") != 0) {
      SaveCanvasToFile(path);
    } else {
      // user cancelled the save dialogue
    }

    // clean up
    drop_rstring(path);
    dlclose(handler);
  }

} // namespace hackforge
