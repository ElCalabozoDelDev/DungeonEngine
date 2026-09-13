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

### 2. Smoke tests — the startup path

```
game --frames 30            # smoke_menu
game --level --frames 60    # smoke_level: loads arena.tmj
```

with `SDL_VIDEODRIVER=dummy`, `SDL_AUDIODRIVER=dummy`,
`SDL_RENDER_DRIVER=software`. They exercise `game.json`, assets, Tiled JSON
load, render and teardown.

### 3. `--sim` — measured gameplay

```
game --sim --sim-out sim.csv
```

Always-right snake policy. Metrics: `score`, `length`, `steps_alive`.
Two identical runs must produce byte-identical CSVs.
