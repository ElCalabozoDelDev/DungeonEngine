# CMake toolchain file that points the build at the LLVM-MinGW toolchain
# (clang/clang++ targeting x86_64-w64-windows-gnu via mingw-w64), so nothing
# from Visual Studio / the MSVC ABI / the Microsoft Windows SDK is required.
#
# Install with:
#   winget install --id MartinStorsjo.LLVM-MinGW.UCRT
#
# If installed elsewhere, override with:
#   cmake --preset default -DLLVM_MINGW_ROOT="C:/path/to/llvm-mingw-..."
# or by setting the LLVM_MINGW_ROOT environment variable before configuring.

if(NOT LLVM_MINGW_ROOT)
    if(DEFINED ENV{LLVM_MINGW_ROOT})
        set(LLVM_MINGW_ROOT "$ENV{LLVM_MINGW_ROOT}")
    else()
        # Discover the default winget install location. The package folder name
        # carries the release date (llvm-mingw-<date>-ucrt-x86_64), so glob for
        # it and take the newest match rather than pinning one version.
        file(TO_CMAKE_PATH "$ENV{LOCALAPPDATA}" _llvm_mingw_localappdata)
        file(GLOB _llvm_mingw_candidates
            "${_llvm_mingw_localappdata}/Microsoft/WinGet/Packages/MartinStorsjo.LLVM-MinGW.UCRT_*/llvm-mingw-*-ucrt-x86_64"
        )
        if(_llvm_mingw_candidates)
            list(SORT _llvm_mingw_candidates)
            list(GET _llvm_mingw_candidates -1 LLVM_MINGW_ROOT)
        endif()
        unset(_llvm_mingw_localappdata)
        unset(_llvm_mingw_candidates)
    endif()
endif()
set(LLVM_MINGW_ROOT "${LLVM_MINGW_ROOT}" CACHE PATH "Root of the LLVM-MinGW (mingw-w64) toolchain")

if(NOT EXISTS "${LLVM_MINGW_ROOT}/bin/x86_64-w64-mingw32-clang++.exe")
    message(FATAL_ERROR
        "LLVM-MinGW toolchain not found at '${LLVM_MINGW_ROOT}'. "
        "Install it with: winget install --id MartinStorsjo.LLVM-MinGW.UCRT, "
        "or pass -DLLVM_MINGW_ROOT=<path> pointing at the extracted llvm-mingw-*-ucrt-x86_64 folder."
    )
endif()

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(CMAKE_C_COMPILER   "${LLVM_MINGW_ROOT}/bin/x86_64-w64-mingw32-clang.exe"   CACHE FILEPATH "" FORCE)
set(CMAKE_CXX_COMPILER "${LLVM_MINGW_ROOT}/bin/x86_64-w64-mingw32-clang++.exe" CACHE FILEPATH "" FORCE)
set(CMAKE_RC_COMPILER  "${LLVM_MINGW_ROOT}/bin/x86_64-w64-mingw32-windres.exe" CACHE FILEPATH "" FORCE)
set(CMAKE_AR           "${LLVM_MINGW_ROOT}/bin/llvm-ar.exe" CACHE FILEPATH "" FORCE)
set(CMAKE_RANLIB       "${LLVM_MINGW_ROOT}/bin/llvm-ranlib.exe" CACHE FILEPATH "" FORCE)

set(CMAKE_C_COMPILER_TARGET   x86_64-w64-windows-gnu)
set(CMAKE_CXX_COMPILER_TARGET x86_64-w64-windows-gnu)

# Host and target OS are both Windows here, so this isn't a real cross-compile.
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    set(CMAKE_CROSSCOMPILING OFF CACHE BOOL "" FORCE)
endif()
