---
id: SYS-STAIRS
titulo: Escaleras y profundidad
estado: parcial
pilares: [PILAR-03]
codigo:
  - game/include/game/components/stairs_component.hpp
  - game/src/systems/progression_system.cpp
  - game/include/game/systems/progression_system.hpp
  - game/src/scene/in_game_scene.cpp
  - game/include/game/run/run_config.hpp
  - game/include/game/state.hpp
---

# Escaleras y profundidad

`parcial`: hay escaleras, cambio de piso y victoria en la entrada; la UX de
profundidad / mensaje de victoria sigue cruda.

## Qué hace hoy

- Objetos `StairsDown` / `StairsUp` → `StairsComponent`.
- Al solapar, `ProgressionSystem` escribe `pendingFloorChange`; la escena
  regenera el piso ([[SYS-PROC]]) conservando vida y `hasObjective`.
- Generador: bajada en pisos anteriores al último; subida en pisos por
  encima del primero, cerca de la entrada del piso.
- **N** = `RunConfig::floorsPerRun` (default **3**).
- **Objetivo** desde `RunConfig::objectiveFloor` (default **3**, inclusivo).
- Victoria: piso 1 + celda `entranceColumn`/`entranceRow` + objetivo
  ([[LOOP-01]]).

## Decisiones

- Victoria = entrada de la run + objetivo, no “llegar al piso N”.
- Sin portal de escape de un solo uso.
- N y `objectiveFloor` son **defaults de playtest**, configurables; no
  están cerrados como balance canónico (sí anclados en
  `tests/test_balance.cpp` / [[BAL-01]] para no drift).

## Limitaciones conocidas

- **No hace (todavía):** indicador de piso en HUD (competiría con
  [[PILAR-02]]); pantalla de victoria explícita.
- **No hará (core loop):** saltarse el retorno con un atajo de victoria.
