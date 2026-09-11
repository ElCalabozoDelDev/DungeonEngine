# Custom vcpkg triplet: x64, mingw-w64 ABI, built with the LLVM-MinGW
# toolchain (clang/clang++/lld) instead of GCC or MSVC. No Visual Studio,
# no Microsoft Windows SDK involved.

set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE dynamic)
set(VCPKG_ENV_PASSTHROUGH PATH)

set(VCPKG_CMAKE_SYSTEM_NAME MinGW)
set(VCPKG_POLICY_DLLS_WITHOUT_LIBS enabled)

set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE "${CMAKE_CURRENT_LIST_DIR}/../llvm-mingw.cmake")
