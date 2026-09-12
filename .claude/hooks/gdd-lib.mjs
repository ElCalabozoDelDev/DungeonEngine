// Shared helpers for the design-document hooks.
//
// The front-matter is a deliberately tiny subset of YAML -- `key: value`,
// inline `[a, b]` lists and `  - item` block lists -- so these hooks parse it
// directly rather than taking on a dependency. If the format ever outgrows
// this, that is the signal to reach for a real parser, not to extend this one.

import { readFileSync, readdirSync, statSync } from "node:fs";
import { dirname, join, relative, resolve, sep } from "node:path";
import { fileURLToPath } from "node:url";

/**
 * The repository root, derived from this file's own location rather than from
 * process.cwd(): a hook must behave the same whatever directory it is invoked
 * from, and a wrong root makes every check below silently pass.
 */
export const ROOT = resolve(dirname(fileURLToPath(import.meta.url)), "..", "..");

export const DESIGN_DIR = "docs/design";
export const ESTADOS = ["implementado", "parcial", "propuesto", "descartado"];

/** Reads stdin to completion. Hook payloads are small; no streaming needed. */
export async function readStdin() {
    const chunks = [];
    for await (const chunk of process.stdin) chunks.push(chunk);
    return Buffer.concat(chunks).toString("utf8");
}

/** Every design document, excluding the index and the templates. */
export function designFiles(root) {
    const base = join(root, DESIGN_DIR);
    const out = [];
    const walk = (dir) => {
        let entries;
        try {
            entries = readdirSync(dir);
        } catch {
            return;
        }
        for (const entry of entries) {
            const full = join(dir, entry);
            if (statSync(full).isDirectory()) {
                if (entry !== "_plantillas") walk(full);
            } else if (entry.endsWith(".md") && entry !== "README.md") {
                out.push(full);
            }
        }
    };
    walk(base);
    return out;
}

/**
 * Parses the front-matter block. Returns null when the file has none, which
 * the callers treat as a finding rather than an error.
 */
export function frontMatter(text) {
    const match = /^---\r?\n([\s\S]*?)\r?\n---/.exec(text);
    if (match === null) return null;

    const data = {};
    let listKey = null;
    for (const raw of match[1].split(/\r?\n/)) {
        const item = /^\s+-\s+(.*)$/.exec(raw);
        if (item !== null && listKey !== null) {
            data[listKey].push(item[1].trim());
            continue;
        }
        const pair = /^([A-Za-z_][A-Za-z0-9_]*):\s*(.*)$/.exec(raw);
        if (pair === null) continue;

        const [, key, value] = pair;
        if (value === "") {
            data[key] = [];
            listKey = key;
        } else if (value.startsWith("[")) {
            data[key] = value
                .replace(/^\[|\]$/g, "")
                .split(",")
                .map((s) => s.trim())
                .filter(Boolean);
            listKey = null;
        } else {
            data[key] = value.trim();
            listKey = null;
        }
    }
    return data;
}

/** id -> { file, codigo[] } for the whole tree. */
export function designIndex(root) {
    const byId = new Map();
    for (const file of designFiles(root)) {
        let meta;
        try {
            meta = frontMatter(readFileSync(file, "utf8"));
        } catch {
            continue;
        }
        if (meta?.id === undefined) continue;
        const rel = relative(root, file).split(sep).join("/");
        if (!byId.has(meta.id)) byId.set(meta.id, { file: rel, codigo: [] });
        const entry = byId.get(meta.id);
        entry.codigo = Array.isArray(meta.codigo) ? meta.codigo : [];

        // A document may own further ids for sections inside it -- the four
        // PILAR-NN live in one file. They are link targets, not documents, so
        // they carry no `codigo` of their own.
        for (const sub of Array.isArray(meta.subids) ? meta.subids : []) {
            if (!byId.has(sub)) byId.set(sub, { file: rel, codigo: [] });
        }
    }
    return byId;
}

/**
 * Emits an advisory line and exits cleanly. These hooks never block an edit:
 * `systemMessage` surfaces the warning to the user without failing the tool
 * call, which is the whole contract they were set up under.
 */
export function warn(message) {
    if (message) process.stdout.write(JSON.stringify({ systemMessage: message }));
    process.exit(0);
}
