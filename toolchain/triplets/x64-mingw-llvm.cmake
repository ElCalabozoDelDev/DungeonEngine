# Custom vcpkg triplet: x64, mingw-w64 ABI, built with the LLVM-MinGW
# toolchain (clang/clang++/lld) instead of GCC or MSVC. No Visual Studio,
# no Microsoft Windows SDK involved.

set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE dynamic)
# vcpkg sanitises the environment of the subprocesses it runs -- port builds
# and its own compiler detection -- keeping a default set plus whatever is
# listed here. LOCALAPPDATA is in that default set, which is why the toolchain
# file's winget glob works without help; LLVM_MINGW_ROOT is not, so without it
# here an install in a custom location is invisible to vcpkg and detection
# fails with "toolchain not found at ''".
set(VCPKG_ENV_PASSTHROUGH "PATH;LLVM_MINGW_ROOT")

set(VCPKG_CMAKE_SYSTEM_NAME MinGW)
set(VCPKG_POLICY_DLLS_WITHOUT_LIBS enabled)

set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE "${CMAKE_CURRENT_LIST_DIR}/../llvm-mingw.cmake")
