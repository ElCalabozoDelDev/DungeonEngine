---
id: ADR-0001
titulo: Números en C++ (no en JSON de balance)
estado: implementado
pilares: [PILAR-01]
codigo:
  - game/include/game/components/snake_component.hpp
  - tests/test_balance.cpp
---

# ADR-0001 — Números en C++

**Decisión:** los valores de balance del snake viven como `constexpr` /
defaults en componentes C++, pinneados por `test_balance.cpp` y documentados
en [[BAL-01]].

**Alternativa rechazada:** moverlos a `game.json`. El JSON es solo arranque
(ventana, niveles). Mezclar balance ahí duplicaría fuentes sin ganar
hot-reload en este boilerplate.
