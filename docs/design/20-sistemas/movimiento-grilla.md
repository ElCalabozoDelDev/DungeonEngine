---
id: SYS-SNK
titulo: Movimiento de grilla (snake)
estado: implementado
pilares: [PILAR-01, PILAR-02]
codigo:
  - game/src/systems/snake_system.cpp
  - game/include/game/components/snake_component.hpp
---

# Movimiento de grilla

Cada 200 ms la cabeza avanza un `stride` (20 px lógicos). Los segmentos
usan `At`/`To` con interpolación visual entre ticks.

- Input cardinal; se rechaza el giro de 180° (producto punto ≤ 0).
- Avance: insertar nueva cabeza, quitar cola (salvo `pendingGrowth`).
- Colisión cabeza–cuerpo o fuera de `roomBounds` → Game Over.
