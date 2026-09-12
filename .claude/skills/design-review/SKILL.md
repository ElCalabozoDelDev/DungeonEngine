---
name: design-review
description: >-
  Review the current change against the game's design pillars. Use when asked
  for a design review, pillar check, or whether a branch/PR fits the GDD.
disable-model-invocation: true
---

Review the change against DungeonEngine's design pillars.

Target: whatever the user named (branch, PR, or nothing). If empty, review the
working tree (`git diff` plus `git diff --staged`); if a branch name, review
`git diff main...<branch>`.

Prefer the **pillar-guard** subagent/agent if the host supports it (Claude Code
subagent or Cursor Task). Pass the diff and instruct it to report conflicts
against `docs/design/00-pilares.md`. If no subagent is available, follow
`.claude/agents/pillar-guard.md` yourself — read-only, report only.

Relay its findings — it reports, it never edits.

Then add one thing it cannot: whether the change **should have touched a design
document and did not**. A gameplay system changed without its
`docs/design/20-sistemas/` file updated is drift in progress, even when no pillar
is violated.

Finish with a plain verdict: clean, or the conflicts most serious first. A pillar
conflict is information for the user to weigh, not a veto — if they want the
change anyway, the right next step is an ADR under `docs/design/90-decisiones/`,
not silence.
