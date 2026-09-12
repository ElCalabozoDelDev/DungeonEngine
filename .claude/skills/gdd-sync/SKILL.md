---
name: gdd-sync
description: >-
  Reconcile docs/design/ against the codebase and report every drift. Use when
  asked for design sync, GDD drift, or whether documents match the code.
  Read-only unless asked to fix.
disable-model-invocation: true
---

Reconcile `docs/design/` against the codebase and report what has drifted. This
is a **read-only report** — do not fix anything unless asked afterwards.

Check, in this order:

1. **Dead `codigo:` paths.** Read the front-matter of every `.md` under
   `docs/design/` and verify each listed path still exists on disk. A path that
   moved silently breaks the traceability that the drift hook depends on.

2. **Undocumented gameplay code.** Every file under `game/src/systems/`,
   `game/include/game/components/` and `game/src/scene/` should appear in some
   document's `codigo:` list. Report the ones that appear in none — those are
   mechanics with no design record.

3. **Wrong `estado`.** Cross-check each document's claim against reality:
   - `implementado` while its `codigo:` list is empty, or the described behaviour
     is not in the code — the dangerous direction, report first.
   - `propuesto` while the code clearly implements it — harmless but stale.

4. **Broken links and duplicate ids.** Every `[[ID]]` must resolve to a document
   whose front-matter declares that `id`, and no `id` may appear twice in the
   tree.

5. **Balance drift.** Compare the numbers in `docs/design/50-balance/tablas.md`
   against the component defaults and against `tests/test_balance.cpp`. All three
   must agree. (If they disagree, the test should already be red — say so.)

6. **Recent code without recent docs.** `git log --oneline -20` plus
   `git diff --name-only main...HEAD`: gameplay changes whose design documents
   were not touched in the same range.

Report grouped by severity, most serious first, with file paths and ids. If
nothing has drifted, say exactly that in one line — do not pad the report to
look thorough.
