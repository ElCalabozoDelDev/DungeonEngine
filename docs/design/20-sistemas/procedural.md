---
id: SYS-PROC
titulo: Generación procedimental
estado: parcial
pilares: [PILAR-03, PILAR-04]
codigo:
  - game/include/game/proc/dungeon_generator.hpp
  - game/src/proc/dungeon_generator.cpp
  - game/include/game/proc/level_assembler.hpp
  - game/src/proc/level_assembler.cpp
  - game/include/game/proc/floor_blueprint.hpp
  - game/include/game/run/run_config.hpp
  - game/src/scene/in_game_scene.cpp
---

# Generación procedimental

`parcial`: el juego **genera** pisos y ya no carga `dungeon1.tmx` para el
core loop. Faltan biomas distintos, acantilados y más vocabulario de
geometría intencionada.

## Qué hace hoy

- `DungeonGenerator` produce un `FloorBlueprint` (salas + pasillos +
  marcadores) de forma **determinista** para `(seed, floorIndex, config)`.
- `LevelAssembler` lo convierte en el mismo shape ECS que el cargador TMX
  (tiles, objetos tipados, texturas del bioma mazmorra).
- Marcadores: entrada, escaleras up/down, enemigos, score items, objetivo
  si `floorIndex >= objectiveFloor`.
- [[NIVEL-DUNGEON1]] queda como artefacto histórico / referencia; no es el
  camino del loop ([[LOOP-01]]).

## Decisiones

- **Legibilidad > laberinto ilegible** ([[PILAR-02]], [[PILAR-03]]).
- Semilla en `RunConfig::seed` (default 1) — misma semilla → mismos pisos.
- Un solo tileset / bioma en la práctica ([[BIOMA-DUNGEON]]); [[PILAR-04]]
  sigue `propuesto` para reglas por bioma.

## Limitaciones conocidas

- **No hace (todavía):** elección de bioma por piso, zonas especiales ricas,
  acantilados ([[SYS-VERT]]), tuning fino de embudos.
- **No hará:** volver a extender el core loop solo editando `dungeon1.tmx`.
