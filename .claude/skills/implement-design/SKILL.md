---
name: implement-design
description: Turn the design documents in docs/design/ into code — retire the template's example game when the project is a new game, then implement the design one document at a time, raising each document's estado and codigo as the code lands. Use when the design is ready to build, when asked to implement, program or start coding the game, or to implement a specific document id (SYS-…, LOOP-…, NIVEL-…).
---

This is the step after the `gdd-architect` interview: design documents in, code
out, documents updated to say what now exists. It orchestrates other skills —
`new-mechanic` for each component or system, `tiled-level` for levels,
`balance` for numbers, `build` and `playtest` for verification — rather than
repeating them. Load each one when you reach it.

## 0. Preconditions — stop if any fails

1. **Read `docs/design/proyecto.json`.** Missing or not `nuevo`/`existente`:
   stop and ask the user whether this is a new game or the example already in
   the code (the `gdd-architect` asks the same question; its answer lives in that
   file). Never infer it from the code.
2. **The design exists.** `00-pilares.md` and the core loop (`LOOP-…`) must be
   present. Without them there is nothing to implement against — send the user
   to the `gdd-architect` subagent instead of improvising a design in code.
3. **The document is buildable.** Read the one you are about to implement.
   Open questions, "desconocido" numbers or an undecided win/lose condition are
   decisions, not implementation details: ask the user, write the answer into
   the document (following the `gdd` skill), *then* code. Code that settles a
   design question the document leaves open is design drift on day one.
4. **Work on a feature-named branch**, never `main` (see `AGENTS.md`).

Then follow **A** if the mode is `nuevo`, **B** if it is `existente`. A new game
goes through A exactly once and then lives in B.

## A. New game: retire the example first

The template's gameplay (Dungeon Slime: snake, bat, score, arena) has to leave
the code before the new game enters it. Mixing the two produces a hybrid nobody
designed, and every agent after you will read the snake as part of your game.
Do this as **its own branch and commit**, before any new mechanic.

### A1. Inventory — measure, don't recall

```
grep -rlE "Snake|Bat|snake|bat_|Player" game tests --include=*.cpp --include=*.hpp
```

Sort every hit, plus `assets/` and `tests/golden/`, into three piles and **show
the table to the user before deleting anything**:

| Pile | Typical contents — verify each file |
|---|---|
| **Example gameplay** — remove | `game/{include/game,src}/components/{snake,bat,player}_component`, `systems/{snake,bat,snake_view}_system`, `prefabs`, `tests/test_snake_rules.cpp`, `tests/test_gameplay.cpp`, `tests/golden/*.csv` |
| **Shell** — keep, strip example references | `main.cpp`, `command_line`, `play_state`, `state.hpp` (`GameState` fields are the example's: `score`, grid size), `rng`, `assets`, `game_plugin` (system registration), `scene/{title,options,in_game}_scene`, `ui/`, `widgets/{title,options}_widget`, `sim/` (deterministic clock, CSV writer) |
| **Depends on the new design** — ask | `widgets/hud_widget` (the example's HUD is one score), `systems/grayscale_fade_system`, `assets/tilemap.tmj` and the `"arena"` level name in `assets/game.json` and `in_game_scene.cpp`, the window `title`, tilesets and sprites |

Engine tests that only use a `"Player"` type string as sample data
(`test_tiled_loader`, `test_spatial_index`, …) test the engine, not the example
— leave them.

### A2. Cut to an empty, green shell

The target is the smallest game that still **builds, reaches the title screen,
enters an empty level and passes ctest**:

- Remove the example gameplay files and their entries in `game/CMakeLists.txt`
  and `tests/CMakeLists.txt` (source lists are explicit — see `new-mechanic`).
- `InGameScene` loads the level and spawns nothing it no longer knows.
  `GamePlugin` registers no gameplay systems yet.
- `GameState` keeps only what the shell itself reads (`playState`); the new
  design adds its own fields later.
- `tests/test_balance.cpp` keeps only the checks that are still true (the
  logical render size). The balance table in the new tree starts empty.
- `--sim` cannot keep a snake policy or snake CSV columns. Reduce it to the
  deterministic clock and a stub row, keep `smoke_sim`, and **remove the
  `sim_golden_seed*` tests and `tests/golden/`** — there is no behaviour to lock
  yet. Say in the commit that the golden comes back once the new game has a
  policy (step B6).
- Update the prose that names the example as *the* game: `README.md`,
  `AGENTS.md` ("worked example", "Dungeon Slime", `--sim` description),
  `game/CMakeLists.txt`'s header comment.
- Leftover example design documents in `docs/design/` go too (`git rm`), unless
  the user already removed them.

Verify with `build` + `ctest --test-dir build --output-on-failure` (the
`playtest` skill). All green, no warnings, clang-format clean.

### A3. Flip the mode

Write `{ "modo": "existente" }` to `docs/design/proyecto.json` in the same
commit, and tell the user: from now on the code in `game/` is their game, and
the `gdd-architect` will read it as such. Commit, then continue with B on a new
branch.

## B. Implement one document at a time

### B1. Pick the slice

Order: the **core loop's smallest playable slice** first (the player's verb,
one obstacle or goal, lose/win if the loop has one), then the systems it links
to with `[[SYS-…]]`, then levels, then polish. Name the document id and the
slice to the user before starting; a document can land in several passes.

### B2. Check the pillars

Read `00-pilares.md`. If what the document asks for fights a pillar, raise it
now — the `pillar-guard` subagent can check a proposal. The outcome is an ADR,
not a silent choice in code.

### B3. Build it

Follow the `new-mechanic` skill for every component, system, scene or widget:
engine/game split, `addFixedSystem` vs `addSystem`, explicit CMake lists,
prefabs, doctest `World` fixture. Levels follow `tiled-level`. Implement what the
document says — no extra features, no speculative hooks for the next document.
When the document is silent on something the code must decide, ask, and write
the answer into the document.

### B4. Numbers

Every tuning value is a component default **and** a row in
`docs/design/50-balance/tablas.md` **and** a `CHECK` in `tests/test_balance.cpp`,
in the same change (`balance` skill). A number the user gave as intent is still
intent until `--sim` or play confirms it — say so in the table.

### B5. Update the document — same commit

- `estado`: `propuesto` → `parcial` when some of the described behaviour exists,
  `implementado` only when **all** of it does and is verified. When in doubt,
  `parcial`, with a *Pendiente* section listing what is missing.
- `codigo:` lists every file that implements it (the drift hook depends on it).
- Rewrite anything the implementation proved wrong or had to decide, and say so.
  **The code wins; the document follows in the same change.**

### B6. Verify

`build`, then `ctest --test-dir build --output-on-failure`, then the smoke tests
(`playtest` skill). Once the core loop is playable, give `--sim` a policy for
the new game — deterministic, documented as a floor like the example's — and
bring back `sim_golden_seed*` with a fresh `tests/golden/` (command in the
header of `tests/sim_golden.cmake`). From then on, a gameplay change regenerates
the golden on purpose in its commit; a refactor never does.

Report what was verified and how. Do not claim a behaviour works because it
compiles: if only the unit tests cover it, say that, and leave `estado` at
`parcial` until the user has played it.

### B7. Hand off

One document (or slice) per commit on the branch. Tell the user which ids moved
to which `estado`, what is pending, and the next slice you would take. Commit,
push and open PRs only when the user asks.
