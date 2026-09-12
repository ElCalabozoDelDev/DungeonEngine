---
id: SYS-VERT
titulo: Acantilados y caída
estado: propuesto
pilares: [PILAR-03]
codigo: []
---

# Acantilados y caída

`propuesto`: el juego es un plano 2D top-down sin capas verticales ni tiles de
“agujero”. No hay caída.

## Qué debe hacer (observable)

- Algunos pisos generados ([[SYS-PROC]]) incluyen **acantilados / desniveles**.
- **Caer no es muerte instantánea.** El jugador aterriza en una sección
  inferior del mismo piso (u otra región conectada por la caída), puede
  explorar allí, recoger cosas, y debe **encontrar la subida** (ruta, escalera
  interna, o equivalente legible).
- La caída es herramienta de [[PILAR-03]]: atajo caro, riesgo de enemigos
  abajo, o aislamiento temporal — no un hazard de “tocas = mueres”.

## Decisiones

- Separar “caer” de “morir”. La derrota sigue siendo vida a 0 ([[SYS-CMB]]).
- La subida de vuelta debe ser descubrible bajo FoW ([[SYS-FOV]]); no teletransporte
  silencioso al borde del mapa sin señal.
- Detalle de representación (segunda capa de tiles, regiones, triggers) es
  implementación; este documento fija el **comportamiento**.

## Limitaciones conocidas

- **No hace (todavía):** nada.
- **No hará:** usar la caída como kill-zone por defecto del core loop.
