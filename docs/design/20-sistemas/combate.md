---
id: SYS-CMB
titulo: Combate
estado: parcial
pilares: [PILAR-01, PILAR-02]
codigo:
  - game/src/systems/combat_system.cpp
  - game/include/game/components/health_component.hpp
  - game/include/game/components/attack_component.hpp
  - game/include/game/components/enemy_component.hpp
  - game/include/game/components/item_component.hpp
  - game/include/game/state.hpp
  - tests/test_balance.cpp
---

# Combate

`parcial`: contacto, ataque ofensivo básico y muerte de enemigos **existen**;
la **forma** del ataque (melee vs proyectil, feel) sigue abierta
([[ADR-0002]]).

## Daño por contacto (implementado)

1. Baja invulnerabilidad.
2. Solape con enemigo y sin i-frames → `contactDamage` (1), 1,0 s de
   invulnerabilidad, sonido `hurt`.
3. Vida 0 → `gameOver`.

Un golpe por cooldown aunque solapen varios. Caja = sprite 16×16.

## Ataque del jugador (implementado, forma abierta)

- El jugador tiene `AttackComponent` { `damage`, `range`, `cooldownSeconds` }
  y la acción de input `attack`.
- Al pulsar con cooldown listo: aplica `damage` a enemigos con
  `HealthComponent` dentro de `range` (distancia entre posiciones). Si vida
  ≤ 0, se destruyen.
- Defaults actuales ([[BAL-01]]): daño 1, rango 24 px, cooldown 0,35 s.
- Comentario en código: el shape es genérico a propósito — hoy funciona como
  “melee por rango”; un proyectil futuro puede reutilizar el mismo pipeline
  de daño/muerte **sin** decidir el feel todavía.

## Vida enemiga (implementado)

Al tagear `Enemy`, se copia `EnemyComponent::maxHealth` (default **3**) a
`HealthComponent`. Sin pathfinding inteligente ([[SYS-ENE]], [[PILAR-03]]).

## Recogida de coins

Sigue en este sistema: solape con `ItemComponent` → suma al contador. El
**objetivo** de run lo recoge [[SYS-STAIRS]] / `ProgressionSystem` vía
`ObjectiveComponent` ([[SYS-ITM]]).

## Limitaciones conocidas

- Forma de ataque **TBD** (no hay proyectiles, ni arco, ni animación de
  golpe).
- Sin RNG de daño.
- `--sim` aún no modela la política de ataque de forma rica.
