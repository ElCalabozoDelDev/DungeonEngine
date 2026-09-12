---
name: sim
description: >-
  Run a headless --sim telemetry pass, verify determinism, and interpret the
  numbers. Use when asked to simulate, measure balance, or compare a --sim run.
disable-model-invocation: true
---

Run a `--sim` telemetry pass and interpret it. Extra flags from the user go on
the command line after `--sim`.

1. Build first if anything changed: `ninja -C build`.

2. Run it twice, to the same flags, into different files:

```bash
cd build/bin && ./DungeonEngine --sim --sim-out a.csv && ./DungeonEngine --sim --sim-out b.csv
```

3. **Check determinism before anything else**: `diff a.csv b.csv` must be empty.
   If it is not, stop and report that — a non-deterministic run makes every
   comparison below meaningless, and that is the finding.

4. Report the summary: `outcome`, steps, `pickups_at`, `damage_taken`,
   `first_hit_at`, `distance_px`, `blocked_ratio`, `chase_ratio`,
   `min_enemy_dist`.

5. Compare against the reference for `dungeon1` at the current balance values:
   the optimal three-item tour is **53 tiles = 848 px = 254 fixed steps**, and
   damage is expected on the Coin1 → Coin2 leg, which crosses both zombie rooms.
   A materially higher step count means the policy fought the geometry; a
   `blocked_ratio` above a few percent means it spent real time against walls.

6. If a previous CSV or summary exists to compare against, diff the aggregates
   and say what moved and by how much.

Close with what the run does **not** tell you. The policy paths perfectly and
dodges nothing, so these numbers are a floor on difficulty for a mechanically
perfect, tactically blind player — never a verdict on how the game feels.
