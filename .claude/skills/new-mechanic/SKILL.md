---
name: new-mechanic
description: Add a gameplay mechanic to DungeonEngine — a new component, system, scene or widget — following the engine/game layering, the fixed-vs-frame system split, the explicit CMake source lists and the design-doc update that goes with it. Use when implementing a new gameplay feature, adding a component or system, or turning something from docs/design/ into code.
---

Work in this order. Steps 1 and 6 are the ones people skip, and both cost more
later than they save now.

## 1. Check it against the pillars first

Read `docs/design/00-pilares.md`. The pillars exist to make some features *not*
worth building — a player attack contradicts PILAR-01 outright, and smart enemy
pathfinding quietly destroys PILAR-03.

If the mechanic fights a pillar, say so before writing code. That is a
conversation, not a veto — but it should happen first, and it ends in an ADR
under `docs/design/90-decisiones/`, not in a silent decision.

## 2. Decide which side of the line it lives on

**The engine must never reference the game.** `engine` has no `game/include` on
its include path, so `#include <game/...>` from engine code is a compile error —
that is the enforcement, not a convention.

The splitting rule: **if it names a gameplay component, it belongs in `game/`.**

When the engine genuinely needs game knowledge, invert it, the way the Tiled loader
does: the engine records Tiled's `type` string in `de::ObjectTypeComponent`, and
`InGameScene::spawnPlayer` decides that `"Player"` means the snake head.
The engine provides the generic hook; the game supplies the meaning.

Entities are built by the prefabs in `game/src/prefabs.cpp` — add components
there, not at each spawn site, so Tiled objects and code spawns stay identical.
Keep game rules as free functions on components where you can (see
`game::snake::advance`): they test without a registry.

## 3. Write the component

`game/include/game/components/<name>_component.hpp` — a plain struct with
defaulted members. Header guard `GAME_COMPONENTS_<NAME>_COMPONENT_HPP`, classic
`#ifndef`, never `#pragma once`. Unnamespaced.

Defaults are the game's entire tuning surface (nothing in the `.tmj` overrides
them), so any number you put here belongs in `docs/design/50-balance/tablas.md`
and in `tests/test_balance.cpp` too.

## 4. Write the system

`game/include/game/systems/<name>_system.hpp` + `game/src/systems/<name>_system.cpp`,
deriving from `de::System` (one method, `void run(entt::registry&)`). The `.cpp`
may say `using namespace de;`; the header may not.

Register it in `GamePlugin::mount` (`game/src/plugins/game_plugin.cpp:106-113`).
**Order within each list is execution order**, and the existing fixed order is
Movement → EnemyAI → Collision → Combat.

| Use | For |
|---|---|
| `addFixedSystem` | Anything that must not depend on frame rate: movement, physics, collision, damage. Runs 0–5 times per frame at `DeltaTime::fixed` (1/60 s). |
| `addFixedSystemLast` | Engine bookkeeping that must follow every fixed system in the step (the spatial index sync). Rarely needed by gameplay. |
| `addSystem` | Once per frame, at `DeltaTime::value`: cameras, scene updates, animation. |
| `addSystemLast` | Rendering only. |

Getting this wrong is not cosmetic: a gameplay rule in `addSystem` behaves
differently on a fast machine.

## 5. Register the source files

Every `CMakeLists.txt` uses **explicit source lists — there is no globbing.** A
new `.cpp` that is not listed simply does not compile, and the failure looks like
a link error far from the cause.

- gameplay `.cpp` → `game/CMakeLists.txt`, in the `game_lib` list (**not** the
  `game` executable: `tests` links `game_lib`, so anything unlisted there is
  untestable)
- engine `.cpp` → `engine/CMakeLists.txt`
- test `.cpp` → `tests/CMakeLists.txt`

Headers are not listed anywhere.

## 6. Test it, then document it

Tests link `game_lib` and need no window. Follow `tests/test_gameplay.cpp`: an
anonymous-namespace `World` fixture that emplaces every context resource the
systems read (`DeltaTime`, `GameState`, `Paused`, `GameRng`, input) — the
game installs them once in `GamePlugin`, and systems `get` them rather than
creating them — then calls `system.run(registry)` directly. Its `step()`
skips fixed systems while `Paused` is set, as the loop does; a new fixed
system must not check for pause itself. `TEST_CASE` names are
lowercase English sentences.

Then update `docs/design/20-sistemas/<name>.md` — including its `codigo:` list,
which is what keeps the traceability check working. A mechanic that ships without
its document is the failure mode this whole setup exists to prevent.

## Verify

```
ninja -C build && ctest --test-dir build --output-on-failure
```

`clang-format` runs automatically on Write/Edit, but **not** when a file is
written through a shell heredoc — if you did that, run
`clang-format -i <file>` yourself or CI's format job will fail.
