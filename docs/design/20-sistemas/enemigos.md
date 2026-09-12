---
id: SYS-ENE
titulo: Enemigos
estado: implementado
pilares: [PILAR-01, PILAR-03]
codigo:
  - game/src/systems/enemy_ai_system.cpp
  - game/include/game/components/enemy_component.hpp
---

# Enemigos

## Qué hace

Una sola regla, evaluada cada paso fijo para cada enemigo:

> Si el jugador está a 120 px o menos, muévete hacia él a 60 px/s en línea recta.
> Si no, quédate quieto.

No hay más. Ni pathfinding, ni memoria, ni histéresis de desenganche, ni
separación entre enemigos, ni ataque, ni estados.

El objetivo es **el primer jugador de la vista**, no el más cercano. Con un solo
jugador da igual, pero está escrito así.

## Por qué es tan tonto, a propósito

Un enemigo que rodease las paredes haría irrelevante [[PILAR-03]]: la geometría
dejaría de ser la defensa del jugador y el nivel se convertiría en decoración.
La persecución en línea recta es lo que convierte una esquina en una herramienta.

La consecuencia directa —los enemigos **se quedan clavados contra las paredes**
si les pones una entre medias— no es un bug que ocultar, es el mecanismo. El
jugador aprende a usar las paredes. Escribirlo aquí es lo que evita que alguien
"arregle" el pathfinding dentro de seis meses.

Los enemigos tampoco son sólidos: no colisionan entre sí ni empujan al jugador.
Se atraviesan. Solo las capas `Collision` y `Overlay` del mapa frenan a algo.

## Los tres números

| Número | Valor | Qué controla de verdad |
|---|---|---|
| `chaseRange` | 120 px = 7,5 tiles | El tamaño del "territorio" de un enemigo. Con zoom ×3 esto es, más o menos, lo que cabe en pantalla: un enemigo se activa justo cuando lo ves. |
| `speed` | 60 px/s | La proporción 1:3,33 contra el jugador ([[SYS-MOV]]). **Este número es [[PILAR-01]]**, no un parámetro de dificultad. |
| `contactDamage` | 1 | Con 5 corazones, cinco errores. |

Los tres son iguales para todos los enemigos: hoy no hay tipos. Los tres objetos
"Zombie1", "Zombie2" y "Mimic" de [[NIVEL-DUNGEON1]] se comportan de manera
idéntica y solo se distinguen por el sprite — el atributo `name` del `.tmx` no lo
lee nadie.

## Lo que falta para tener variedad

Hoy `tagObjectsByType` solo reconoce el string `"Enemy"` y construye el componente
con valores por defecto: **no hay forma de tener un enemigo rápido y otro lento**
sin tocar C++. Es la limitación más molesta para el diseño de niveles, y la razón
de que exista [[ADR-0001]].
