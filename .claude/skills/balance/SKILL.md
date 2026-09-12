---
name: balance
description: Change or evaluate a gameplay tuning number — player speed, enemy chase range, contact damage, health, item value — keeping the component default, the pinning test and the design table in step, and measuring the effect with a --sim run instead of guessing. Use when asked to tune, rebalance, or make the game easier or harder.
---

## Where the numbers live

In the **default member initialisers of the components**, and nowhere else:

| Number | File |
|---|---|
| player speed (200) | `game/include/game/components/speed_component.hpp` |
| health (5/5) | `game/include/game/components/health_component.hpp` |
| chase range (120), enemy speed (60), contact damage (1) | `game/include/game/components/enemy_component.hpp` |
| item value (1) | `game/include/game/components/item_component.hpp` |
| invulnerability (1.0 s) | `InvulnerabilitySeconds`, `game/src/systems/combat_system.cpp:19` |

**The `.tmx` cannot override any of them** — `tagObjectsByType`
default-constructs every component. That is the whole tuning surface, and it is
why a "fast enemy" is impossible without C++ (see `docs/design/90-decisiones/0001-numeros-en-cpp.md`).

## Changing one: three places, always

1. The component default.
2. `tests/test_balance.cpp` — it pins every value and will go red. That is the
   point: it is the tripwire, not an obstacle.
3. `docs/design/50-balance/tablas.md` — the human-readable table.

Skipping 2 or 3 is how the design document silently stops being true.

## Think in ratios, not values

The individual numbers say little. These relationships are the design:

| Ratio | Value | What it is |
|---|---|---|
| player : enemy speed | **3.33 : 1** | This *is* PILAR-01 (dodge, don't fight). Changing it changes the game, not the difficulty. |
| chase range : visible width | 120 px ≈ one screen at zoom ×3 | An enemy activates roughly when it comes on screen. |
| invulnerability × speed difference | 140 px > 120 px chase range | A hit is effectively a free escape. Probably unintentional; noted in the table. |
| health : damage | 5 hits, no healing anywhere | Five mistakes per run. |

Before changing a value, work out which ratio it moves. If it moves the first
one, it needs a pillar discussion, not a tweak.

## Measure, don't guess

```bash
cd build/bin && ./DungeonEngine --sim --sim-out before.csv
# ...change the number, rebuild...
./DungeonEngine --sim --sim-out after.csv
```

The summary on stdout carries `outcome`, step count, `pickups_at`,
`damage_taken`, `first_hit_at`, `distance_px`, `blocked_ratio`, `chase_ratio` and
`min_enemy_dist`. Compare those, not impressions.

Reference for `dungeon1` at the current values: the optimal three-item tour is
**53 tiles = 848 px = 254 fixed steps**, and damage is expected on the
Coin1 → Coin2 leg, which crosses both zombie rooms.

Two runs with the same numbers must produce **byte-identical CSVs**. If they do
not, the run is not deterministic and no comparison from it means anything — fix
that before drawing any conclusion.

## The honest caveat

The sim policy does not dodge. It measures a floor on difficulty, not the
experience. A change that looks good in the CSV still has to be played. Say so
rather than presenting measured numbers as a verdict on feel.
