# Floor is driven by what this build actually uses: presets v2 (3.21),
# CMAKE_CXX_STANDARD 23 (3.20) and `cmake -E copy_directory_if_different`
# (3.26) in the asset-copy step.
cmake_minimum_required(VERSION 3.26)

# The compiler (clang/clang++ from LLVM-MinGW) is set by
# toolchain/llvm-mingw.cmake, chainloaded from the x64-mingw-llvm vcpkg
# triplet (see CMakePresets.json). It is deliberately not set here so it
# does not override what vcpkg.cmake already configured.
#
# CMAKE_PREFIX_PATH is likewise not set: the vcpkg toolchain resolves
# installed packages on its own. Hardcoding it pins both the build
# directory and the triplet, which breaks any other -B path or triplet.

project(DungeonEngine VERSION 1.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find the dependencies provided by vcpkg. ZLIB is used directly
# (uncompress() on TMX tile data); libpng is not — SDL2_image pulls it in and
# owns PNG decoding, so there is no find_package(PNG) here.
find_package(SDL2 CONFIG REQUIRED)
find_package(sdl2_image CONFIG REQUIRED)
find_package(sdl2_mixer CONFIG REQUIRED)
find_package(entt CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(tinyxml2 CONFIG REQUIRED)

# Three targets, in dependency order. The layering is the point:
#
#   third_party  vendored sources, compiled with warnings suppressed
#   engine       reusable 2D engine; knows nothing about this game
#   game         the example game; links engine
#
# `engine` deliberately does not get game/include on its include path, so an
# #include <game/...> from engine code fails to compile.
add_subdirectory(third_party)
add_subdirectory(engine)
add_subdirectory(game)
