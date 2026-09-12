---
id: SYS-CMB
titulo: Combate
estado: implementado
pilares: [PILAR-01, PILAR-02]
codigo:
  - game/src/systems/combat_system.cpp
  - game/include/game/components/health_component.hpp
  - game/include/game/components/item_component.hpp
  - game/include/game/state.hpp
---

# Combate

"Combate" es un nombre generoso: este sistema hace dos cosas, y ninguna de las
dos es atacar.

## Daño por contacto

Cada paso fijo, para el jugador:

1. Baja el temporizador de invulnerabilidad.
2. Si la caja del jugador solapa la de un enemigo y no es invulnerable: resta
   `contactDamage` (1), arranca **1,0 s de invulnerabilidad** y suena `hurt`.
3. Si la vida llega a 0, la fija en 0 y marca `gameOver`.

**Solo un golpe por cooldown**, aunque te solapen tres enemigos: el bucle corta
tras el primer impacto. Es lo que evita que quedar atrapado entre dos enemigos te
mate instantáneamente, y es lo que hace que los cuellos de botella del nivel sean
tensos en vez de injustos.

**La caja de golpe es el sprite completo**, 16×16 sin reducción ni desplazamiento.
No hay margen de gracia: si los píxeles se tocan, te golpea. Bajar esa caja un
par de píxeles es la palanca más barata que existe para suavizar el juego sin
tocar ningún número de [[BAL-01]].

## Recogida de ítems

Si el jugador solapa un ítem: suma `value` (1) a `itemsCollected`, suena
`pickup`, lo saca del índice espacial y destruye la entidad.

Se hace en dos pasadas —recolectar y luego destruir— porque destruir invalida la
iteración de la vista. Y se quita del quadtree *antes* de destruirlo, o el índice
se queda con un handle muerto.

## Lo que no hace

No hay ataque del jugador, ni vida de enemigo, ni muertes, ni cooldowns más allá
de la invulnerabilidad, ni loot, ni RNG de ningún tipo. **No hay número aleatorio
en todo el código del juego.** Una partida es completamente determinista dados
los mismos inputs — que es lo que hace posible [[BAL-01]] medir con
`--sim`.

Nada comprueba si `itemsCollected == itemsTotal`: ver el hueco de victoria en
[[LOOP-01]].
