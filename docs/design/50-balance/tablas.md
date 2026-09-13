---
id: BAL-01
titulo: Tablas de balance
estado: implementado
pilares: [PILAR-01, PILAR-02, PILAR-03]
codigo:
  - game/include/game/components/snake_component.hpp
  - game/include/game/components/bat_component.hpp
  - tests/test_balance.cpp
  - docs/design/50-balance/tablas.md
---

# Tablas de balance

Los números viven en tres sitios a la vez: default del componente,
`tests/test_balance.cpp` y esta tabla.

| Magnitud | Valor | Código |
|---|---|---|
| Intervalo de tick | **0.2 s** | `SnakeComponent::movementInterval` |
| Puntos por bat | **100** | `SnakeComponent::scorePerBat` |
| Stride (celda) | **20 px** | `SnakeComponent::stride` |
| Velocidad bat | **75 px/s** | `BatComponent::speed` |
| Sprite slime/bat | **20×20** | `DimensionComponent` + sheets `slime.png` / `bat.png` |
| Ventana | **1280×720** | `Config::screenWidth/Height` |
| Lógico | **320×180** | `Config::logicalWidth/Height` |
| Escala efectiva | **4×** | ventana / lógico |
| Música / SFX default | **60 %** | `AudioSettings` |
