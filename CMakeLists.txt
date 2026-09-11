# Floor is driven by what this build actually uses: presets v2 (3.21),
# CMAKE_CXX_STANDARD 23 (3.20) and `cmake -E copy_directory_if_different`
# (3.26) in the asset-copy step below.
cmake_minimum_required(VERSION 3.26)

# The compiler (clang/clang++ from LLVM-MinGW) is set by
# toolchain/llvm-mingw.cmake, chainloaded from the x64-mingw-llvm vcpkg
# triplet (see CMakePresets.json). It is deliberately not set here so it
# does not override what vcpkg.cmake already configured.
#
# CMAKE_PREFIX_PATH is likewise not set: the vcpkg toolchain resolves
# installed packages on its own. Hardcoding it pins both the build
# directory and the triplet, which breaks any other -B path or triplet.

# Project name
project(DungeonEngine VERSION 1.0 LANGUAGES CXX)

# CMake configuration
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Recursively collect the .cpp files under game/src.
# CONFIGURE_DEPENDS makes the build re-run the glob when files are added or
# removed. This is a stopgap: explicit source lists land with the
# engine/game/third_party target split.
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS game/src/*.cpp)

# Put the executable in build/bin
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

# Create the executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Expose the third_party directory to the include path
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/third_party)

# Same for the vendored third_party sources (imgui backends, base64).
file(GLOB_RECURSE THIRD_PARTY_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/third_party/*.cpp)

# Add the third_party implementation files to the target
target_sources(${PROJECT_NAME} PRIVATE ${THIRD_PARTY_SOURCES})

# Find the dependencies provided by vcpkg
find_package(SDL2 CONFIG REQUIRED)
find_package(sdl2_image CONFIG REQUIRED)
find_package(sdl2_mixer CONFIG REQUIRED)
find_package(entt CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
# ZLIB is used directly (uncompress() on TMX tile data in tmx_loader.cpp).
# libpng is NOT: SDL2_image pulls it in and owns the PNG decoding, so there
# is no find_package(PNG) / PNG::PNG here.
find_package(ZLIB REQUIRED)
find_package(tinyxml2 CONFIG REQUIRED)

# Link the dependencies.
# Note: SDL2::SDL2main is deliberately NOT linked, because main.cpp defines
# SDL_MAIN_HANDLED and provides its own main() instead of SDL2main's
# WinMain -> SDL_main shim. Linking it forces the WinMain symbol, which in
# turn requires SDL_main and breaks the link step.
target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL2_image::SDL2_image)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL2_mixer::SDL2_mixer)
target_link_libraries(${PROJECT_NAME} PRIVATE imgui::imgui)
target_link_libraries(${PROJECT_NAME} PRIVATE ZLIB::ZLIB)
target_link_libraries(${PROJECT_NAME} PRIVATE tinyxml2::tinyxml2)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_SOURCE_DIR}/game/include
)

# Copy SDL2.dll and the other required runtime DLLs next to the executable
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}>
    $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMAND_EXPAND_LISTS
)

# Mirror assets next to the build. copy_directory_if_different only touches
# files whose contents changed, so this is cheap on repeat builds and does
# not wipe the destination the way remove_directory + copy_directory did.
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
    ${CMAKE_SOURCE_DIR}/assets
    ${CMAKE_BINARY_DIR}/assets
)