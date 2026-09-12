// PostToolUse hook: sanity-checks a design document after it is written.
//
// Advisory only -- it never blocks an edit. Everything it reports is something
// that silently breaks the traceability the design tree depends on: a missing
// id, a duplicate one, a `codigo:` path that no longer exists, or a [[link]]
// pointing at nothing.

import { existsSync, readFileSync } from "node:fs";
import { join, relative, sep } from "node:path";
import {
    ROOT,
    DESIGN_DIR,
    ESTADOS,
    designFiles,
    frontMatter,
    readStdin,
    warn,
} from "./gdd-lib.mjs";

const root = ROOT;

let payload;
try {
    payload = JSON.parse(await readStdin());
} catch {
    warn(null);
}

const filePath = payload?.tool_input?.file_path;
if (typeof filePath !== "string") warn(null);

const rel = relative(root, filePath).split(sep).join("/");
if (!rel.startsWith(`${DESIGN_DIR}/`) || !rel.endsWith(".md")) warn(null);
if (rel.includes("/_plantillas/") || rel.endsWith("/README.md")) warn(null);
if (!existsSync(filePath)) warn(null);

const findings = [];
let text = "";
try {
    text = readFileSync(filePath, "utf8");
} catch {
    warn(null);
}

const meta = frontMatter(text);

if (meta === null) {
    findings.push("no tiene front-matter");
} else {
    if (meta.id === undefined) findings.push("falta `id`");
    if (meta.titulo === undefined) findings.push("falta `titulo`");

    if (meta.estado === undefined) {
        findings.push("falta `estado`");
    } else if (!ESTADOS.includes(meta.estado)) {
        findings.push(
            `estado \`${meta.estado}\` no válido (${ESTADOS.join(" | ")})`,
        );
    }

    // Every other document, so we can check id uniqueness and link targets.
    const ids = new Map();
    for (const other of designFiles(root)) {
        const otherRel = relative(root, other).split(sep).join("/");
        if (otherRel === rel) continue;
        try {
            const otherMeta = frontMatter(readFileSync(other, "utf8"));
            if (otherMeta?.id !== undefined) ids.set(otherMeta.id, otherRel);
            // `subids` are section-level link targets owned by a document.
            for (const sub of Array.isArray(otherMeta?.subids)
                ? otherMeta.subids
                : []) {
                if (!ids.has(sub)) ids.set(sub, otherRel);
            }
        } catch {
            // An unreadable sibling is not this document's problem.
        }
    }

    if (meta.id !== undefined && ids.has(meta.id)) {
        findings.push(`el id \`${meta.id}\` ya existe en ${ids.get(meta.id)}`);
    }

    const codigo = Array.isArray(meta.codigo) ? meta.codigo : [];
    for (const path of codigo) {
        if (!existsSync(join(root, path))) {
            findings.push(`\`codigo:\` apunta a ${path}, que no existe`);
        }
    }

    if (meta.estado === "implementado" && codigo.length === 0) {
        findings.push("estado `implementado` pero `codigo:` está vacío");
    }

    const known = new Set([
        ...ids.keys(),
        meta.id,
        ...(Array.isArray(meta.subids) ? meta.subids : []),
    ]);
    const missing = new Set();
    for (const [, id] of text.matchAll(/\[\[([A-Za-z0-9-]+)\]\]/g)) {
        if (!known.has(id)) missing.add(id);
    }
    for (const id of missing) {
        findings.push(`enlace \`[[${id}]]\` no resuelve a ningún documento`);
    }
}

if (findings.length === 0) warn(null);

warn(
    `GDD · ${rel}\n` +
        findings.map((f) => `  · ${f}`).join("\n") +
        "\n  (aviso, no bloquea)",
);
