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

Un murciélago a la vez. Se mueve en continuo (integra su propia velocidad en
el fixed step; no pasa por `TransformSystem`) y **rebota** en
`roomBounds` con reflexión de velocidad + `bounce.wav` — el mismo contrato
que el tutorial MonoGame (círculo vs bordes del room).

Al ser comido (intersección de círculos cabeza–bat):

- el slime crece,
- score += 100,
- suena `collect`,
- el bat respawnea al lado opuesto del room respecto a la cabeza,
- nueva velocidad con ángulo aleatorio.
