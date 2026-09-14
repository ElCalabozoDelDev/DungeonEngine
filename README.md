# DungeonEngine

[![CI](https://github.com/ElCalabozoDelDev/DungeonEngine/actions/workflows/ci.yml/badge.svg)](https://github.com/ElCalabozoDelDev/DungeonEngine/actions/workflows/ci.yml)

A C++23 + SDL2 boilerplate for 2D games, with **Dungeon Slime** (a grid
snake that eats bats) as the worked example.

*[Versión en español](README.es.md)*

The point of the split is that `engine/` knows nothing about the game. Delete
`game/`, write your own against `engine`, and you have a starting point rather
than someone else's project to unpick.

---

## What you get

- **ECS** on [EnTT](https://github.com/skypjack/entt), with a plugin/hook
  system for composing startup, frame and teardown work.
- **Fixed-timestep loop** — movement and collision advance in fixed steps and
  do not depend on frame rate; rendering follows the frame.
- **Tiled JSON (`.tmj`) loading**: tile layers, external `.tsj` tilesets,
  object layers. Startup config is `assets/game.json` (no XML).
- **Camera** helpers for games that need them; Dungeon Slime draws a fixed
  arena in logical 320×180 scaled to 1280×720 (no follow cam).
- **Quadtree spatial index** per layer, kept correct for moving entities.
- **Input** as named actions with rebindable keys, not scancodes in systems.
- **Dear ImGui** integration, an entity inspector, and a hook-based widget
  layer (`use_state`).
- **Sprite animation** with looping, ping-pong and single-shot runs, a speed
  multiplier and a hold at each end.
- **Audio** through SDL2_mixer.
- **Scenes** with deferred switching, and pausing handled by the loop.
- **Tests** on doctest, runnable headlessly, plus a GitHub Actions workflow
  that builds and tests a plain checkout on Windows.

## Requirements

- Git
- CMake 3.26 or newer
- Ninja
- [vcpkg](https://github.com/microsoft/vcpkg), with `VCPKG_ROOT` set in the
  environment
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

```bash
./build/bin/DungeonEngine.exe
```

Assets are located relative to the executable, so it runs from any working
directory — double-clicking it in the file explorer works too. `build/bin` is
self-contained: the build copies both the vcpkg dependencies and the compiler's
own runtime (`libc++`, `libunwind`) next to the binaries, so the toolchain does
not need to be on `PATH`. Copy that directory somewhere and it still runs; move
the `.exe` out on its own and it will not.

Reach the three coins without letting the skeletons touch you.

| Key | |
|---|---|
| Arrows / WASD | move |
| Enter / Space | confirm (menu) |
| Esc | pause, or quit from the menu |
| F5 | reload the level |

Two flags exist for CI and debugging: `--frames N` exits after N frames, and
`--level` starts in the level instead of the menu.

## Measure

`--sim` runs the game headless on a synthetic clock with a scripted policy —
the snake always turns right and dodges nothing — and writes one CSV row per
fixed step, plus a one-line summary:

```bash
./build/bin/DungeonEngine.exe --sim --sim-out sim.csv
```

```
sim: outcome=game_over score=0 length=1 steps=97
```

The CSV columns are `step,score,length,game_over,head_x,head_y,bat_x,bat_y`.
Positions are printed with enough digits to round-trip a float.

It runs on a synthetic clock (`GameLoop::setFrameDelta`), so every frame is
exactly one fixed step and two runs of the same build produce **byte-identical**
files — which is what makes a before/after comparison mean anything.

| Flag | |
|---|---|
| `--sim-out PATH` | CSV destination (default `sim.csv`) |
| `--sim-steps N` | fixed-step budget (default 3600 = 60 simulated seconds) |
| `--sim-seed N` | seed for the gameplay RNG (bat spawn and bounces) |
| `--sim-window` | do not force SDL's dummy drivers, so the run can be watched |

Exit code is 0 whenever the run completed and wrote its CSV — dying included, so
a balance harness does not go red because the game is hard. 1 is an
infrastructure failure.

The policy's numbers are a floor on difficulty, not a verdict on how the game
feels.

## Test

```bash
ctest --test-dir build --output-on-failure
```

The unit tests link the engine and the game library directly, so they need no
window, no input and no desktop session — which is what makes them usable in CI
and is the reason to reach for them rather than trying to drive the built game.
Three smoke tests run the real binary with SDL's dummy drivers to cover startup,
level loading, rendering and teardown end to end; the third drives a short
`--sim` pass and checks a summary came out of it.

The `sim_golden_seed*` tests lock behaviour: they run `--sim` twice per seed,
require both CSVs to be byte-identical, and compare them with the references in
`tests/golden/`. A refactor must leave them untouched. A change that is meant to
alter gameplay regenerates the reference in the same commit — the command is in
the header of `tests/sim_golden.cmake`.

Configure with `-DDE_WARNINGS_AS_ERRORS=ON` to build the way CI does.

## Design documents

`docs/design/` holds the game design — pillars, core loop, systems, biomes,
levels, balance tables and decision records. It is written in Spanish, and it is
about the *game*, not the engine; this README stays the technical reference.

Every document declares an `estado` (`implementado`, `parcial`, `propuesto`,
`descartado`) and lists the code that implements it, so drift between the two is
detectable rather than discovered late. `AGENTS.md` is the shared agent brief;
`.claude/` holds portable skills (also used by Cursor/Copilot), Claude-only
subagents and hooks that keep design and code in step. See `.claude/README.md`.

---

## Architecture

```
        third_party            vendored: imgui SDL2 backends, entity editor
             |                 headers exposed as SYSTEM includes
             v
          engine               static library, namespace de
             |                 loop, plugins, SDL, input, audio, textures,
             |                 Tiled loading, rendering, camera, quadtree,
             v                 scenes, widgets
         game_lib              this game: components, systems, scenes, HUD
             |
             v
           game                main()
```

**`engine` does not have `game/include` on its include path.** An
`#include <game/...>` from engine code is a compile error, not a convention
somebody has to remember. When the engine needs to know something about the
game, invert it: the Tiled loader records Tiled's `type` string in
`de::ObjectTypeComponent`, and `InGameScene::spawnPlayer` is what turns
`"Player"` into a `PlayerComponent`.

Rule of thumb for new code: **if it names a gameplay component, it belongs in
`game/`.**

### Where the frame goes

```
frame begin   input (SDL events -> InputState)  ->  ImGui::NewFrame
fixed steps   the game's systems (snake, bat, snake view), then spatial sync
              (zero or more times, each advancing DeltaTime::fixed)
frame         animation, grayscale fade, scenes, debug
last          render passes in `order`, then the GUI
frame end     present
```

Systems that must not depend on frame rate go in `addFixedSystem`. Everything
else goes in `addSystem`, and rendering in `addSystemLast`.
`addFixedSystemLast` runs after every fixed system in each step, for
bookkeeping that must see the step's final positions (the spatial index sync).

---

## Recipes

### Add a component

Components are plain data. Engine-level ones live in
`engine/include/engine/components/`, gameplay ones in
`game/include/game/components/`.

```cpp
// game/include/game/components/mana_component.hpp
#ifndef GAME_COMPONENTS_MANA_COMPONENT_HPP
#define GAME_COMPONENTS_MANA_COMPONENT_HPP

struct ManaComponent
{
    int current = 10;
    int max = 10;
};

#endif // GAME_COMPONENTS_MANA_COMPONENT_HPP
```

Attach it where the entity is built: the prefabs in `game/src/prefabs.cpp`,
which `InGameScene` uses for both Tiled objects and code-spawned entities.

### Add a system

```cpp
// game/include/game/systems/mana_regen_system.hpp
#include <engine/systems/system.hpp>

class ManaRegenSystem final : public de::System
{
public:
    void run(entt::registry& registry) override;
};
```

```cpp
// game/src/systems/mana_regen_system.cpp
void ManaRegenSystem::run(entt::registry& registry)
{
    const float dt = registry.ctx().get<de::DeltaTime>().fixed;
    for (auto&& [entity, mana] : registry.view<ManaComponent>().each())
    {
        // ...
    }
}
```

Register it in `GamePlugin::mount` and add the `.cpp` to `game/CMakeLists.txt`
— source lists are explicit, not globbed.

```cpp
gameLoop.addFixedSystem(std::make_shared<ManaRegenSystem>());
```

### Add a scene

Implement `de::Scene`: `onEnter` builds the entities, `onExit` destroys them.

```cpp
class ShopScene : public de::Scene
{
public:
    void onEnter(entt::registry& registry) override;
    void onUpdate(entt::registry& registry) override;
    void onExit(entt::registry& registry) override;

private:
    std::vector<entt::entity> m_entities;
};
```

Two things `onExit` must do, both learned the hard way:

```cpp
void ShopScene::onExit(entt::registry& registry)
{
    // The spatial trees hold entity handles. Clear them first, or the next
    // query hands back destroyed entities.
    registry.ctx().get<de::SpatialIndex>().clear();

    for (auto entity : m_entities)
    {
        // Checked: gameplay may already have destroyed some of them.
        if (registry.valid(entity))
        {
            registry.destroy(entity);
        }
    }
    m_entities.clear();
}
```

Switch to it with `requestScene`, which applies between frames:

```cpp
registry.ctx().get<de::SceneSystem>().requestScene(
    std::make_unique<ShopScene>());
```

Use `setScene` only from a setup callback, where there is no frame in progress.

---

## Layout

```
engine/          reusable 2D engine, static library (namespace de)
  include/engine/  public headers
  src/             implementation
game/            the example game
  include/game/    headers
  src/             implementation, plus main.cpp
tests/           doctest unit tests, .tmj fixtures and sim references
third_party/     vendored sources: imgui SDL2 backends, entity editor
assets/          textures, audio, Tiled (.tmj) levels and game.json
toolchain/       LLVM-MinGW CMake toolchain file and the vcpkg triplet
```

## Configuration

`assets/game.json` is read at startup:

| Key | |
|---|---|
| `title`, `fullScreen`, `screenWidth`, `screenHeight` | the window |
| `logicalWidth`, `logicalHeight` | the resolution the game draws in, which SDL scales to the window |
| `zoomLevel` | how many logical pixels one world unit takes |
| `frameRate`, `vsync` | pacing: vsync, or a manual limiter at `frameRate` |
| `debug` | opens the entity inspector |
| `levels` | level name → `.tmj` path; the game loads `arena` |

Every key but `levels` is optional.

If the file is missing or malformed, the game exits with a message naming the
problem rather than crashing.

## Troubleshooting

**`vcpkg was unable to detect the active compiler's information`** — vcpkg
sanitises the environment of the subprocesses it runs, so `LLVM_MINGW_ROOT`
only reaches it because the triplet lists it in `VCPKG_ENV_PASSTHROUGH`. If you
write your own triplet, keep that.

**Exit code `0xc0000135` when running a binary** — a DLL is missing. The build
puts everything needed in `build/bin`; this means the executable was moved out
of that directory on its own.

## Dependencies

Resolved through vcpkg (see `vcpkg.json`): SDL2, SDL2_image, SDL2_mixer, EnTT,
Dear ImGui, tinyxml2, zlib, libpng, doctest.

The sound effects in `assets/Audio/` are synthesized placeholders, not
recordings — see `assets/Audio/README.txt`.

## License

See [LICENSE](LICENSE).
