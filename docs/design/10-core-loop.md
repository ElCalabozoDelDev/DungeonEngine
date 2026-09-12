---
id: LOOP-01
titulo: Core loop
estado: parcial
pilares: [PILAR-01, PILAR-02, PILAR-03]
codigo:
  - game/src/scene/in_game_scene.cpp
  - game/src/scene/menu_scene.cpp
  - game/src/plugins/game_plugin.cpp
  - game/src/systems/progression_system.cpp
  - game/include/game/state.hpp
  - game/include/game/run/run_config.hpp
---

# Core loop

## Lo que hay hoy (código)

La run ya **no** arranca en [[NIVEL-DUNGEON1]]. `InGameScene` genera el piso
con [[SYS-PROC]], aplica FoW ([[SYS-FOV]]), y `ProgressionSystem` gestiona
objetivo, escaleras y victoria.

1. Apareces en la entrada del **piso 1** de N (`RunConfig::floorsPerRun`,
   default 3), con 5 corazones. Esa celda se guarda en
   `GameState::entranceColumn` / `entranceRow`.
2. Exploras bajo **FoW** ([[SYS-FOV]]): FOV actual a plena vista; explorado
   fuera de FOV atenuado; nunca visto oculto. El zoom de cámara sigue siendo
   aparte (no es FoW).
3. Enemigos a ≤120 px te persiguen en línea recta. Puedes **atacar**
   ([[SYS-CMB]]: `AttackComponent` + acción `attack`); chocar sigue doliendo.
4. Recoges coins (puntuación) y, en pisos ≥ `objectiveFloor`, el **objetivo**
   ([[SYS-ITM]]).
5. **Escaleras** suben/bajan de piso ([[SYS-STAIRS]]); vida y
   `hasObjective` se conservan entre pisos.
6. **Victoria:** piso 1 + celda de entrada original + `hasObjective` →
   `victory` → vuelta al menú.
7. **Derrota:** vida 0 → `gameOver` → menú "You died."

### Qué sigue parcial

- El menú **no muestra** un mensaje de victoria dedicado: al detectar
  `victory` se limpia el flag y se pide `MenuScene`, así que ganas volviendo
  al menú sin texto de “You won.”
- Acantilados / caída ([[SYS-VERT]]) **no existen**.
- Forma del ataque (melee vs proyectil) **abierta**; hoy es chequeo de rango
  genérico ([[SYS-CMB]]).
- N y `objectiveFloor` son **defaults configurables**, no balance cerrado
  ([[BAL-01]]).

`dungeon1` y su Coin3 inalcanzable siguen documentados en
[[NIVEL-DUNGEON1]] como artefacto histórico; ya no son el loop jugable.

## Loop acordado (vigente en diseño y, en gran parte, en código)

1. Generación procedimental por piso ([[SYS-PROC]]).
2. Entrada del piso 1 = punto de victoria de la run (spawn tile guardado).
3. FoW con memoria atenuada ([[SYS-FOV]]).
4. Esquivar + atacar + geometría ([[PILAR-01]], [[PILAR-03]]); acantilados
   aún propuestos ([[SYS-VERT]]).
5. N pisos fijos, objetivo desde `objectiveFloor` (defaults 3 / 3).
6. Victoria = entrada de piso 1 + objetivo. Sin portal de escape.
7. Derrota = vida 0.

### Cleared por piso

- Intermedio: usar escalera válida (bajar / subir en el retorno).
- Run: objetivo en posesión **y** vuelta a la entrada de piso 1.

### Ritmo

El tiempo lo ponen FoW, deshacer caminos y el viaje de ida y vuelta. Las
cifras históricas de `dungeon1` en [[BAL-01]] son referencia, no target.

## Soft decisions (cerradas)

| Pregunta | Respuesta |
|---|---|
| N y piso del objetivo | `RunConfig::floorsPerRun` / `objectiveFloor`, defaults **3 / 3**. Configurables para balance; **no** son números de diseño cerrados. |
| Forma del ataque | **Abierta.** Existe arquitectura `AttackComponent` (damage / range / cooldown); el *feel* melee vs proyectil queda por decidir. |
| Entrada de victoria | **Spawn tile original del piso 1** (`entranceColumn` / `entranceRow`). No un tile de salida aparte. |

## Estados y transiciones (código)

```
MenuScene  ──"Play"──>  InGameScene (piso 1..N generados)
    ^                         │
    │                         ├─ vida == 0      (gameOver)
    │                         └─ entrada+obj.   (victory)
    └─────────────────────────┘
    ^
    └──── pausa / "Back to menu"

InGameScene  ──Esc──>  pausa
InGameScene  ──F5───>  recarga la escena (nueva run)
Escaleras             cambian `pendingFloorChange` → regeneran piso
```
