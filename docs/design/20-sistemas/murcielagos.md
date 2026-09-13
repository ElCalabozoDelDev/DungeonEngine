---
id: SYS-BAT
titulo: Murciélagos
estado: implementado
pilares: [PILAR-01, PILAR-02]
codigo:
  - game/src/systems/bat_system.cpp
  - game/include/game/components/bat_component.hpp
---

# Murciélagos

Un murciélago a la vez. Se mueve en continuo con velocidad aleatoria y
**rebota** en los bordes del room (`bounce.wav`). Al ser comido:

- el slime crece,
- score += 100,
- suena `collect`,
- el bat respawnea lejos de la cabeza.
