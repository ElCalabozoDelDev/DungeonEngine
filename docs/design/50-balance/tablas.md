---
id: BAL-01
titulo: Tablas de balance
estado: implementado
pilares: [PILAR-01]
codigo:
  - game/include/game/components/enemy_component.hpp
  - game/include/game/components/health_component.hpp
  - game/include/game/components/item_component.hpp
  - game/include/game/components/speed_component.hpp
  - game/include/game/components/attack_component.hpp
  - game/include/game/run/run_config.hpp
  - game/src/systems/combat_system.cpp
  - tests/test_balance.cpp
---

# Tablas de balance

## Dónde viven estos números, de verdad

En los inicializadores por defecto de los componentes y de `RunConfig`, en
C++. **No hay archivo de configuración de balance** que el `.tmx` pueda
sobreescribir: `tagObjectsByType` construye por defecto. Cambiar un número es
editar un `.hpp` y recompilar.

Esta tabla es una **copia**. `tests/test_balance.cpp` la mantiene honesta.
Ver [[ADR-0001]].

Los defaults de **run** (`floorsPerRun`, `objectiveFloor`, `fovRadiusTiles`)
están anclados en el test para evitar drift, pero son **punto de partida de
playtest**, no balance cerrado de diseño ([[LOOP-01]]).

## Jugador

| Qué | Valor | Dónde | Notas |
|---|---|---|---|
| Velocidad | **200 px/s** | `SpeedComponent::value` | 12,5 tiles/s. Diagonal normalizada. |
| Vida inicial / máx. | **5 / 5** | `HealthComponent` | Un corazón `[#]` por punto. |
| Invulnerabilidad tras golpe | **1,0 s** | `InvulnerabilitySeconds` en `combat_system.cpp` | Literal de gameplay. |
| Caja de colisión | **16 × 16 px** | sprite completo | Sin margen de gracia. |
| Ataque — daño | **1** | `AttackComponent::damage` | Arquitectura genérica; forma abierta ([[SYS-CMB]]). |
| Ataque — rango | **24 px** | `AttackComponent::range` | ~1,5 tiles; hoy chequeo por distancia. |
| Ataque — cooldown | **0,35 s** | `AttackComponent::cooldownSeconds` | |

## Enemigo

| Qué | Valor | Dónde | Notas |
|---|---|---|---|
| Rango de persecución | **120 px** | `EnemyComponent::chaseRange` | 7,5 tiles. |
| Velocidad | **60 px/s** | `EnemyComponent::speed` | |
| Daño por contacto | **1** | `EnemyComponent::contactDamage` | |
| Vida | **3** | `EnemyComponent::maxHealth` → `HealthComponent` | |

Sin tipos distintos todavía.

## Ítem / run

| Qué | Valor | Dónde | Notas |
|---|---|---|---|
| Valor coin | **1** | `ItemComponent::value` | |
| Pisos por run (N) | **3** | `RunConfig::floorsPerRun` | Configurable; no cerrado. |
| Piso mín. objetivo | **3** | `RunConfig::objectiveFloor` | Inclusivo; configurable. |
| Radio FOV | **8** tiles | `RunConfig::fovRadiusTiles` | libtcod `FOV_SHADOW` ([[SYS-FOV]]). |

## Las proporciones, que es lo que importa

| Relación | Valor | Qué significa |
|---|---|---|
| Velocidad jugador : enemigo | **3,33 : 1** | Parte de [[PILAR-01]] (posición). En línea recta no te alcanzan. |
| Rango de persecución : pantalla | **120 px ≈ ancho visible** | Con cámara ×3 sobre 800 px se ven ~267 px de mundo. |
| Distancia de escape post-golpe | **1,0 s × 140 px/s = 140 px** | Más que el chase range: un golpe es casi una salida gratis. |
| Vida jugador : daño contacto | **5 golpes** | Sin curación. |
| Vida enemigo : daño ataque | **3 golpes** | Con defaults actuales. |

## Cómo medir en vez de opinar

`--sim` sigue siendo la sonda determinista. La referencia histórica sobre
`dungeon1` (abajo) **ya no describe el loop jugable** ([[SYS-PROC]]); sirve
como archivo de lo que el test medía antes del generador. Hay que re-basar
telemetría sobre runs procedimentales cuando la política de `--sim` cubra
escaleras / objetivo / ataque.

```
sim: outcome=unreachable
sim: steps=164 sim_seconds=2.7333
sim: items=2/3 unreachable=1 pickups_at=0.4000,2.7167
sim: damage_taken=2 hits=2 final_health=3 first_hit_at=0.7833
sim: distance_px=529.554 mean_speed=193.739 blocked_ratio=0.0122
sim: chase_ratio=0.9024 min_enemy_dist=3.722
```

Ver [[NIVEL-DUNGEON1]] para el contexto de esas cifras. La política no
esquiva: es cota, no predicción de feel.
