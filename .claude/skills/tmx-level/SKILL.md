---
name: tmx-level
description: Read, edit or analyse the Tiled .tmx levels in assets/Levels/. Covers exactly which object types, object properties and layer names the loader understands (everything else is silently ignored), the base64+zlib tile encoding, the map-level texture registry, the sprite/tileset inventory available in assets/, and how to measure a level's real geometry with a BFS. Use when placing or moving enemies and items, designing a level, or answering questions about what a level actually contains.
---

The loader is `engine/src/loaders/tmx_loader.cpp`. It is strict in the worst way:
**everything it does not recognise is skipped in silence**, with no error. A typo
in a `type` or a property name produces an entity that renders and does nothing.

## The three things that are matched by exact string

**Object `type`** — read into `de::ObjectTypeComponent` verbatim, then
interpreted by `InGameScene::tagObjectsByType` (`game/src/scene/in_game_scene.cpp:124-151`):

| `type` | Becomes |
|---|---|
| `Player` | `PlayerComponent`, `SpeedComponent`, `HealthComponent`, `SolidBodyComponent` |
| `Enemy` | `EnemyComponent`, `SolidBodyComponent` |
| `Item` | `ItemComponent`, and `++itemsTotal` |

Case-sensitive. Anything else — including a missing `type` — is ignored.
**Exactly one `Player` is required**: without it the camera has nothing to follow
and the level fails to start with a named error.

**Object `<properties>`** — only these five names are read
(`tmx_loader.cpp:200-234`):

`textureID`, `spriteRow`, `spriteCol`, `totalFrames`, `animationTime`

`textureID` must match a key in the **map-level** `<properties>`, which is the
texture registry:

```xml
<properties>
 <property name="items" value="../Items/1-Items.png"/>
 <property name="monster1" value="../Enemies/1-Monsters-Animated.png"/>
 <property name="player" value="../Player/1-Heroes-Animated.png"/>
</properties>
```

Note: **the object's `name` attribute is read by nothing.** "Zombie1", "Mimic"
are comments for whoever opens Tiled.

**Layer names** — only `Bottom`, `Overlay` and `Collision` (`tmx_loader.cpp:312-323`).
A layer with any other name loads but is never inserted into a quadtree and never
drawn. `Overlay` and `Collision` are both solid for collision
(`engine/src/systems/collision_system.cpp:22`) when they carry
`<property name="Collidable" type="bool" value="true"/>` — matched as the literal
string `"true"`.

## You cannot repaint tiles by hand

Tile data must be **base64 + zlib**; anything else fails to load
(`tmx_loader.cpp:297-303`). Editing the object layer is plain XML and is fine.
Editing the *map* means decoding, mutating a 26×46 int array and re-encoding — do
that with a script, or say plainly that it needs Tiled. Never guess at the blob.

## Measuring a level, instead of guessing

Design claims about a level must be computed. Decode the layers, mark a cell solid
if `Collision` **or** `Overlay` has a non-zero gid, and BFS from the player's
centre cell:

```python
import base64, zlib, xml.etree.ElementTree as ET
from collections import deque
r = ET.parse("assets/Levels/dungeon1.tmx").getroot()
W, H, TS = int(r.get("width")), int(r.get("height")), int(r.get("tilewidth"))
L = {l.get("name"): [int.from_bytes(d[i*4:i*4+4], "little") for i in range(W*H)]
     for l in r.findall("layer")
     for d in [zlib.decompress(base64.b64decode(l.find("data").text.strip()))]}
solid = [L["Collision"][i] != 0 or L["Overlay"][i] != 0 for i in range(W*H)]
```

Objects are placed by their **top-left** corner and are 16×16, so the cell of
interest is the one containing `(x + 8, y + 8)`.

**Watch out:** an item's own cell can be solid — Coin3 in `dungeon1` is embedded
in a wall and is only reachable from the cell above. Path *to a free cell
overlapping the item's box*, not to the item's cell.

## Art inventory

Four themed sets, of which **only set 1 is used by any level**:

| | Tiles | Enemies | Items | Player | Minimap |
|---|---|---|---|---|---|
| 1 Dungeon | `1-Dungeon`, `1-Cave`, `Floor` | `1-Monsters-Animated` | `1-Items` | `1-Heroes(-Animated)` | `1-Mini-Map` |
| 2 Frozen | `2-Frozen-Cavern` | `2-Monsters-Animated-Frozen` | `2-Items-Frozen` | `2-Heroes(-Animated)` | `2-Frozen-Mini-Map` |
| 3 Haunted | `3-Haunted-Temple` | `3-Monsters-Animated-Haunted` | `3-Items-Haunted` | `3-Heroes(-Animated)` | `3-Haunted-Mini-Map` |
| 4 Magma | `4-Cave-Magma`, `4-Cave-Obsidian`, `4-Extras` | `4-Monsters-Animated-Magma` | `4-Items-Magma` | `4-Heroes(-Animated)` | `4-Magma-Mini-Map` |

There is **no minimap system**, so the minimap art is unused by design, not by
oversight.

## Two limits worth stating up front

- **All enemies are identical.** `tagObjectsByType` default-constructs
  `EnemyComponent`, and no object property can override speed, chase range or
  damage. A "fast enemy" is impossible without C++ (see `ADR-0001`).
- **Only `level1` ever loads.** The level name is hardcoded at
  `game/src/scene/in_game_scene.cpp:62`. Adding a `<Level>` to `assets/game.xml`
  does not make it reachable.

After editing a level, update `docs/design/40-niveles/<name>.md` with the
re-measured numbers.
