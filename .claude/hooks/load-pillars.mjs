// SessionStart hook: puts the design pillars and the document index into
// context, so every session starts knowing what the game is trying to be.
//
// Once per session rather than once per prompt: a UserPromptSubmit hook would
// achieve the same thing and pay the tokens on every message.

import { readFileSync } from "node:fs";
import { join } from "node:path";
import { ROOT, DESIGN_DIR, designIndex, readStdin } from "./gdd-lib.mjs";

const root = ROOT;

// Drain stdin so the caller never blocks on an unread pipe.
try {
    await readStdin();
} catch {
    // Nothing to do: the payload is not needed here.
}

const done = (context) => {
    if (context) {
        process.stdout.write(
            JSON.stringify({
                hookSpecificOutput: {
                    hookEventName: "SessionStart",
                    additionalContext: context,
                },
            }),
        );
    }
    process.exit(0);
};

let pillars = "";
try {
    pillars = readFileSync(join(root, DESIGN_DIR, "00-pilares.md"), "utf8");
} catch {
    done(null); // No design tree yet; say nothing rather than nag.
}

let index;
try {
    index = designIndex(root);
} catch {
    index = new Map();
}

const listing = [...index.entries()]
    .map(([id, entry]) => `  ${id} — ${entry.file}`)
    .sort()
    .join("\n");

done(
    `Design documents for this project live in \`${DESIGN_DIR}/\` and are the ` +
        "source of truth for game design (the READMEs cover the technical " +
        "side). They are written in Spanish; code and tooling stay English.\n\n" +
        "Read the `estado` field before trusting any of it: only " +
        "`implementado` and `parcial` describe the game that exists — " +
        "`propuesto` is intent, not description.\n\n" +
        `--- ${DESIGN_DIR}/00-pilares.md ---\n${pillars}\n` +
        `--- índice ---\n${listing}\n`,
);
