---
description: Review the current change against the game's design pillars
argument-hint: "[branch, PR number, or nothing for the working tree]"
allowed-tools: Read, Glob, Grep, Bash(git diff:*), Bash(git log:*), Bash(git status:*)
---

Review the change against DungeonEngine's design pillars using the
**pillar-guard** subagent.

Target: $ARGUMENTS — if empty, review the working tree (`git diff` plus
`git diff --staged`); if a branch name, review `git diff main...<branch>`.

Spawn `pillar-guard` with the diff and the instruction to report conflicts
against `docs/design/00-pilares.md`. Relay its findings — it reports, it never
edits.

Then add one thing it cannot: whether the change **should have touched a design
document and did not**. A gameplay system changed without its
`docs/design/20-sistemas/` file updated is drift in progress, even when no pillar
is violated.

Finish with a plain verdict: clean, or the conflicts most serious first. A pillar
conflict is information for the user to weigh, not a veto — if they want the
change anyway, the right next step is an ADR under `docs/design/90-decisiones/`,
not silence.
