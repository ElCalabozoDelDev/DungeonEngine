---
id: SYS-UI
titulo: Menús y overlays
estado: implementado
pilares: [PILAR-03]
codigo:
  - game/src/ui/ui_skin.cpp
  - game/src/widgets/title_widget.cpp
  - game/src/widgets/options_widget.cpp
  - game/src/widgets/hud_widget.cpp
  - game/src/systems/grayscale_fade_system.cpp
  - engine/src/systems/render_system.cpp
  - engine/src/plugins/imgui_plugin.cpp
  - assets/ui.tsj
  - assets/fonts/04B_30_5x.spritefont
  - assets/effects/grayscaleEffect.fx
  - learn-monogame-2d/src/27-Conclusion/DungeonSlime/UI/
  - learn-monogame-2d/src/27-Conclusion/DungeonSlime/Scenes/TitleScene.cs
  - learn-monogame-2d/src/27-Conclusion/DungeonSlime/Scenes/GameScene.cs
---

# Menús y overlays

Fidelidad al resultado final del tutorial (`learn-monogame-2d` cap. 27).
Gum usa canvas **320×180** con zoom ×4 — esas coordenadas son nuestro
espacio lógico.

- **Title** — clear `(32,40,78)`, patrón PointWrap scroll. “Dungeon”/“Slime”
  con BMFont a escala lógica **1** (texels 1:1, present nearest ×4 = píxeles
  gordos sin antialias TTF), centros `(640,100)` / `(757,207)` /4 y sombra
  `+10`/4. Start `(50,-12)`, Options `(-50,-12)`. Esc sale.
- **Options** — label `(10,10)`, sliders MUSIC/SFX (panel 264×55 en Y 30/93
  con OFF/MAX), BACK `(-28,-10)`.
- **Pause / Game Over** — panel 264×70 centrado, título `(10,10)`, botones
  bottom-left/right margen 9. El escenario (tilemap + slime + bat) pasa a
  escala de grises con el mismo fade del tutorial (`Saturation` 1→0 a
  `0.02`/frame, pesos 0.3/0.59/0.11); la UI sigue a color encima.
- **SCORE** — `(20, 5)`, escala tipo FontScale 0.25 del `.fnt`.

Chrome desde `assets/ui.tsj`. Menús/HUD: BMFont `04b_30.fnt` + `atlas.png`
(Gum). Título: el mismo BMFont a escala 1 con nearest (sin TTF). Paneles
nine-slice con borde **5** px (1/3 de `panel-background.png` 15×15) para
estirar solo el centro α=175.
