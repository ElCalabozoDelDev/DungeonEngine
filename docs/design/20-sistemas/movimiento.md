---
id: SYS-MOV
titulo: Movimiento
estado: implementado
pilares: [PILAR-01, PILAR-03]
codigo:
  - game/src/systems/movement_system.cpp
  - game/include/game/components/speed_component.hpp
  - game/src/plugins/game_plugin.cpp
---

# Movimiento

## Qué hace

Lee dos ejes del `ActionMap` (`move_left`/`move_right`, `move_up`/`move_down`),
normaliza el vector resultante y lo multiplica por `SpeedComponent::value`
(200 px/s). Corre en el paso fijo, así que la velocidad no depende de los FPS.

Las teclas están enlazadas dos veces: flechas y WASD apuntan a las mismas
acciones. Los sistemas nunca ven scancodes, solo nombres de acción.

## Decisiones

**Normalizar la diagonal.** Sin normalizar, moverse en diagonal daba
√2 ≈ 1,41 veces más velocidad, y la ruta óptima en cualquier sala abierta era
zigzaguear. Se normaliza: la velocidad es 200 px/s en las ocho direcciones. El
comentario en el código deja constancia de que era un bug real.

**Movimiento libre, no por casillas.** El jugador no está alineado a la grilla de
16 px — de hecho arranca en x=182, que no es múltiplo de 16. Esto hace el
movimiento suave, pero significa que atravesar un hueco de un tile exige
precisión de píxel, porque el cuerpo mide exactamente lo mismo que el hueco.
Es una fuente deliberada de dificultad ([[PILAR-03]]) y la razón de que
[[NIVEL-DUNGEON1]] sea más duro de lo que parece.

**200 px/s no es un número suelto.** Es 3,33× la velocidad de un enemigo
([[SYS-ENE]]), y esa proporción *es* [[PILAR-01]]. Tocar la velocidad del jugador
sin tocar la del enemigo cambia el pilar, no el balance.

## Limitaciones conocidas

- No hay aceleración ni inercia: la velocidad pasa de 0 a máxima en un paso.
- No hay dash, ni esquiva, ni ninguna habilidad de movimiento. Dado
  [[PILAR-01]], un dash sería la primera adición coherente con el diseño — y
  también la que más desequilibraría la relación de velocidades.
