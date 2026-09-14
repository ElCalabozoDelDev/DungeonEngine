---
id: SYS-UI
titulo: Menús y overlays
estado: implementado
pilares: [PILAR-03]
codigo:
  - game/src/ui/ui_skin.cpp
  - game/src/ui/ui_layout.cpp
  - game/src/ui/bitmap_font.cpp
  - game/src/widgets/title_widget.cpp
  - game/src/widgets/options_widget.cpp
  - game/src/widgets/hud_widget.cpp
  - game/src/systems/grayscale_fade_system.cpp
  - game/src/play_state.cpp
  - engine/src/systems/render_system.cpp
  - engine/src/graphics/world_color_grade_pass.cpp
  - engine/src/plugins/imgui_plugin.cpp
  - assets/ui.tsj
  - assets/effects/grayscaleEffect.fx
  - learn-monogame-2d/src/27-Conclusion/DungeonSlime/UI/
  - learn-monogame-2d/src/27-Conclusion/DungeonSlime/Scenes/TitleScene.cs
  - learn-monogame-2d/src/27-Conclusion/DungeonSlime/Scenes/GameScene.cs
---

# Menús y overlays

Fidelidad al resultado final del tutorial (`learn-monogame-2d` cap. 27).
Gum usa canvas **320×180** con zoom ×4 — esas coordenadas son nuestro
espacio lógico. ImGui `DisplaySize` y el ratón se mapean a ese espacio.

- **Title** — clear `(32,40,78)`, patrón PointWrap scroll. “Dungeon”/“Slime”
  con BMFont a escala lógica **1** (texels 1:1, present nearest ×4), centros
  lógicos `(160,25)` / `(189.25,51.75)` y sombra `+(2.5,2.5)`. Start / Options
  con márgenes 50 / 12. Esc sale.
- **Options** — label `(10,10)`, sliders MUSIC/SFX (panel 264×55 en Y 30/93),
  BACK margen 28 / 10. Todo en BackgroundDrawList.
- **Pause / Game Over** — panel 264×70 centrado (Foreground). El escenario
  desatura con `WorldColorGrade` (`colorAmount` 1→0 a `1.2`/s, el `0.02` por
  frame del tutorial a 60 Hz; no depende de la frecuencia del monitor).
- **SCORE** — `(20, 5)`, FontScale 0.25.

Chrome desde `assets/ui.tsj`. Texto UI/título: BMFont `04b_30.fnt` +
`atlas.png`. Paneles nine-slice borde **5** px (centro α=175).
