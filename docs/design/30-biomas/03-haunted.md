---
id: BIOMA-HAUNTED
titulo: Bioma 3 — Templo maldito
estado: propuesto
pilares: [PILAR-01, PILAR-03, PILAR-04]
codigo: []
---

# Bioma 3 — Templo maldito

**Nada de esto existe en el código.**

## Arte disponible

`assets/Tiles/3-Haunted-Temple.png`, `assets/Player/3-Heroes-Animated.png`,
`assets/Enemies/3-Monsters-Animated-Haunted.png`,
`assets/Items/3-Items-Haunted.png`, `assets/Minimap/3-Haunted-Mini-Map.png`.

## Regla propia propuesta: los enemigos atraviesan las paredes

Los fantasmas ignoran la colisión. Persiguen a 60 px/s en línea recta, como todos
([[SYS-ENE]]), pero **la geometría deja de protegerte**.

Es la inversión deliberada de [[PILAR-03]], y por eso es interesante: en los
demás biomas la pared es tu herramienta; aquí es solo un estorbo para ti. Obliga
a resolver el peligro con distancia pura en vez de con posición, que es un modo
distinto de jugar sin añadir ni un botón.

**Coste:** bajo. `EnemyAISystem` ya mueve en línea recta; lo único que hace que un
enemigo choque es `SolidBodyComponent`, que `tagObjectsByType` le añade. Un tipo
de enemigo sin ese componente ya atraviesa paredes. Es casi gratis.

**Riesgo:** puede resultar simplemente injusto si el nivel no da rutas de escape
largas. El diseño del nivel tendría que ser más abierto que
[[NIVEL-DUNGEON1]], no más laberíntico — justo lo contrario de lo que sugiere
el tema.

## Nota de contenido

Es el bioma que más pide ficción: un templo maldito invita a texto ambiental, y
hoy el juego no tiene ni una línea de ficción escrita en ninguna parte.
