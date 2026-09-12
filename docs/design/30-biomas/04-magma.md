---
id: BIOMA-MAGMA
titulo: Bioma 4 — Cavernas de magma
estado: propuesto
pilares: [PILAR-02, PILAR-03, PILAR-04]
codigo: []
---

# Bioma 4 — Cavernas de magma

**Nada de esto existe en el código.**

## Arte disponible

Es el bioma con más material: `assets/Tiles/4-Cave-Magma.png`,
`assets/Tiles/4-Cave-Obsidian.png` y `assets/Tiles/4-Extras.png` — **tres** sets
de tiles frente a uno de los demás. Más `assets/Player/4-Heroes-Animated.png`,
`assets/Enemies/4-Monsters-Animated-Magma.png`, `assets/Items/4-Items-Magma.png`,
`assets/Minimap/4-Magma-Mini-Map.png`.

Que tenga un set "Extras" propio sugiere que el arte lo pensó como el bioma
final.

## Regla propia propuesta: suelo que hace daño

Casillas de lava: un tipo de tile que quita un corazón al pisarlo, con la misma
invulnerabilidad de 1 s que un enemigo ([[SYS-CMB]]).

- Convierte el mapa en el enemigo de forma literal, que es [[PILAR-03]] llevado
  al extremo.
- Reutiliza el sistema de daño existente: no hay mecánica nueva que explicar al
  jugador, y el HUD no cambia ([[PILAR-02]]).
- Hace que las rutas tengan **coste** en vez de solo longitud. Es la primera vez
  que el jugador tendría que elegir entre rápido y seguro.

**Coste:** medio. Hace falta un tipo de tile dañino, que hoy no existe — el
cargador de TMX solo distingue las capas `Bottom`, `Overlay` y `Collision`.
Probablemente una cuarta capa, o una propiedad por tile.

**Riesgo:** con 5 corazones y sin curación en todo el juego ([[BAL-01]]), un suelo
que hace daño puede volver el bioma imposible. Es el primer caso donde el balance
del jugador tendría que cambiar por bioma — y hoy no hay forma de hacer eso sin
tocar C++ ([[ADR-0001]]).
