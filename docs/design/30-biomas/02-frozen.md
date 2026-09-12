---
id: BIOMA-FROZEN
titulo: Bioma 2 — Caverna helada
estado: propuesto
pilares: [PILAR-01, PILAR-03, PILAR-04]
codigo: []
---

# Bioma 2 — Caverna helada

**Nada de esto existe en el código.** Hay arte completo y ninguna línea que lo
cargue.

## Arte disponible

`assets/Tiles/2-Frozen-Cavern.png`, `assets/Player/2-Heroes-Animated.png`,
`assets/Enemies/2-Monsters-Animated-Frozen.png`, `assets/Items/2-Items-Frozen.png`,
`assets/Minimap/2-Frozen-Mini-Map.png`.

## Regla propia propuesta: el suelo resbala

El hielo añade inercia al movimiento del jugador: sueltas la tecla y sigues
deslizándote.

Encaja con los pilares mejor que ninguna otra idea que se me ocurra para este
tema:

- Ataca directamente [[PILAR-01]]: esquivar deja de ser gratis. La ventaja de
  velocidad 3,33:1 sigue ahí, pero **dejar de moverte cuesta**, y los enemigos
  lentos se vuelven peligrosos cerca de una pared.
- Multiplica [[PILAR-03]]: un corredor de un tile como el de
  [[NIVEL-DUNGEON1]] pasa de difícil a brutal si no puedes frenar.
- No toca el HUD, así que respeta [[PILAR-02]].

**Coste:** es el único cambio que requiere tocar [[SYS-MOV]], que hoy fija la
velocidad directamente sin aceleración. Habría que introducir inercia como
propiedad del nivel o del bioma, no del jugador.

**Riesgo:** el control resbaladizo es de las mecánicas que peor caen si está mal
ajustada. Necesita medirse con `--sim` (`blocked_ratio` es exactamente la métrica
que lo delata) y luego jugarse a mano.

## Alternativas descartadas de momento

- *Visibilidad reducida por ventisca*: choca con [[PILAR-02]].
- *Daño por frío con el tiempo*: convierte la exploración en una carrera, y
  [[LOOP-01]] dice que el tiempo de partida lo pone el descubrimiento. Sería
  pelearse con el core loop.
