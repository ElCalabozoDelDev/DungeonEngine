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
  - assets/ui.tsj
  - learn-monogame-2d/src/27-Conclusion/DungeonSlime/UI/
---

# Menús y overlays

Fidelidad al resultado final del tutorial (`learn-monogame-2d` cap. 27).
Gum usa canvas **320×180** con zoom ×4 — esas coordenadas son nuestro
espacio lógico.

- **Title** — clear `(32,40,78)`, patrón PointWrap scroll, “Dungeon”/“Slime”
  con sombra (posiciones pantalla/4), Start bottom-left `(50,-12)`, Options
  bottom-right `(-50,-12)`. Esc sale (sin botón Quit).
- **Options** — label `(10,10)`, sliders MUSIC/SFX (panel 264×55 en Y 30/93
  con OFF/MAX), BACK `(-28,-10)`.
- **Pause / Game Over** — panel 264×70 centrado, título `(10,10)`, botones
  bottom-left/right margen 9.
- **SCORE** — `(20, 5)`, escala tipo FontScale 0.25 del `.fnt`.

Chrome desde `assets/ui.tsj`. Texto con BMFont `04b_30.fnt` + `atlas.png`
(igual que Gum), no TTF de ImGui. Paneles nine-slice respetan el alpha 175
del centro de `panel-background.png`.
