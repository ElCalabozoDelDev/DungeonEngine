---
id: LOOP-01
titulo: Core loop
estado: implementado
pilares: [PILAR-01, PILAR-02, PILAR-03]
codigo:
  - game/src/scene/in_game_scene.cpp
  - game/src/scene/title_scene.cpp
  - game/src/systems/snake_system.cpp
  - game/src/systems/bat_system.cpp
  - game/include/game/state.hpp
---

# Core loop

1. **Title** — START abre la arena; OPTIONS ajusta música/SFX. El tema
   arranca aquí y no se detiene ni reinicia al cambiar de escena.
2. **Jugar** — el slime avanza por celdas; el murciélago vaga y rebota.
3. **Comer** — contacto cabeza–bat → +1 segmento, +100 puntos, bat respawnea.
4. **Morir** — muro o cuerpo propio → Game Over (RETRY / QUIT).
5. **Pausa** — Esc muestra Pause; no avanza el tick del snake.

No hay victoria ni pisos. La partida termina solo al morir o al salir.
