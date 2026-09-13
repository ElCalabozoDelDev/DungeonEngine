---
name: sim
description: >-
  Run a headless --sim telemetry pass, verify determinism, and interpret the
  numbers. Use when asked to simulate, measure balance, or compare a --sim run.
disable-model-invocation: true
---

1. Build if needed: `ninja -C build`.

2. Run twice with the same flags:

```bash
cd build/bin && ./DungeonEngine --sim --sim-out a.csv && ./DungeonEngine --sim --sim-out b.csv
```

3. `diff a.csv b.csv` must be empty — otherwise stop; determinism failed.

4. Report `score`, `length`, `steps_alive` (and any summary the binary prints).

5. The policy always moves right and never dodges — floor on difficulty, not
   feel. There is no chase ratio or dungeon1 tour anymore.
