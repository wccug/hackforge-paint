# This can likely be bumped higher, but we're not doing anything
# fancy.

cmake_minimum_required(VERSION 3.10)

# Declare our project.

project(hackforge-paint LANGUAGES CXX)

# Add an executable to our project (use the same name because why
# not).  List our source files.

add_executable(hackforge-paint src/main.cpp)

# Find and link in the SDL3 library. For debian-based Linux
# distributions, execute `sudo apt install libsdl3-dev` For windows
# builds, see the accompanying MSBuild file and instructions.

find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)
target_link_libraries(hackforge-paint SDL3::SDL3)
