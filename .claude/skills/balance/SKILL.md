---
name: balance
description: Change or evaluate a Dungeon Slime tuning number — tick interval, points per bat, stride — keeping the component default, the pinning test and the design table in step, and measuring with --sim instead of guessing. Use when asked to tune, rebalance, or make the game easier or harder.
---

## Where the numbers live

| Number | File |
|---|---|
| movement interval (0.2 s) | `SnakeComponent::movementInterval` |
| score per bat (100) | `SnakeComponent::scorePerBat` |
| stride (20 px) | `SnakeComponent::stride` |

Defaults are in `game/include/game/components/snake_component.hpp`.
`game.json` is startup only (window, levels) — not balance.

## Changing one: three places, always

1. The component default.
2. `tests/test_balance.cpp`
3. `docs/design/50-balance/tablas.md`

## Measure with `--sim`

```
DungeonEngine --sim --sim-out before.csv
# change the number
DungeonEngine --sim --sim-out after.csv
```

Compare `score`, `length`, `steps_alive`. The policy always turns right and
dodges nothing — floor on difficulty, not feel.
