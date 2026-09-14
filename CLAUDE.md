# CLAUDE.md

@AGENTS.md

## Claude Code only

- **Hooks** in `.claude/settings.json` (clang-format on C++ edits, Tiled/GDD
  validators, design-drift on Stop, pillars on SessionStart). Scripts are
  plain Node under `.claude/hooks/` — other agents can run the same files
  manually or via their own hook systems.
- **Subagents** in `.claude/agents/` (pillar-guard, balance-analyst, etc.).
- **Skills** in `.claude/skills/` (also loaded by Cursor and Copilot).
  Former slash commands (`sim`, `gdd-sync`, `design-review`, `spike`) are
  skills with `disable-model-invocation: true` — invoke them with `/name`.
