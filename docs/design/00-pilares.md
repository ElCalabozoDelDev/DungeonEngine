---
id: PILARES
titulo: Pilares de diseño
estado: implementado
subids: [PILAR-01, PILAR-02, PILAR-03]
codigo:
  - docs/design/00-pilares.md
---

# Pilares

Principios cerrados de Dungeon Slime. Añadir uno exige un ADR.

## PILAR-01 — Crecer es arriesgar

Cada murciélago comido alarga la cadena. Más largo = más puntos y menos
espacio libre. La dificultad nace de la propia longitud, no de enemigos
que persiguen.

## PILAR-02 — La grilla es justa

El slime se mueve a ticks fijos (200 ms), sin diagonales ni giros de 180°.
Colisión con muro o consigo mismo termina la partida de forma legible.
El murciélago se mueve en continuo y rebota; el jugador no.

## PILAR-03 — El marcador es el HUD

Una sola cifra: `SCORE` con seis dígitos. Sin corazones, inventarios ni
minimapa. Pause y Game Over son overlays, no una segunda capa de stats.
