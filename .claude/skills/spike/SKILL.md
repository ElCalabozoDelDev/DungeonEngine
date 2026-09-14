---
name: spike
description: >-
  Turn a game idea into a costed one-pager ending in a draft ADR. Use when
  evaluating a feature idea, costing a spike, or deciding whether to build
  something.
disable-model-invocation: true
---

Evaluate the idea the user gave. Produce a one-pager. Investigate the repository
first — every section below must be grounded in what is actually there, not in
what a game like this usually has.

**1. What it is.** Restate the idea in two sentences, concretely enough to
disagree with. If it is ambiguous, pick the most plausible reading and say which
one you picked.

**2. Pillars.** Which of the four in `docs/design/00-pilares.md` it serves, which
it is neutral to, and which it contradicts. Be direct: a player attack
contradicts PILAR-01, smart pathfinding quietly kills PILAR-03.

**3. Art.** Does it need new art? Check `assets/` first — there are four complete
themed sets and three are entirely unused, so the answer is often no. If it needs
a new sprite, say exactly what.

**4. Cost.** The concrete files it would touch, with paths. Flag the expensive
shapes explicitly:
   - anything needing per-instance enemy or item values is blocked by
     `ADR-0001` — the `.tmj` cannot override component defaults
   - anything needing a second level is blocked by the hardcoded `"arena"` at
     `game/src/scene/in_game_scene.cpp:62`
   - a new gameplay rule that must not depend on frame rate goes in
     `addFixedSystem`, which is a different cost from a render-time effect
   - anything in the engine must not name a gameplay component

**5. Risk.** How it could go wrong, and what would be hard to undo. Measurable
with `--sim` or only by playing it?

**6. Verdict.** Recommend for, against, or "not yet, because X first". Take a
position — a survey of considerations is not a verdict.

**7. Draft ADR.** If it is worth doing or worth formally rejecting, write a draft
for `docs/design/90-decisiones/NNNN-<slug>.md` using `_plantillas/adr.md`, with
the next free number. Leave `estado: propuesto`. **Show it — do not write the
file** unless asked.
