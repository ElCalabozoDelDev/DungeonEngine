---
id: ADR-0001
titulo: Los números de balance siguen en C++, fijados por un test
estado: implementado
pilares: [PILAR-01]
codigo:
  - tests/test_balance.cpp
---

# ADR-0001 — Los números de balance siguen en C++, fijados por un test

**Fecha:** 2026-09-11
**Estado:** aceptado, revisable

## Contexto

Los valores de balance ([[BAL-01]]) viven únicamente como inicializadores por
defecto de miembros en los componentes de `game/include/game/components/`.
`InGameScene::tagObjectsByType` los construye por defecto y **no lee ninguna
propiedad del objeto de Tiled**, así que el `.tmx` no puede sobreescribirlos.

Consecuencias prácticas:

- No se puede tener un enemigo rápido y otro lento sin escribir C++.
- Ajustar un número exige recompilar.
- Documentar los valores en [[BAL-01]] crea una copia que se desincroniza en
  cuanto alguien toca un `.hpp`.

## Opciones consideradas

**A. Mover los números a `assets/game.xml`.** Ya existe `ConfigLoader` con
validación y un `Config` publicado en el contexto. Sería el sitio natural.

**B. Leer propiedades por objeto del `.tmx`.** El cargador ya tiene el bucle de
`<properties>`; solo reconoce cinco nombres. Añadir `speed`, `chaseRange`, etc.
daría tuning por instancia, que es lo que de verdad quiere el diseño de niveles.

**C. Dejarlos en C++ y fijarlos con un test que nombre el documento.**

## Decisión

**C, por ahora.**

A y B son las soluciones correctas a medio plazo, y B es claramente la mejor para
[[PILAR-03]]: colocar en Tiled un enemigo lento que corta un pasillo es diseño de
nivel, no programación. Pero las dos son refactors con riesgo propio, tocan el
camino de carga que hoy funciona, y ninguna resuelve el problema inmediato, que
es **que el documento y el código se separen sin que nadie se entere**.

`tests/test_balance.cpp` lo resuelve hoy, cuesta veinte líneas y no toca runtime:
si cambias un valor y no actualizas [[BAL-01]], el test se pone rojo y el
comentario te dice por qué.

## Consecuencias

- El balance sigue exigiendo recompilar. Aceptado: los ciclos de build son
  cortos y `--sim` da la medición sin abrir el juego.
- Sigue sin haber tipos de enemigo. **Es la limitación más seria que queda**, y
  bloquea buena parte de [[BIOMA-FROZEN]], [[BIOMA-HAUNTED]] y [[BIOMA-MAGMA]].
- El test añade fricción deliberada a cambiar un número. Esa fricción es el
  objetivo, no un efecto secundario.

## Cuándo revisar esto

En cuanto se quiera un segundo tipo de enemigo. Ahí la opción B deja de ser una
mejora y pasa a ser un requisito: escríbase un ADR-0002 que la implemente.
