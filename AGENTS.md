# AGENTS.md

Shared guidance for any coding agent working in this repository
(Claude Code, Cursor, GitHub Copilot, and others).

Claude-specific wiring (hooks, subagent frontmatter, slash-command
compat) lives under `.claude/`. Portable skills live in `.claude/skills/`
— Cursor and Copilot also load that path. Prefer invoking a skill over
re-deriving its procedure.

## Build

C++23, built with CMake + Ninja, using Clang via the LLVM-MinGW toolchain
(target `x86_64-w64-windows-gnu`, no MSVC involved) and vcpkg for
dependencies.

```
cmake --preset default -B build
ninja -C build
```

Requires:
- `VCPKG_ROOT` env var set (the preset's toolchain file resolves via
  `$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake`).
- LLVM-MinGW installed
  (`winget install --id MartinStorsjo.LLVM-MinGW.UCRT`). If installed to
  a non-default path, set `LLVM_MINGW_ROOT`.

`toolchain/` is tracked and `CMakePresets.json` depends on it
(`toolchain/llvm-mingw.cmake` toolchain file,
`toolchain/triplets/x64-mingw-llvm.cmake` vcpkg triplet). The toolchain
file globs `%LOCALAPPDATA%` for the default winget install; override with
`LLVM_MINGW_ROOT` if LLVM-MinGW lives elsewhere.

Tests are doctest + CTest in `tests/`, built by default
(`-DDE_BUILD_TESTS=OFF` to skip):

```
ctest --test-dir build --output-on-failure
```

They link `engine` and `game_lib` directly, so they need no window, input
or desktop session — run them rather than trying to drive the built game,
which needs a foreground window and has repeatedly proved unreliable to
automate. Three smoke tests run the real binary under SDL's dummy drivers
(`--frames N`, `--level`, a short `--sim`). `.github/workflows/ci.yml`
runs configure with `-DDE_WARNINGS_AS_ERRORS=ON`, build, ctest, an
advisory clang-tidy pass and a clang-format check on a fresh checkout.
First-party targets build with `-Wall -Wextra -Wpedantic`; add
`de_enable_warnings(<target>)` to any new one.

## Architecture

CMake targets, in dependency order:

| Target | Contents |
|---|---|
| `third_party` | Vendored sources (imgui SDL2 backends, imgui EnTT entity editor). Headers exposed as `SYSTEM` so its warnings don't leak. |
| `engine` | The reusable 2D engine: game loop, plugin/hook system, SDL + ImGui integration, texture manager, Tiled JSON (`.tmj`) loading, rendering, camera, quadtree, scenes, widget layer. |
| `game_lib` | The example game (Dungeon Slime) as a library: snake/bat systems, scenes, HUD. |
| `game` | Just `main()`. Links `game_lib`; binary is named `DungeonEngine`. |
| `tests` | doctest unit tests, linking `game_lib`. |

**The engine must never reference the game.** `engine` does not have
`game/include` on its include path, so `#include <game/...>` from engine
code is a compile error — that's the enforcement mechanism, not
convention. When something in the engine needs game knowledge, invert it:
the Tiled loader records Tiled's `type` string in `de::ObjectTypeComponent`
and `InGameScene` turns `"Player"` into the slime snake head.

Splitting rule for new code: if it names a gameplay component, it belongs
in `game/`.

- Engine code lives in `namespace de`; game code is unnamespaced. Game
  `.cpp` files may use `using namespace de;` — headers may not.
- Includes are angle-bracketed and prefixed:
  `#include <engine/core/game_loop.hpp>`,
  `#include <game/components/player_component.hpp>`.
- Header guards follow the path: `DE_CORE_GAME_LOOP_HPP`,
  `GAME_SCENE_IN_GAME_SCENE_HPP`.
- Source lists in the `CMakeLists.txt` files are explicit — add new files
  there.
- Systems that must not depend on frame rate go in `addFixedSystem`; the
  rest in `addSystem`, rendering in `addSystemLast`. `addFixedSystemLast`
  is for per-step bookkeeping after all movement (spatial index sync).
- Game-wide context resources (`GameState`, `AudioSettings`, `GameRng`,
  `de::Paused`, `de::WorldColorGrade`) are installed once, in
  `GamePlugin`'s setup. Everything else reads them with `ctx().get` —
  never `if (!contains) emplace`, which hides a missing resource.
- `GameState::playState` is the truth about pause and game over;
  `de::Paused` is derived from it. Change both only through
  `setPlayState()`. Fixed systems do not check for pause: the loop does
  not step them while `Paused` is set.
- Scenes hand the entities they create to `Scene::track()`; `SceneSystem`
  destroys them after `onExit()`. `TiledLoader::load` returns every entity
  it created in `LoadedLevel::entities` for exactly that.
- Setup callbacks do not check for `StartupError`: `GameLoop` stops calling
  them after the first one that reports it. Teardown callbacks still run
  and must cope with a partial setup.
- `BasePlugin` provides `SceneSystem` and `DebugSystem` (context references);
  mount game plugins after it.
- Vector maths (`lerp`, `reflect`, `Circle`, `Vector2D::dot`) lives in
  `engine/core/math.hpp`; do not re-declare it in game code.

## Design documents

`docs/design/` is the source of truth for *game design* — pillars, core
loop, systems, levels, balance, ADRs. It is written in **Spanish**; code,
comments and agent tooling stay in English. This file and the READMEs
remain the technical reference; design docs never document the engine.
The repository is a template, and **Dungeon Slime** (grid snake) is
only its worked example; config is `assets/game.json`, maps are `.tmj`.

**New-project mode.** When `docs/design/` is empty or missing, or none of
its documents is `implementado` or `parcial`, the code, levels, tests,
READMEs and git history describe the example, not the game the user is
designing. Treat design questions as a blank slate: do not research the
repository or `git log` first, and delegate to the `gdd-architect`
subagent (Claude Code) with the user's request passed through as-is — no
summary of the existing code. The subagent enforces this with a hook
(`.claude/hooks/greenfield-guard.mjs`).

Each document declares an `estado` (`implementado`, `parcial`,
`propuesto`, `descartado`) and a `codigo:` list of the files that
implement it. Read the `estado` before trusting a document: only the
first two describe the game that exists.

**If you change a gameplay system, update its document in the same
change.** Claude Code's `Stop` hook names the document that fell behind;
the `/gdd-sync` skill reports drift across the whole tree.

Tuning numbers live in three places at once — the component default,
`tests/test_balance.cpp`, and `docs/design/50-balance/tablas.md`. The
test exists to make skipping the other two impossible.

## Simulation mode

`--sim` runs the game headless on a synthetic clock with a scripted
navigation policy, writing one CSV row per fixed step plus a summary on
stdout. It is how balance is measured instead of guessed:

```
DungeonEngine --sim --sim-out sim.csv
```

Two runs with the same flags must produce **byte-identical CSVs**; that
diff is the acceptance test for the whole feature. The policy always
turns right and dodges nothing, so its numbers (`score`, `length`,
`steps_alive`) are a floor on difficulty, never a verdict on feel.

`sim_golden_seed*` in ctest runs that diff for you and also compares the
CSV with `tests/golden/seed<N>.csv`. A pure refactor must keep it green
without touching the reference. A change that alters gameplay on purpose
regenerates the reference in the same commit (command in the header of
`tests/sim_golden.cmake`) — never regenerate it just to make a refactor
pass.

## Code style

`.clang-format` is LLVM-based with overrides (4-space indent, 80-col
limit, **Allman braces**, left-aligned pointers) — run it, don't
hand-format. On top of that, conventions not enforced by clang-format:
- Header guards: classic `#ifndef FOO_HPP` / `#define` / `#endif`, not
  `#pragma once`.
- Private members prefixed `m_` (e.g. `m_registry`).
- Methods/functions `camelCase`, classes `PascalCase`.
- Builder-style setup methods (`addPlugin`, `addSystem`, etc.) return
  `*this` by reference for chaining.
- Comments and identifiers in English.

`main.cpp` deliberately does not link `SDL2::SDL2main` — it defines
`SDL_MAIN_HANDLED` and its own `main()`. Don't add `SDL2::SDL2main` to
the link step; it will force a `WinMain` symbol and break linking.

## Repo etiquette

Work happens on feature-named branches (e.g. `camera`, `qtree`), merged
into `main` via PR.
