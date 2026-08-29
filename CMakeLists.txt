# This can likely be bumped higher, but we're not doing anything
# fancy.

cmake_minimum_required(VERSION 3.10)

# Declare our project.

project(hackforge-paint LANGUAGES CXX)

# Add an executable to our project (use the same name because why
# not).  List our source files.

# TODO: Change this once cmake can do windows as well.
set(BUILD_TARGET_LINUX "True")

add_executable(hackforge-paint
   src/main.cpp
   src/toolbar.cpp
   src/bucket.cpp
   $<$<BOOL:${BUILD_TARGET_LINUX}>:src/linux/colorpicker_linux.cpp>
   $<$<BOOL:${BUILD_TARGET_LINUX}>:src/linux/toolbar_linux.cpp>
)

# Find and link in the SDL3 library. For debian-based Linux
# distributions, execute `sudo apt install libsdl3-dev` For windows
# builds, see the accompanying MSBuild file and instructions.

find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)
target_link_libraries(hackforge-paint SDL3::SDL3)

# The Linux native color picker talks to the XDG desktop portal over
# D-Bus, so link libdbus-1 on Linux. Install `libdbus-1-dev` if
# pkg-config can't find it.
