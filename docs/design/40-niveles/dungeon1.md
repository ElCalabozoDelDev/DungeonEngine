---
id: NIVEL-DUNGEON1
titulo: Dungeon 1
estado: implementado
pilares: [PILAR-01, PILAR-02, PILAR-03]
codigo:
  - assets/Levels/dungeon1.tmx
---

# Dungeon 1

El único nivel que existe. Bioma [[BIOMA-DUNGEON]].

## Datos medidos

Decodificando las capas de tiles y corriendo un BFS de 4 vecinos desde el punto
de aparición (una celda es sólida si tiene tile en `Collision` **o** en
`Overlay`):

| | |
|---|---|
| Tamaño | 26 × 46 tiles de 16 px = **416 × 736 px** |
| Celdas transitables | **659 de 1196** (55 %) |
| Aparición del jugador | `(182, 715)` → celda `(11, 45)`, abajo del todo |
| Enemigos | 3, en celdas `(3, 41)`, `(5, 38)`, `(1, 3)` |
| Ítems | 3 |
| Recorrido óptimo de los 3 ítems | **53 tiles = 848 px = 4,24 s = 254 pasos fijos** (teórico: el tercero no se puede recoger, ver abajo) |
| Recorrido real medido con `--sim` | 2 de 3 ítems en **164 pasos**, 530 px, 2 de daño |

## Estructura: una subida

Apareces en la fila 45 —la última— y el último ítem está en la fila 24. El nivel
es **una ascensión**: el progreso es hacia arriba y hacia el fondo del mapa, con
los enemigos escalonados en el camino.

El tercer enemigo, el "Mimic", está en la celda `(1, 3)`: arriba del todo, casi
en la esquina opuesta, y **fuera de la ruta de los tres ítems**. Con 120 px de
rango de persecución ([[SYS-ENE]]) no llega a activarse nunca en un recorrido
directo. Es decoración funcional: solo lo encuentra quien explora de más.

## Los tres ítems, y lo que enseña cada uno

| Ítem | Celda | Distancia | Qué hace |
|---|---|---|---|
| Coin1 | `(9, 41)` | 6 tiles | El tutorial. Casi gratis, en línea recta, enseña que los ítems existen. |
| Coin2 | `(4, 33)` | +17 tiles | **El filtro.** |
| Coin3 | `(19, 25)` | — | **Imposible de recoger.** Ver abajo. |

### Coin2 es el corazón del nivel

La ruta pasa obligatoriamente por la celda `(3, 35)`, que es **un corredor
vertical de exactamente un tile de ancho**. Y antes, por `(8, 42)`, un paso
horizontal de un tile de alto.

El cuerpo del jugador mide 16×16: exactamente lo que mide el hueco. Y el
movimiento es libre, no por casillas ([[SYS-MOV]]) — de hecho apareces en x=182,
que ni siquiera es múltiplo de 16. **Atravesar ese hueco exige alineación de
píxel.**

Además, el camino pasa por las salas de los dos zombis. Es decir: te obligan a
hacer un movimiento de precisión mientras algo te persigue. Eso es [[PILAR-03]]
funcionando, y es el único momento del nivel donde el juego es realmente duro.

### Coin3 es imposible de recoger, y por tanto el nivel no se puede completar

**Esto no es una curiosidad: es un defecto del nivel.** Lo descubrió `--sim`, y
está demostrado, no supuesto.

Coin3 está en `(300, 400)`, así que su caja ocupa x 300–316, y 400–416: las
celdas `(18, 25)` y `(19, 25)`. **Las dos son sólidas.** Las únicas celdas libres
cercanas son `(18, 24)` y `(19, 24)`, en la fila de encima.

El jugador mide exactamente un tile, así que la colisión nunca le deja solapar
una celda sólida: como mucho queda pegado con su borde inferior en y=400, que es
justo el borde superior del ítem. Y `Box::intersects` es **estricto** —
`getBottom() <= box.m_top` cuenta como *no* solapar—, de modo que tocarse no
recoge nada.

Es decir: **no existe ninguna posición desde la que un jugador, humano o
scripteado, pueda recoger Coin3.** Y como el HUD presenta los ítems como un
progreso (`Items 2 / 3`), el nivel enseña una meta que no se puede alcanzar.

Corrida de `--sim` que lo demuestra:

```
sim: outcome=unreachable
sim: items=2/3 unreachable=1 pickups_at=0.4000,2.7167
sim: damage_taken=2 hits=2 final_health=3 first_hit_at=0.7833
```

**La corrección es trivial** —mover el objeto a una celda libre, por ejemplo
`(19, 24)` → `(304, 384)`— pero es una decisión de diseño, no una errata que
arreglar de paso: cambia el recorrido, la dificultad y las cifras de arriba. Por
eso queda documentada y sin tocar.

Mientras siga así, cualquier medición de este nivel debe leerse sabiendo que el
tercer ítem no cuenta.

## Lo que el nivel no usa

- Un solo tileset (`1-Dungeon.png`). `1-Cave.png` y `Floor.png` están en
  `assets/Tiles/` sin usar.
- Tres texturas registradas: `items`, `monster1`, `player`. Los sets de los otros
  biomas no se cargan.
- Los nombres de objeto ("Zombie1", "Mimic") **no los lee nadie**: solo cuenta el
  atributo `type`. Son comentarios para quien abra Tiled.
