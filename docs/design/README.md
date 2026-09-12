# Documentos de diseño

Esta carpeta es la **fuente de verdad de diseño** de DungeonEngine. El código
dice *cómo* funciona el juego; esto dice *por qué* es así y qué se pretende que
sea.

`README.md` y `AGENTS.md` en la raíz siguen siendo la referencia técnica: build,
arquitectura, convenciones (`CLAUDE.md` solo apunta a eso para Claude Code).
Aquí no se documenta el engine, se documenta el juego.

## Cómo leerlo

| Carpeta | Contiene |
|---|---|
| `00-pilares.md` | Los pilares. El filtro para decir que no. |
| `10-core-loop.md` | El minuto a minuto de una partida. |
| `20-sistemas/` | Un documento por sistema de gameplay, espejo de `game/src/systems/`. |
| `30-biomas/` | Los cuatro temas visuales y qué promete cada uno. |
| `40-niveles/` | La intención de cada nivel, antes y después del `.tmx`. |
| `50-balance/` | La tabla canónica de números. |
| `90-decisiones/` | ADRs: qué se descartó y por qué. |
| `_plantillas/` | Plantillas para documentos nuevos. |

## Qué es normativo y qué no

Cada documento declara un `estado` en su front-matter:

| `estado` | Significa |
|---|---|
| `implementado` | Está en el código y el documento lo describe con fidelidad. |
| `parcial` | Parte está en el código; el documento marca explícitamente qué falta. |
| `propuesto` | No existe en el código. Es intención, no descripción. |
| `descartado` | Se consideró y se rechazó. Se conserva con el porqué. |

**Solo `implementado` y `parcial` describen el juego que existe.** Todo lo demás
es material de trabajo. Si lees un documento y quieres saber si es real, mira el
`estado` antes que el texto.

## Front-matter obligatorio

```yaml
---
id: SYS-CMB
titulo: Combate
estado: implementado
pilares: [PILAR-01, PILAR-02]
codigo:
  - game/src/systems/combat_system.cpp
  - game/include/game/components/health_component.hpp
---
```

- `id` — único en todo el árbol. Es cómo se enlaza y cómo se referencia desde el
  código y desde los commits.
- `codigo` — los archivos que implementan lo que describe el documento. Es lo que
  permite detectar automáticamente cuándo el código avanzó y el documento no.
  Documentos `propuesto` llevan la lista vacía.

Los enlaces entre documentos se escriben `[[SYS-CMB]]`, con el `id`, no con la
ruta: los archivos se mueven, los ids no.

## Reglas

1. **Si tocas un sistema, actualiza su documento en el mismo cambio.** Un hook
   te lo recuerda, pero no te obliga.
2. **Si cambias un número de balance**, cambia los tres sitios:
   el default del componente, `tests/test_balance.cpp` y
   [[BAL-01]]. El test existe para que no se te olvide.
3. **Si una idea no cabe en un pilar, no cabe en el juego.** Ese es el trabajo de
   `00-pilares.md`; si te descubres peleando con él, discute el pilar en un ADR
   en vez de ignorarlo.
