---
id: BIOMA-DUNGEON
titulo: Bioma 1 — Mazmorra
estado: implementado
pilares: [PILAR-02, PILAR-04]
codigo:
  - assets/Levels/dungeon1.tmx
---

# Bioma 1 — Mazmorra

Línea base visual y de vocabulario contra la que se definen los otros tres.

Hoy el código solo lo ejerce vía [[NIVEL-DUNGEON1]]. A futuro el bioma se
elige/asigna desde [[SYS-PROC]] (misma promesa de arte y regla, sin seguir
editando el `.tmx` handcrafted como destino del core loop).

## Arte disponible

| Uso | Archivo | ¿Se usa? |
|---|---|---|
| Tiles | `assets/Tiles/1-Dungeon.png` (10×10 tiles de 16 px) | sí |
| Tiles alternativos | `assets/Tiles/1-Cave.png`, `assets/Tiles/Floor.png` | no |
| Héroes | `assets/Player/1-Heroes-Animated.png` | sí (fila 1, 3 frames) |
| Monstruos | `assets/Enemies/1-Monsters-Animated.png` | sí (3 variantes de sprite) |
| Ítems | `assets/Items/1-Items.png` | sí (3 variantes) |
| Minimapa | `assets/Minimap/1-Mini-Map.png` | **no** — no hay sistema de minimapa |

## Qué promete

Piedra, pasillos, salas pequeñas. El vocabulario clásico: es el bioma que enseña
las reglas, y por eso es el que **no** debe tener una regla propia. Su
particularidad es no tener ninguna.

## Regla propia

Ninguna, a propósito. Es la referencia.
