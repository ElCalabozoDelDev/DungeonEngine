// PreToolUse hook for the gdd-architect subagent (wired in its frontmatter,
// so it only runs while that agent is active).
//
// The repository is a template: until the user says otherwise, its code,
// levels, tests and git history are the worked example (Dungeon Slime), not
// the game being designed. The mode is the user's answer, recorded in
// docs/design/proyecto.json (see projectMode). Unless it says `existente` --
// that is, when it says `nuevo` or the question is still unanswered -- reads
// are limited to an allowlist, because an instruction alone has proved too
// easy to talk past:
//
//   docs/design/          the drafts being written, minus the old README and
//                         any document still `implementado`/`parcial`, which
//                         in this mode can only describe the example
//   .claude/skills/gdd/   the front-matter contract
//   engine/include/       what the engine can do, as a constraint
//
// Exit 2 blocks the call and shows stderr to the agent. Never append
// `|| true` or `2>/dev/null` to the command that runs this: both would turn
// the block into a silent pass.

import { readFileSync } from "node:fs";
import { relative, resolve, sep } from "node:path";
import {
    ROOT,
    PROJECT_FILE,
    designFiles,
    frontMatter,
    projectMode,
    readStdin,
} from "./gdd-lib.mjs";

const ALLOWED = ["docs/design", ".claude/skills/gdd", "engine/include"];
const DENIED = ["docs/design/readme.md"];

let payload;
try {
    payload = JSON.parse(await readStdin());
} catch {
    process.exit(0); // Malformed payload: not ours to judge.
}

const mode = projectMode(ROOT);
if (mode === "existente") process.exit(0);

/** Repository-relative, forward-slashed, lower-cased ("" is the root). */
const relOf = (p) =>
    relative(ROOT, resolve(ROOT, p)).split(sep).join("/").toLowerCase();

const allowed = (rel) =>
    !DENIED.includes(rel) &&
    ALLOWED.some((pre) => rel === pre || rel.startsWith(`${pre}/`));

/** A design document that claims to describe code that exists. */
const describesExistingCode = (absolute) => {
    try {
        const estado = frontMatter(readFileSync(absolute, "utf8"))?.estado;
        return estado === "implementado" || estado === "parcial";
    } catch {
        return false; // Missing or a directory: nothing to leak.
    }
};

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
        target = resolve(ROOT, input.file_path ?? "");
        break;
    case "Glob":
        target = resolve(
            resolve(ROOT, input.path ?? "."),
            literalPrefix(input.pattern ?? ""),
        );
        break;
    case "Grep":
        target = resolve(ROOT, input.path ?? ".");
        break;
    default:
        process.exit(0);
}

const rel = relOf(target);
const why =
    mode === null
        ? `The user has not said yet whether this is a new game ` +
          `(${PROJECT_FILE} is missing). Ask them before reading anything ` +
          `else, then record the answer in that file.`
        : `${PROJECT_FILE} says "nuevo": the code, levels, tests, git ` +
          `history and any implementado/parcial document belong to the ` +
          `template's example game, not to the game being designed.`;

if (allowed(rel)) {
    // Reading contents of a document -- directly, or by grepping a folder
    // that holds one -- is where an example-game document would leak.
    // Globbing only lists names, which is how the mode check starts.
    const inDesign = rel === "docs/design" || rel.startsWith("docs/design/");
    const leaks =
        inDesign &&
        (payload.tool_name === "Grep"
            ? designFiles(ROOT).some(
                  (f) =>
                      (relOf(f) + "/").startsWith(rel + "/") &&
                      describesExistingCode(f),
              )
            : payload.tool_name === "Read" && describesExistingCode(target));
    if (!leaks) process.exit(0);
    process.stderr.write(
        `Blocked: '${rel}' is, or contains, a document marked ` +
            `implementado/parcial, so it describes code that already exists ` +
            `-- the example game, in this mode. ` +
            `${why}\n`,
    );
    process.exit(2);
}

process.stderr.write(
    `Blocked: '${rel || "."}' is off limits. ${why} Readable: ` +
        `${ALLOWED.map((p) => `${p}/`).join(", ")} (except ` +
        `docs/design/README.md). Pass an explicit path inside one of them; ` +
        `anything else, ask the user.\n`,
);
process.exit(2);
