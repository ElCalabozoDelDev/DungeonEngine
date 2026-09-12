---
name: playtest
description: Verify a gameplay change without a window — doctest unit tests, the headless smoke tests under SDL's dummy drivers, and the --sim telemetry run. Covers what each one can and cannot prove. Use when asked to test, verify or check a gameplay change, or before claiming something works.
---

**Do not try to drive the built game.** It needs a foreground window and has
repeatedly proved unreliable to automate. Everything below runs headless.

## The three levels of verification

### 1. Unit tests — the default

```
ctest --test-dir build --output-on-failure
```

`tests` links `engine` and `game_lib` directly, so systems run with no window,
no input and no desktop session. This is where gameplay rules get pinned.

### 2. Smoke tests — the startup path

Two CTest entries run the real binary under dummy drivers:

```
game --frames 30            # smoke_menu:  boots, shows the menu, exits
game --level --frames 60    # smoke_level: loads dungeon1, runs 60 frames, exits
```

with `SDL_VIDEODRIVER=dummy`, `SDL_AUDIODRIVER=dummy`,
`SDL_RENDER_DRIVER=software`. They exercise config, assets, TMX load, render and
teardown together — the only tests that run `main()` and the plugins as a whole.

They prove the game **starts and shuts down cleanly**. They prove nothing about
whether it is fun, fair or even playable.

### 3. `--sim` — measured gameplay

```
game --sim --sim-out sim.csv
```

Runs headless on a synthetic clock with a deterministic navigation policy, and
writes one CSV row per fixed step plus a summary to stdout. This is the only
tool that produces *numbers* about gameplay. See the `balance` skill.

## What none of this can tell you

`--sim` drives a **repeatable probe, not a player**: it paths perfectly and
dodges nothing, walking straight through enemies and taking the hits. Its numbers
answer "how much does this level punish someone mechanically perfect and
tactically blind" — a bound, not an experience.

Feel, readability, tension, whether a corridor is tense or just annoying: none of
it is measurable here. When a change is about feel, say that it needs a human at
the keyboard rather than implying the tests covered it.

## Running the binary by hand

```bash
cd build/bin && SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy SDL_RENDER_DRIVER=software ./DungeonEngine --level --frames 120
```

Always pass `--frames`: without it the run never exits on its own.

Assets are mirrored into `build/assets` by the `sync_assets` target, which is
always considered out of date, so editing a `.tmx` and re-running `ninja` is
enough — no need to touch a source file.

## Reporting

Report what actually ran. If a test failed, show the output. If a change was only
verified by unit tests, say that the startup path was not exercised. "Tests pass"
after running one file is a misreport.
