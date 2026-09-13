---
id: SYS-SCR
titulo: Score
estado: implementado
pilares: [PILAR-03]
codigo:
  - game/include/game/state.hpp
  - game/src/widgets/hud_widget.cpp
  - game/src/systems/bat_system.cpp
---

# Score

`GameState::score` empieza en 0. Cada bat suma 100. El HUD muestra
`SCORE: NNNNNN` (seis dígitos con ceros a la izquierda). No hay high-score
persistente en v1.
