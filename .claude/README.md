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

Workflow skills (`gdd`, `balance`, `tmx-level`, `new-mechanic`, `playtest`)
auto-apply when relevant. Former slash commands are skills with
`disable-model-invocation: true` — invoke explicitly with `/sim`,
`/gdd-sync`, `/design-review`, `/spike`.

Do **not** duplicate these under `.cursor/skills/` or `.github/skills/`
unless a tool stops reading `.claude/skills/`.

## Claude-only

| Path | Role |
|---|---|
| `agents/` | Subagent prompts (pillar-guard, level-designer, …) |
| `hooks/` + `settings.json` | Post-edit validators, Stop drift check, SessionStart pillars |

The hook scripts are plain Node and safe to run from any agent or CI:

```
node .claude/hooks/validate-tmx.mjs
node .claude/hooks/validate-gdd.mjs
node .claude/hooks/design-drift.mjs
```

Cursor/Copilot do not load `.claude/settings.json`; wire the same scripts
into their hook systems or git/CI if you want the same gates.
