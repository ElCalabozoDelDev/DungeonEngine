---
id: SYS-UI
titulo: Menús y overlays
estado: implementado
pilares: [PILAR-03]
codigo:
  - game/src/scene/title_scene.cpp
  - game/src/scene/options_scene.cpp
  - game/src/widgets/title_widget.cpp
  - game/src/widgets/options_widget.cpp
  - game/src/widgets/hud_widget.cpp
---

# Menús y overlays

- **Title** — fondo de patrón, título, START / OPTIONS.
- **Options** — sliders MUSIC / SFX (0–100) y BACK.
- **In-game** — SCORE; Esc → Pause; muerte → Game Over con RETRY / QUIT.

UI con widgets ImGui + texturas del pack slime (no Gum).
