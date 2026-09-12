---
id: ADR-0002
titulo: Combate ofensivo del jugador (opción B)
estado: implementado
pilares: [PILAR-01]
codigo: []
---

# ADR-0002 — Combate ofensivo del jugador (opción B)

**Fecha:** 2026-09-12
**Estado:** aceptado

## Contexto

[[PILAR-01]] estaba **derivado del juego que existía**: no había input de
ataque, los enemigos no tenían vida y no morían; el peligro se resolvía solo
con posición y velocidad (jugador 3,33× más rápido).

El core loop acordado ([[LOOP-01]]) es un roguelike de descenso con objetivo y
retorno. El brief exige **combate/ataque básico** como parte del riesgo de
explorar, no solo daño por contacto. Eso no es un matiz de balance: cambia la
propuesta del pilar.

## Opciones consideradas

**A. Mantener el pilar antiguo.** Solo contacto + esquiva. Descartada: no
cumple el brief.

**B. Relajar el pilar: combate ofensivo completo.** El jugador ataca; los
enemigos tienen vida y pueden morir. La evasión deja de ser la única
respuesta, pero la posición y la geometría siguen importando ([[PILAR-03]]).

**C. Ataque no letal / utilidad.** Empuje, aturdimiento, etc. Descartada: el
brief pide combate ofensivo real.

## Decisión

**Opción B.** [[PILAR-01]] se reescribe en este mismo cambio. El combate
ofensivo forma parte del riesgo de explorar; no sustituye la lectura del
espacio ni justifica pathfinding inteligente en enemigos.

La forma concreta del ataque (cuerpo a cuerpo, proyectil, cadencia, daño) se
especifica en [[SYS-CMB]] como `propuesto` hasta existir en código. Los
números irán a [[BAL-01]] cuando existan defaults.

## Consecuencias

- [[SYS-CMB]] pasa a `parcial`: daño por contacto sigue; ataque y vida enemiga
  faltan.
- [[SYS-ENE]] deberá admitir vida / muerte sin “arreglar” la IA a pathfinding
  ([[PILAR-03]] sigue vigente).
- El HUD ([[PILAR-02]]) no gana un tercer indicador solo por tener arma: si hace
  falta munición o similar, hay que justificarlo o expresarlo en el mundo.
- `--sim` tendrá que modelar combate o seguir midiendo políticas sin ataque
  hasta que exista input de ataque en la política.

## Cuándo revisar esto

Si el combate ofensivo convierte la geometría en decoración (matar siempre es
más barato que rodear), o si el HUD se hincha por el arma. Entonces reabrir,
no ignorar el pilar.
