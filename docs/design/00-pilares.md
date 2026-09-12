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

Los pilares son **cerrados**: añadir, quitar o reescribir uno exige un ADR en
`90-decisiones/`. [[PILAR-01]] se reescribió vía [[ADR-0002]] (aceptado).
[[PILAR-02]] y la velocidad relativa del jugador siguen anclados al código;
[[PILAR-03]] es `parcial`; [[PILAR-04]] es `propuesto`.

---

## PILAR-01 — Posición y combate bajo riesgo

`estado: parcial`

El peligro se resuelve **moviéndose y peleando**, no solo sobreviviendo al
contacto. El jugador **tiene (debe tener) un ataque propio**; los enemigos
**pueden tener vida y morir**. El combate es parte del riesgo de explorar
([[LOOP-01]]), no un minijuego aparte ni un contador de kills como meta.

Lo que **sigue siendo deliberado** (y ya está en código):

- El jugador se mueve a 200 px/s contra los 60 px/s de un enemigo: **3,3 veces
  más rápido**. En línea recta el enemigo no te alcanza; te atrapa si te
  encierras o te rodean.
- El daño por contacto sigue existiendo ([[SYS-CMB]]): chocar duele.

Lo que **aún no está en código** (por eso `parcial`): el ataque del jugador,
la vida enemiga y las muertes. Hasta que existan, el juego jugable sigue siendo
solo evasión + contacto — pero **ya no es la propuesta de diseño**. Ver
[[SYS-CMB]] y [[ADR-0002]].

**Implicación práctica:** un enemigo nuevo es interesante si cambia *cómo debes
moverte o pelear* (te corta el paso, te obliga a rodear, te niega una ruta, te
fuerza a gastar un golpe en un mal momento). Pegar más fuerte sin más no basta.
Matar al bloqueador no debe ser siempre más barato que usar la geometría
([[PILAR-03]]).

---

## PILAR-02 — Legibilidad inmediata

`estado: implementado`

Tiles de 16 px con zoom de cámara ×3: todo se lee grande y sin ambigüedad. La
vida son corazones literales (`[#][#][ ]`), no una barra ni un número. El HUD
tiene dos cosas y nada más: corazones e ítems.

El jugador debe poder saber, en cualquier fotograma y sin pensar, cuánta vida le
queda, qué lleva (puntuación / objetivo, ver [[SYS-ITM]]) y dónde está el
peligro **visible**.

**Implicación práctica:** cualquier información nueva compite por esas dos
líneas de HUD. Si no cabe en un vistazo, no cabe. El FoW ([[SYS-FOV]]) informa
ocultando el mapa, no añadiendo un tercer indicador.

---

## PILAR-03 — El nivel es el enemigo

`estado: parcial`

La dificultad real no la ponen solo los números de combate: la pone la
**geometría**. En el artefacto histórico [[NIVEL-DUNGEON1]] eso se veía en
pasillos de un tile y embudos; el camino adelante es el generador
procedimental ([[SYS-PROC]]), que debe hablar el mismo vocabulario: embudos,
acantilados jugables ([[SYS-VERT]]), rutas caras de deshacer.

Un enemigo que persigue en línea recta es trivial en campo abierto y letal en
un pasillo. El diseño de nivel (o de la generación) es, literalmente, el
diseño de dificultad.

`parcial` porque hoy esto es cierto por accidente en un `.tmx` deprecado para
el core loop, y aún no hay reglas de generación que lo hagan a propósito.

**Implicación práctica:** antes de añadir un enemigo nuevo o subir el daño,
pregúntate si el efecto se consigue moviendo una pared o un acantilado.
**Enemigos con pathfinding inteligente** hacen irrelevante este pilar: si el
enemigo rodea las paredes, la geometría deja de ser herramienta del jugador.

---

## PILAR-04 — Cada bioma es una promesa distinta

`estado: propuesto`

Hay arte para cuatro biomas —Dungeon, Frozen, Haunted, Magma— con su propio
set de tiles, monstruos, ítems y minimapa. Hoy el cargador handcrafted solo
usa el primero vía [[NIVEL-DUNGEON1]]; el core loop futuro elige bioma a través
de [[SYS-PROC]], no extendiendo ese `.tmx`.

La propuesta es que cada bioma no sea un reskin sino un cambio de regla: el
mismo core loop bajo una restricción distinta. Detalle en [[BIOMA-DUNGEON]],
[[BIOMA-FROZEN]], [[BIOMA-HAUNTED]] y [[BIOMA-MAGMA]].

`propuesto`: las reglas distintas por bioma no existen en el código.

---

## Lo que los pilares dejan fuera

- **Pathfinding inteligente en enemigos** — contradice [[PILAR-03]].
- **Un HUD con más de dos indicadores** — erosiona [[PILAR-02]] (salvo
  rediseño consciente con ADR).
- **Victoria que no exija volver a la entrada con el objetivo** — fuera de
  [[LOOP-01]] acordado.
- **Seguir construyendo el core loop sobre `dungeon1.tmx`** — descartado;
  ver [[SYS-PROC]] y [[NIVEL-DUNGEON1]].
- **Inventar FoW desde cero sin mirar algoritmos/libs existentes** — ver
  [[SYS-FOV]]; el recorte de cámara no es FoW.

---

## Alineación con el brief (cerrada)

| Intención | Estado respecto a pilares |
|---|---|
| Descenso, objetivo, volver a la **entrada** | [[LOOP-01]] propuesto; compatible |
| N pisos fijos, objetivo en profundidad | [[SYS-STAIRS]] / [[SYS-PROC]] |
| Procedimental (no seguir con dungeon1) | [[SYS-PROC]]; [[PILAR-03]] |
| Combate ofensivo | [[ADR-0002]] aceptado; [[PILAR-01]] reescrito |
| FoW: visible ahora + explorado atenuado | [[SYS-FOV]] propuesto |
| Acantilados jugables (caer ≠ muerte) | [[SYS-VERT]]; [[PILAR-03]] |
| Biomas como variedad | [[PILAR-04]] |
