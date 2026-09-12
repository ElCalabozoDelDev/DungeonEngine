---
id: PILARES
titulo: Pilares de diseño
estado: parcial
pilares: []
subids: [PILAR-01, PILAR-02, PILAR-03, PILAR-04]
codigo: []
---

# Pilares

Cuatro frases que deciden qué entra y qué no. Cuando una idea es buena pero no
sirve a ninguno de estos pilares, la respuesta por defecto es no.

Tres de los cuatro están **derivados del juego que ya existe**: son una lectura
de los números y de las mecánicas implementadas, no una invención. El cuarto es
una propuesta y está marcado como tal.

---

## PILAR-01 — Esquivar, no pelear

`estado: implementado`

El jugador **no tiene ataque**. No hay input de ataque, los enemigos no tienen
vida y no pueden morir. Lo único que ocurre al tocar a un enemigo es que pierdes
un corazón.

Y el jugador se mueve a 200 px/s contra los 60 px/s de un enemigo: **3,3 veces
más rápido**. El enemigo nunca te alcanza en línea recta; solo te atrapa si te
encierras tú solo.

Eso no es una carencia, es la propuesta: el peligro se resuelve con posición y
lectura del espacio, no con daño por segundo. Todo lo que empuje hacia el combate
frontal —un arma, vida de enemigo, un contador de kills— contradice este pilar y
necesita un ADR antes de existir.

**Implicación práctica:** un enemigo nuevo es interesante si cambia *cómo debes
moverte* (te corta el paso, te obliga a rodear, te niega una ruta), no si pega
más fuerte.

---

## PILAR-02 — Legibilidad inmediata

`estado: implementado`

Tiles de 16 px con zoom de cámara ×3: todo se lee grande y sin ambigüedad. La
vida son corazones literales (`[#][#][ ]`), no una barra ni un número. El HUD
tiene dos cosas y nada más: corazones e ítems.

El jugador debe poder saber, en cualquier fotograma y sin pensar, cuánta vida le
queda, cuántos ítems faltan y dónde está el peligro.

**Implicación práctica:** cualquier información nueva compite por esas dos
líneas de HUD. Si no cabe en un vistazo, no cabe.

---

## PILAR-03 — El nivel es el enemigo

`estado: parcial`

La dificultad real de `dungeon1` no la ponen los tres zombis: la pone la
geometría. La ruta entre los dos primeros ítems pasa por **un hueco de un solo
tile de ancho**, y el tercer ítem está colocado *dentro* de la pared, accesible
solo desde una celda concreta.

Un enemigo que persigue a 60 px/s en línea recta es trivial en campo abierto y
letal en un pasillo del que no puedes salir. El diseño de nivel es, literalmente,
el diseño de dificultad.

`parcial` porque hoy esto es cierto por accidente más que por método: está en el
nivel que existe, pero no hay vocabulario ni reglas para hacerlo a propósito.
[[NIVEL-DUNGEON1]] empieza a escribirlo.

**Implicación práctica:** antes de añadir un enemigo nuevo, pregúntate si el
efecto que buscas se consigue moviendo una pared.

---

## PILAR-04 — Cada bioma es una promesa distinta

`estado: propuesto`

Hay arte para cuatro biomas completos —Dungeon, Frozen, Haunted, Magma— con su
propio set de tiles, sus monstruos, sus ítems y su minimapa. Hoy **solo se usa
el primero**; los otros tres están en el repositorio sin que nada los cargue.

La propuesta es que cada bioma no sea un reskin sino un cambio de regla: el mismo
core loop bajo una restricción distinta. Qué restricción concretamente es una
decisión abierta y se desarrolla en [[BIOMA-FROZEN]], [[BIOMA-HAUNTED]] y
[[BIOMA-MAGMA]].

`propuesto`: nada de esto existe en el código. Es la dirección sugerida, no una
descripción.

---

## Lo que los pilares dejan fuera

Consecuencias de lo anterior, escritas para poder señalarlas cuando aparezcan:

- **Un arma para el jugador** contradice [[PILAR-01]] de raíz.
- **Vida de enemigo y muertes** convierten la evasión en combate: mismo problema.
- **Un HUD con más de dos indicadores** erosiona PILAR-02.
- **Enemigos con pathfinding inteligente** hacen irrelevante PILAR-03: si el
  enemigo rodea las paredes, la geometría deja de ser la defensa del jugador.
  Esta es la tentación más fácil de justificar y la más dañina.
