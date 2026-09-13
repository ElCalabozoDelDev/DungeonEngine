---
id: SYS-GRW
titulo: Crecimiento
estado: implementado
pilares: [PILAR-01]
codigo:
  - game/src/systems/snake_system.cpp
  - game/src/systems/bat_system.cpp
  - game/include/game/components/snake_component.hpp
---

# Crecimiento

Al comer un murciélago se incrementa `pendingGrowth`. En el siguiente tick
no se elimina la cola: la cadena gana un segmento. Cada segmento reutiliza
el mismo sprite animado del slime.
