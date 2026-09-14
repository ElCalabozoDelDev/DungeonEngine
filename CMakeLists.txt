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

# Find the dependencies provided by vcpkg. libpng is not listed — SDL2_image
# pulls it in and owns PNG decoding, so there is no find_package(PNG) here.
find_package(SDL2 CONFIG REQUIRED)
find_package(sdl2_image CONFIG REQUIRED)
find_package(sdl2_mixer CONFIG REQUIRED)
find_package(entt CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

# LLVM-MinGW links libc++ and libunwind dynamically, so anything it builds
# needs those DLLs beside it. $<TARGET_RUNTIME_DLLS:...> only covers imported
# targets -- the vcpkg ones -- not the compiler's own runtime, so a binary that
# ran fine on a machine with the toolchain on PATH failed with 0xc0000135
# (DLL not found) anywhere else. CI hit it first, because the test executable
# is the only one run during the build.
set(DE_TOOLCHAIN_RUNTIME_DLLS "")
foreach(_dll libc++.dll libunwind.dll libwinpthread-1.dll)
    if(EXISTS "${LLVM_MINGW_ROOT}/bin/${_dll}")
        list(APPEND DE_TOOLCHAIN_RUNTIME_DLLS "${LLVM_MINGW_ROOT}/bin/${_dll}")
    endif()
endforeach()

# Puts every DLL an executable needs next to it, so the build output is
# self-contained: both the vcpkg dependencies and the compiler runtime.
function(de_copy_runtime_dlls target)
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_RUNTIME_DLLS:${target}>
        $<TARGET_FILE_DIR:${target}>
        COMMAND_EXPAND_LISTS
    )
    if(DE_TOOLCHAIN_RUNTIME_DLLS)
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${DE_TOOLCHAIN_RUNTIME_DLLS}
            $<TARGET_FILE_DIR:${target}>
        )
    endif()
endfunction()

# Warnings for first-party code only; third_party is compiled without them.
# -Werror is opt-in so a newer compiler cannot break a consumer build, and CI
# turns it on so no new warning lands.
option(DE_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

function(de_enable_warnings target)
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    if(DE_WARNINGS_AS_ERRORS)
        target_compile_options(${target} PRIVATE -Werror)
    endif()
endfunction()

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

# Tests are on by default; turn them off for a consumer build that only wants
# the game binary.
option(DE_BUILD_TESTS "Build the unit tests" ON)
if(DE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
