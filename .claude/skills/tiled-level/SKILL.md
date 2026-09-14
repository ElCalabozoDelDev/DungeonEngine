---
name: tiled-level
description: Read, edit or analyse the Tiled .tmj levels in assets/. Covers which object types, object properties and layer names the JSON loader understands, the tile array encoding, tilesets (.tsj), and how to measure a level's geometry with a BFS. Use when placing spawns, designing a level, or answering questions about what a level contains.
---

The loader is `engine/src/loaders/tiled_loader.cpp`. It reads **Tiled JSON**
(`.tmj`) and external tilesets (`.tsj`). XML `.tmx` is not supported.

## Layers

Tile layers are placed by name, through `de::TiledLayerNames` (the
defaults below; a game can pass its own to `TiledLoader`):

| Name | Effect |
|---|---|
| `Bottom`, `Collision` | Drawn under objects |
| `Overlay` | Drawn over objects |
| anything else | Not drawn — unless it is the map's only tile layer, which is drawn under objects whatever its name |

Dungeon Slime does not collide with tiles; the snake is bounded by
`roomBounds`.

Tile `data` is a flat JSON array of global tile ids, length `width * height`.

## Objects

Every object gets a transform, a sprite and its `type` string as
`ObjectTypeComponent` — no velocity or other movement; that is the game's to
add. Recognised `type` strings:

- `Player` — slime spawn; `InGameScene` turns it into the snake head.

Bats are spawned in code, not from the map.

## Measuring

Parse the Bottom layer array; treat perimeter / non-walkable gids as blocked;
BFS the interior for path claims. Never eyeball distances.

## Config

Levels are registered in `assets/game.json` under `levels`.
