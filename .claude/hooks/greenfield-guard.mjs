// PreToolUse hook for the gdd-architect subagent (wired in its frontmatter,
// so it only runs while that agent is active).
//
// When the design tree describes no existing game (see isGreenfield), the
// repository still holds the template's worked example -- Dungeon Slime's
// code, levels, tests and git history. An interview for a new game must not
// be grounded in any of that, and an instruction alone has proved too easy
// to talk past. So in that mode reads are limited to an allowlist:
//
//   docs/design/          the drafts being written (minus the old README)
//   .claude/skills/gdd/   the front-matter contract
//   engine/include/       what the engine can do, as a constraint
//
// Exit 2 blocks the call and shows stderr to the agent. Never append
// `|| true` or `2>/dev/null` to the command that runs this: both would turn
// the block into a silent pass.

import { relative, resolve, sep } from "node:path";
import { ROOT, isGreenfield, readStdin } from "./gdd-lib.mjs";

const ALLOWED = ["docs/design", ".claude/skills/gdd", "engine/include"];
const DENIED = ["docs/design/readme.md"];

let payload;
try {
    payload = JSON.parse(await readStdin());
} catch {
    process.exit(0); // Malformed payload: not ours to judge.
}

if (!isGreenfield(ROOT)) process.exit(0);

/** Repository-relative, forward-slashed, lower-cased ("" is the root). */
const relOf = (p) =>
    relative(ROOT, resolve(ROOT, p)).split(sep).join("/").toLowerCase();

const allowed = (rel) =>
    !DENIED.includes(rel) &&
    ALLOWED.some((pre) => rel === pre || rel.startsWith(`${pre}/`));

/** The literal directory part of a glob, before its first wildcard. */
const literalPrefix = (pattern) => {
    const out = [];
    for (const segment of pattern.replace(/\\/g, "/").split("/")) {
        if (/[*?[\]{}]/.test(segment)) break;
        out.push(segment);
    }
    return out.join("/");
};

const input = payload.tool_input ?? {};
let target;
switch (payload.tool_name) {
    case "Read":
        target = input.file_path ?? "";
        break;
    case "Glob":
        target = resolve(
            resolve(ROOT, input.path ?? "."),
            literalPrefix(input.pattern ?? ""),
        );
        break;
    case "Grep":
        target = input.path ?? ".";
        break;
    default:
        process.exit(0);
}

const rel = relOf(target);
if (allowed(rel)) process.exit(0);

process.stderr.write(
    `Blocked: new-project mode. No design document is implementado or ` +
        `parcial, so the code, levels, tests and git history in this ` +
        `repository belong to the template's example game, not to the game ` +
        `being designed. '${rel || "."}' is off limits. Readable: ` +
        `${ALLOWED.map((p) => `${p}/`).join(", ")} (except ` +
        `docs/design/README.md). Pass an explicit path inside one of them; ` +
        `anything else, ask the user.\n`,
);
process.exit(2);
