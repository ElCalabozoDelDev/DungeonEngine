---
id: SYS-ITM
titulo: Ítems
estado: parcial
pilares: [PILAR-02, PILAR-03]
codigo:
  - game/include/game/components/item_component.hpp
  - game/src/systems/combat_system.cpp
  - game/src/scene/in_game_scene.cpp
---

# Ítems

## Qué hace

Un ítem es una entidad con `ItemComponent{value = 1}` y un sprite. Al solaparlo,
suma su `value` al contador, suena y desaparece. La recogida vive en
[[SYS-CMB]], no en un sistema propio.

`itemsTotal` se cuenta al cargar el nivel, contando los objetos con
`type="Item"`.

## Lo que son hoy: un contador

No hay tipos de ítem, ni efectos, ni inventario, ni curación, ni rareza. Todos
valen 1 y todos hacen lo mismo. `value` existe como campo pero **nunca es
distinto de 1**, porque nada puede sobreescribir el valor por defecto.

Y como no hay condición de victoria ([[LOOP-01]]), recogerlos todos tampoco hace
nada. Son, literalmente, un número que sube.

`estado: parcial` por eso: el mecanismo está implementado, el propósito no.

## La tentación a evitar

Lo obvio sería convertirlos en pociones, llaves y armas. Dos tercios de eso
chocan con los pilares: un arma contradice [[PILAR-01]], y tres indicadores nuevos
en el HUD erosionan [[PILAR-02]].

Lo que **sí** encaja son ítems que cambian cómo te mueves o qué rutas existen —
una llave que abre un paso, algo que te deja cruzar un bioma hostil. Es decir,
ítems al servicio de [[PILAR-03]]. Cualquier propuesta concreta va a un ADR.

## Colocación

Un ítem es un punto de interés: dice "ven aquí" y el diseño del nivel decide cuánto
cuesta. En [[NIVEL-DUNGEON1]] los tres están colocados de formas distintas y solo
una es accidental.
