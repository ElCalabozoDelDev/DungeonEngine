---
name: gdd
description: Write or update the game design documents in docs/design/ — pillars, systems, biomes, levels, balance tables and ADRs. Covers the required front-matter, the stable-ID scheme, the estado vocabulary and the traceability contract that links each document to the code it describes. Use when adding or editing anything under docs/design/, when a gameplay change needs its design doc updated, or when asked about the game's design rather than its implementation.
---

`docs/design/` is the source of truth for *why* the game is the way it is.
`README.md` and `CLAUDE.md` cover the technical side; design docs never document
the engine.

**The documents are written in Spanish.** This is deliberate: they are the user's
own creative material. Code, comments, skills and agents stay in English, per
`CLAUDE.md`.

## Required front-matter

Every `.md` under `docs/design/` (except `README.md` and `_plantillas/`) starts
with:

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

- **`id`** — unique across the whole tree. Prefixes in use: `PILARES`, `LOOP-`,
  `SYS-`, `BIOMA-`, `NIVEL-`, `BAL-`, `ADR-`. Never reuse or renumber an id;
  documents get moved, ids do not.
- **`estado`** — exactly one of `implementado`, `parcial`, `propuesto`,
  `descartado`. Only the first two describe the game that exists. **Getting this
  wrong is the worst failure mode of the whole system**: a `propuesto` document
  read as description invents a game that isn't there.
- **`codigo`** — every file that implements what the document describes. This is
  the traceability link a hook uses to notice when code moved and the document
  did not. `propuesto` documents carry an empty list.

Cross-references are written `[[SYS-CMB]]` — by id, never by path.

A document that owns several link targets declares them with `subids`.
`00-pilares.md` is the only current case: its `id` is `PILARES` and it declares
`subids: [PILAR-01, PILAR-02, PILAR-03, PILAR-04]`, so `[[PILAR-01]]` resolves
to a section rather than to a file of its own. The validating hook rejects any
`[[link]]` that is neither an `id` nor a declared `subid`.

## Where things go

| Content | File |
|---|---|
| A design principle | `00-pilares.md` (the pillars are closed; adding one needs an ADR) |
| How a run plays out | `10-core-loop.md` |
| One gameplay system | `20-sistemas/<name>.md`, mirroring `game/src/systems/` |
| A visual theme and its rule | `30-biomas/0N-<name>.md` |
| A level's intent | `40-niveles/<tmx-name>.md` |
| A tuning number | `50-balance/tablas.md` — and `tests/test_balance.cpp` |
| A decision with alternatives | `90-decisiones/NNNN-<slug>.md` |

Start from `_plantillas/`. Never invent a new top-level folder.

## Rules that matter

1. **Describe observable behaviour, not implementation.** If a paragraph only
   makes sense after reading the code, rewrite it.
2. **Measure, don't estimate.** Level geometry, path lengths and step counts are
   computed by parsing the `.tmx` CSV layers and running a BFS — never
   eyeballed. See the `tmx-level` skill.
3. **Write down what is deliberate.** The most valuable paragraphs are the ones
   that stop someone "fixing" an intentional limitation six months from now — the
   dumb straight-line enemy AI is the standing example.
4. **Separate "won't do" from "hasn't been done yet".** They read the same in a
   *Limitaciones* section and mean opposite things.
5. **When the code and the document disagree, the code wins** — then fix the
   document in the same change, and say so.
6. **Don't fill gaps with invention.** If the game has no win condition, the
   document says it has no win condition. Unknowns get `estado: propuesto` or an
   open question, never a confident guess.
