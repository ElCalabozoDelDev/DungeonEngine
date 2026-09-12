# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build

C++23, built with CMake + Ninja, using Clang via the LLVM-MinGW toolchain (target `x86_64-w64-windows-gnu`, no MSVC involved) and vcpkg for dependencies.

```
cmake --preset default -B build
ninja -C build
```

Requires:
- `VCPKG_ROOT` env var set (the preset's toolchain file resolves via `$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake`).
- LLVM-MinGW installed (`winget install --id MartinStorsjo.LLVM-MinGW.UCRT`). If installed to a non-default path, set `LLVM_MINGW_ROOT`.

`toolchain/` is tracked and `CMakePresets.json` depends on it (`toolchain/llvm-mingw.cmake` toolchain file, `toolchain/triplets/x64-mingw-llvm.cmake` vcpkg triplet). The toolchain file globs `%LOCALAPPDATA%` for the default winget install; override with `LLVM_MINGW_ROOT` if LLVM-MinGW lives elsewhere.

Tests are doctest + CTest in `tests/`, built by default (`-DDE_BUILD_TESTS=OFF` to skip):

```
ctest --test-dir build --output-on-failure
```

They link `engine` and `game_lib` directly, so they need no window, input or desktop session — run them rather than trying to drive the built game, which needs a foreground window and has repeatedly proved unreliable to automate. Two smoke tests run the real binary under SDL's dummy drivers (`--frames N`, `--level`). `.github/workflows/ci.yml` runs configure, build, ctest and a clang-format check on a fresh checkout.

## Architecture

Three CMake targets, in dependency order:

| Target | Contents |
|---|---|
| `third_party` | Vendored sources (imgui SDL2 backends, base64). Headers exposed as `SYSTEM` so its warnings don't leak. |
| `engine` | The reusable 2D engine: game loop, plugin/hook system, SDL + ImGui integration, texture manager, TMX loading, rendering, camera, quadtree, scenes, widget layer. |
| `game_lib` | The example game as a library: components, systems, scenes, HUD. |
| `game` | Just `main()`. Links `game_lib`; binary is named `DungeonEngine`. |
| `tests` | doctest unit tests, linking `game_lib`. |

**The engine must never reference the game.** `engine` does not have `game/include` on its include path, so `#include <game/...>` from engine code is a compile error — that's the enforcement mechanism, not convention. When something in the engine needs game knowledge, invert it: the TMX loader records Tiled's `type` string in `de::ObjectTypeComponent` and `InGameScene::tagObjectsByType` turns `"Player"` into a `PlayerComponent`.

Splitting rule for new code: if it names a gameplay component, it belongs in `game/`.

- Engine code lives in `namespace de`; game code is unnamespaced. Game `.cpp` files may use `using namespace de;` — headers may not.
- Includes are angle-bracketed and prefixed: `#include <engine/core/game_loop.hpp>`, `#include <game/components/player_component.hpp>`.
- Header guards follow the path: `DE_CORE_GAME_LOOP_HPP`, `GAME_SCENE_IN_GAME_SCENE_HPP`.
- Source lists in the `CMakeLists.txt` files are explicit — add new files there.
- Systems that must not depend on frame rate go in `addFixedSystem`; the rest in `addSystem`, rendering in `addSystemLast`.

## Design documents

`docs/design/` is the source of truth for *game design* — pillars, core loop,
systems, biomes, levels, balance, ADRs. It is written in **Spanish**; code,
comments and the `.claude/` tooling stay in English. This file and the READMEs
remain the technical reference; design docs never document the engine.

Each document declares an `estado` (`implementado`, `parcial`, `propuesto`,
`descartado`) and a `codigo:` list of the files that implement it. Read the
`estado` before trusting a document: only the first two describe the game that
exists.

**If you change a gameplay system, update its document in the same change.** A
`Stop` hook names the document that fell behind, and `/gdd-sync` reports drift
across the whole tree.

Tuning numbers live in three places at once — the component default,
`tests/test_balance.cpp`, and `docs/design/50-balance/tablas.md`. The test exists
to make skipping the other two impossible.

## Simulation mode

`--sim` runs the game headless on a synthetic clock with a scripted navigation
policy, writing one CSV row per fixed step plus a summary on stdout. It is how
balance is measured instead of guessed:

```
DungeonEngine --sim --sim-out sim.csv
```

Two runs with the same flags must produce **byte-identical CSVs**; that diff is
the acceptance test for the whole feature. The policy paths perfectly and dodges
nothing, so its numbers are a floor on difficulty, never a verdict on feel.

## Code style

`.clang-format` is LLVM-based with overrides (4-space indent, 80-col limit, **Allman braces**, left-aligned pointers) — run it, don't hand-format. On top of that, conventions not enforced by clang-format:
- Header guards: classic `#ifndef FOO_HPP` / `#define` / `#endif`, not `#pragma once`.
- Private members prefixed `m_` (e.g. `m_registry`).
- Methods/functions `camelCase`, classes `PascalCase`.
- Builder-style setup methods (`addPlugin`, `addSystem`, etc.) return `*this` by reference for chaining.
- Comments and identifiers in English.

`main.cpp` deliberately does not link `SDL2::SDL2main` — it defines `SDL_MAIN_HANDLED` and its own `main()`. Don't add `SDL2::SDL2main` to the link step; it will force a `WinMain` symbol and break linking.

## Repo etiquette

Work happens on feature-named branches (e.g. `camera`, `qtree`), merged into `main` via PR.
