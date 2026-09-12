// Stop hook: notices when gameplay code moved and its design document did not.
//
// It compares the changed files against every document's `codigo:` list. If a
// changed file is claimed by a document and nothing under docs/design/ was
// touched, it names the documents that fell behind.
//
// Advisory only -- it never asks the session to continue. Drift is a judgement
// call, and forcing a doc edit at the end of every turn would train people to
// ignore the message.

import { execFileSync } from "node:child_process";
import { ROOT, designIndex, warn } from "./gdd-lib.mjs";

const root = ROOT;

const git = (args) => {
    try {
        return execFileSync("git", args, {
            cwd: root,
            encoding: "utf8",
            stdio: ["ignore", "pipe", "ignore"],
        });
    } catch {
        return "";
    }
};

const changed = new Set(
    [...git(["diff", "--name-only"]).split("\n"),
     ...git(["diff", "--name-only", "--staged"]).split("\n")]
        .map((line) => line.trim())
        .filter(Boolean),
);

if (changed.size === 0) warn(null);

// Any design work at all counts as keeping up: this hook flags neglect, not
// completeness.
for (const file of changed) {
    if (file.startsWith("docs/design/")) warn(null);
}

const behind = [];
for (const [id, entry] of designIndex(root)) {
    const hits = entry.codigo.filter((path) => changed.has(path));
    if (hits.length > 0) behind.push({ id, file: entry.file, hits });
}

if (behind.length === 0) warn(null);

const lines = behind.map(
    ({ id, file, hits }) => `  · [[${id}]] (${file}) — tocado: ${hits.join(", ")}`,
);

warn(
    "Diseño desincronizado: cambiaste código de gameplay sin tocar su " +
        `documento.\n${lines.join("\n")}\n` +
        "  (aviso, no bloquea)",
);
