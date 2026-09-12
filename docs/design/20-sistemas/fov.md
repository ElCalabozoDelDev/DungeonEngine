---
id: SYS-FOV
titulo: Campo de visión y niebla de guerra
estado: parcial
pilares: [PILAR-02, PILAR-03]
codigo:
  - game/src/systems/fov_system.cpp
  - game/include/game/systems/fov_system.hpp
  - engine/include/engine/spatial/visibility_map.hpp
  - engine/include/engine/graphics/renderer.hpp
  - tests/test_proc_fov.cpp
---

# Campo de visión y niebla de guerra

`parcial`: FoW de tiles **existe**; no confundirlo con el recorte de cámara
(zoom ×3), que sigue siendo otra cosa.

## Comportamiento observable

Para cada tile del piso actual (`de::VisibilityMap`):

| Estado | Apariencia |
|---|---|
| Nunca visto | Oculto (no se dibuja el tile). |
| En FOV actual | Visible a plena claridad (alpha 255). |
| Explorado, fuera de FOV | **Recordado atenuado** (`RememberedTileAlpha` = 90). |

Las **entidades** (sprites de enemigos, ítems, etc.) solo se dibujan en tiles
**Visible** — la niebla recordada conserva geometría, no delata amenazas ni
loot fuera de FOV.

## Decisión de implementación (investigación)

- **No se inventó un FoV desde cero.** Se reutiliza **libtcod** vía vcpkg
  (`default-features: false`, licencia BSD-3), API `TCOD_map_*`, algoritmo
  **`FOV_SHADOW`** (shadowcasting recursivo clásico).
- `FovSystem` construye un `TCOD_Map` desde la opacidad del
  `VisibilityMap`, llama `TCOD_map_compute_fov` desde la celda del jugador
  con radio `RunConfig::fovRadiusTiles` (default **8**), y escribe
  `visible` / `explored`.
- El render aplica alpha / skip según `VisibilityMap::appearanceAt`.

## Decisiones de diseño

- FoW es **regla de juego**, no cosmética ([[LOOP-01]], [[PILAR-02]]: informa
  ocultando el mapa, no con un tercer indicador de HUD).
- Radio en [[BAL-01]] como default tunable, no balance cerrado.

## Limitaciones conocidas

- **No hace (todavía):** minimapa filtrado por explorado; FoW persistente
  entre pisos (cada piso regenera / resetea el mapa de visión al montarse).
- **No hará:** sustituir FoW por “la cámara no muestra todo el mapa”.
