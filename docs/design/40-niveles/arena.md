---
id: NIVEL-ARENA
titulo: Arena
estado: implementado
pilares: [PILAR-02]
codigo:
  - assets/tilemap.tmj
  - game/src/scene/in_game_scene.cpp
---

# Arena

Mapa fijo 16×9 tiles de 20 px (320×180 lógicos), escalado ×4 a ventana
1280×720. Perímetro de muros; el área jugable es el interior inset un tile
(`roomBounds`). Spawn del Player en el object layer; el bat se crea en código.

Sin FOV, sin pisos procedurales, sin cámara que siga al jugador.
