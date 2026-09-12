---
id: LOOP-01
titulo: Core loop
estado: parcial
pilares: [PILAR-01, PILAR-02, PILAR-03]
codigo:
  - game/src/scene/in_game_scene.cpp
  - game/src/scene/menu_scene.cpp
  - game/src/plugins/game_plugin.cpp
  - game/include/game/state.hpp
---

# Core loop

## El minuto a minuto

1. Apareces en un punto fijo del nivel, con 5 corazones.
2. Ves parte del mapa: la cámara te sigue con zoom ×3, así que en pantalla cabe
   una fracción del nivel (416×736 px de mundo contra 800×600 de cámara a ×3).
   **No sabes dónde están los ítems hasta que los encuentras.**
3. Te mueves buscándolos. Al acercarte a menos de 120 px de un enemigo, éste
   empieza a perseguirte en línea recta.
4. Los esquivas ([[PILAR-01]]: no puedes hacer otra cosa). Si uno te toca,
   pierdes un corazón y quedas invulnerable un segundo.
5. Recoges un ítem: suena, desaparece, el contador sube.
6. Repites hasta recogerlos todos… **o hasta quedarte sin corazones**, que es la
   única forma que el juego tiene hoy de terminar.

## El hueco: no hay victoria

`estado: parcial` es por esto.

`GameState::itemsCollected` e `itemsTotal` existen y se muestran en el HUD, pero
**nada en el código comprueba que sean iguales**. Recoger los tres ítems de
`dungeon1` no produce ningún efecto: no hay pantalla de victoria, no hay
siguiente nivel, no vuelve al menú. Te quedas caminando por un nivel vacío.

La única transición de salida es morir: `CombatSystem` pone `gameOver` cuando la
vida llega a 0, y `GamePlugin` lo detecta en el frame-begin siguiente y te manda
al menú, que muestra "You died." y cuántos ítems llevabas.

Es decir: **el bucle está abierto por el extremo bueno**. El juego solo sabe
terminar mal.

Y hay algo peor, medido con `--sim`: en el único nivel que existe, **uno de los
tres ítems no se puede recoger desde ninguna posición** ([[NIVEL-DUNGEON1]]). Así
que hoy el contador del HUD no puede llegar a `3 / 3` ni siquiera jugando
perfecto. El bucle no solo no tiene final: tiene una meta visible e inalcanzable.

Esto no es un bug que arreglar a la ligera — es la decisión de diseño más grande
que queda pendiente, porque define qué *es* una partida:

- ¿Recoger todo es ganar, y el nivel se reinicia o se pasa al siguiente?
- ¿O los ítems son una puntuación y el objetivo es la salida, una puerta que se
  abre al completarlos?
- ¿O no hay victoria y el objetivo es sobrevivir el mayor tiempo posible?

La tercera opción es la que menos código necesita y la que peor encaja con el
HUD actual, que presenta los ítems como un progreso hacia algo. Las dos primeras
implican un objeto o un estado que hoy no existe.

**Esto se decide en un ADR, no aquí.** Mientras tanto el documento describe lo
que hay, no lo que debería haber.

## Estados y transiciones que sí existen

```
MenuScene  ──"Play" / Enter──>  InGameScene
    ^                                │
    │                                │ vida == 0  (gameOver)
    └────────────────────────────────┘
    ^                                │
    └────"Back to menu" desde pausa──┘

InGameScene  ──Esc──>  pausa (la simulación se congela, el render sigue)
InGameScene  ──F5───>  recarga la escena desde cero
```

La pausa es real: `GameLoop` deja de acumular tiempo y no corre ningún fixed
step, así que despausar no reproduce el parón como un atracón de simulación.

## Ritmo

Con los números actuales ([[BAL-01]]), un recorrido limpio de los tres ítems de
`dungeon1` son unos **850 px de camino, algo más de 4 segundos** de movimiento
puro. La partida dura lo que tardes en *encontrarlos*, no lo que tardes en
llegar: el tiempo real lo pone la exploración a ciegas, no la distancia.

Esto es importante para el balance: alargar una partida subiendo la vida del
jugador o bajando el daño no funciona, porque el cuello de botella no es la
supervivencia, es el descubrimiento. Lo que alarga una partida es esconder mejor
los ítems o hacer más caro volver sobre tus pasos.
