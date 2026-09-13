// PostToolUse hook: sanity-checks a Tiled JSON (.tmj) level after it is written.
//
// Advisory only. The loader silently ignores unknown layer names and object
// types; this hook surfaces the common mistakes.

import { existsSync, readFileSync } from "node:fs";
import { ROOT, readStdin, warn } from "./gdd-lib.mjs";

const LAYERS = ["Bottom", "Overlay", "Collision"];
const TYPES = ["Player"];

let payload;
try {
    payload = JSON.parse(await readStdin());
} catch {
    warn(null);
}

const filePath = payload?.tool_input?.file_path;
if (typeof filePath !== "string" || !filePath.endsWith(".tmj")) warn(null);
if (!existsSync(filePath)) warn(null);

let map;
try {
    map = JSON.parse(readFileSync(filePath, "utf8"));
} catch (e) {
    warn(`TMJ inválido: ${e.message}`);
}

const findings = [];

if (map.type !== "map") {
    findings.push('el JSON no tiene "type": "map"');
}

for (const layer of map.layers ?? []) {
    if (layer.type === "tilelayer") {
        if (!LAYERS.includes(layer.name)) {
            findings.push(
                `capa "${layer.name}" no reconocida (esperadas: ${LAYERS.join(", ")})`,
            );
        }
        const expected = (map.width ?? 0) * (map.height ?? 0);
        if (Array.isArray(layer.data) && layer.data.length !== expected) {
            findings.push(
                `capa "${layer.name}": ${layer.data.length} gids, esperados ${expected}`,
            );
        }
    }
    if (layer.type === "objectgroup") {
        for (const obj of layer.objects ?? []) {
            const type = obj.type ?? "";
            if (type && !TYPES.includes(type)) {
                findings.push(
                    `objeto type="${type}" desconocido (esperados: ${TYPES.join(", ")})`,
                );
            }
        }
    }
}

if (findings.length === 0) warn(null);

const rel = filePath.replace(ROOT, "").replace(/^[/\\]/, "");
warn(`${rel}:\n- ${findings.join("\n- ")}`);
