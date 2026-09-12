---
id: SYS-ITM
titulo: Ítems
estado: parcial
pilares: [PILAR-02, PILAR-03]
codigo:
  - game/include/game/components/item_component.hpp
  - game/include/game/components/objective_component.hpp
  - game/src/systems/combat_system.cpp
  - game/src/systems/progression_system.cpp
  - game/src/scene/in_game_scene.cpp
  - game/src/proc/dungeon_generator.cpp
---

# Ítems

`parcial`: coins y objetivo están separados en código; el HUD aún trata sobre
todo el contador de coins, y “llevas el objetivo” es flag de run sin UI rica.

## Dos roles (código)

| Rol | Componente | Efecto |
|---|---|---|
| **Puntuación** | `ItemComponent` | Suma `value` a `itemsCollected` ([[SYS-CMB]]) |
| **Objetivo** | `ObjectiveComponent` | `hasObjective = true`; no suma al contador de coins |

El generador coloca score items en cada piso y el objetivo si
`floorIndex >= objectiveFloor` ([[SYS-PROC]], [[SYS-STAIRS]]).

Victoria: volver a la **entrada del piso 1** con `hasObjective`
([[LOOP-01]]).

## Lo que sigue siendo cierto

- `ItemComponent::value` default 1; sin rarezas ni inventario.
- Sin pociones / llaves todavía.
- [[PILAR-02]]: preferir no añadir un tercer número al HUD; el objetivo puede
  vivir como flag / feedback visual de mundo.

## Colocación

[[NIVEL-DUNGEON1]] documenta el viejo patrón de tres coins. En el generador,
las coins son cebo de exploración; el objetivo ancla el viaje de ida y vuelta.
