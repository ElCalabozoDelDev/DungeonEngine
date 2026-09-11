# DungeonEngine

A C++23 + SDL2 game engine boilerplate with a small 2D dungeon example.

## Requirements

- Git
- CMake 3.26 or newer
- Ninja
- [vcpkg](https://github.com/microsoft/vcpkg), with `VCPKG_ROOT` set in the environment
- LLVM-MinGW (UCRT, x86_64) — clang targeting `x86_64-w64-windows-gnu`

No Visual Studio, MSVC or Windows SDK is involved. Install the toolchain with:

```bash
winget install --id MartinStorsjo.LLVM-MinGW.UCRT
```

`toolchain/llvm-mingw.cmake` discovers the default winget install location
automatically. If you installed it somewhere else, point at it with the
`LLVM_MINGW_ROOT` environment variable, or pass
`-DLLVM_MINGW_ROOT=C:/path/to/llvm-mingw-...-ucrt-x86_64` when configuring.

## Build

```bash
git clone git@github.com:ElCalabozoDelDev/DungeonEngine.git
cd DungeonEngine
cmake --preset default -B build
ninja -C build
```

The `default` preset already sets the build directory, the vcpkg toolchain and
the `x64-mingw-llvm` triplet — run it from the repository root, not from inside
`build/`. The first configure builds all dependencies through vcpkg and takes a
few minutes; later ones are cached.

## Run

The executable is written to `build/bin/` and the assets are mirrored to
`build/assets/`. Asset paths are currently resolved relative to the working
directory, so launch it from the binary's directory:

```bash
cd build/bin && ./DungeonEngine.exe
```

Arrow keys move the character.

## Layout

```
engine/          reusable 2D engine, built as a static library (namespace de)
  include/engine/  public headers
  src/             implementation
game/            the example game, links engine
  include/game/    headers
  src/             implementation
third_party/     vendored sources: imgui SDL2 backends, base64
assets/          textures, Tiled (.tmx) levels and game.xml configuration
toolchain/       LLVM-MinGW CMake toolchain file and the vcpkg triplet
```

The engine does not have `game/include` on its include path, so it cannot
reference the game even by accident. To reuse it, drop `game/` and write your
own against `engine`.

`assets/game.xml` holds the window, framerate, camera and level settings read at
startup.

## Dependencies

Resolved through vcpkg (see `vcpkg.json`): SDL2, SDL2_image, SDL2_mixer, EnTT,
Dear ImGui, tinyxml2, zlib, libpng.

## License

See [LICENSE](LICENSE).
