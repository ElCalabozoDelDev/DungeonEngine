# Agent tooling

How this repo teaches coding agents (Claude Code, Cursor, Copilot, …).

## Always-on instructions

| File | Who reads it |
|---|---|
| [`AGENTS.md`](../AGENTS.md) | Cursor, Copilot, Codex, and other AGENTS.md clients |
| [`CLAUDE.md`](../CLAUDE.md) | Claude Code (includes `@AGENTS.md` plus Claude-only notes) |

Edit **`AGENTS.md`** when changing build/architecture/style guidance. Keep
`CLAUDE.md` as the thin Claude-specific wrapper.

## Skills (portable)

`.claude/skills/<name>/SKILL.md` — Agent Skills standard. Loaded by:

- Claude Code (native)
- Cursor (compatibility path)
- GitHub Copilot (documented project skill path)

Workflow skills (`gdd`, `balance`, `tiled-level`, `new-mechanic`, `playtest`,
`implement-design`) auto-apply when relevant. `implement-design` is the path
from design documents to code: for a new game it retires the template's
example first, then implements one document at a time. Former slash commands are skills with
`disable-model-invocation: true` — invoke explicitly with `/sim`,
`/gdd-sync`, `/design-review`, `/spike`.

Do **not** duplicate these under `.cursor/skills/` or `.github/skills/`
unless a tool stops reading `.claude/skills/`.

## Claude-only

| Path | Role |
|---|---|
| `agents/` | Subagent prompts (pillar-guard, level-designer, …) |
| `hooks/` + `settings.json` | Post-edit validators, Stop drift check, SessionStart pillars |
| `hooks/greenfield-guard.mjs` | Wired in `gdd-architect`'s frontmatter, not `settings.json`: unless `docs/design/proyecto.json` says `existente`, blocks that agent's reads of the example game (`game/`, `assets/`, `tests/`, `.git/`, implemented design docs, …) |

The hook scripts are plain Node and safe to run from any agent or CI:

```
node .claude/hooks/validate-tiled.mjs
node .claude/hooks/validate-gdd.mjs
node .claude/hooks/design-drift.mjs
```

Cursor/Copilot do not load `.claude/settings.json`; wire the same scripts
into their hook systems or git/CI if you want the same gates.
