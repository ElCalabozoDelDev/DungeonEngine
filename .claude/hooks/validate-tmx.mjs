// PostToolUse hook: sanity-checks a Tiled level after it is written.
//
// Advisory only. Every check here covers something the loader ignores in
// complete silence -- a mistyped layer name, an unknown object `type`, a
// textureID with no entry in the map's texture registry. The result is an
// entity that renders and does nothing, with no error anywhere, which is very
// hard to spot by reading the file.
//
// Regex, not an XML parser: these are shape checks on a machine-generated file,
// and a hook must not need `npm install` to run.

import { existsSync, readFileSync } from "node:fs";
import { relative, sep } from "node:path";
import { ROOT, readStdin, warn } from "./gdd-lib.mjs";

const LAYERS = ["Bottom", "Overlay", "Collision"];
const TYPES = ["Player", "Enemy", "Item"];
const PROPS = [
    "textureID",
    "spriteRow",
    "spriteCol",
    "totalFrames",
    "animationTime",
];

const root = ROOT;

let payload;
try {
    payload = JSON.parse(await readStdin());
} catch {
    warn(null);
}

const filePath = payload?.tool_input?.file_path;
if (typeof filePath !== "string" || !filePath.endsWith(".tmx")) warn(null);
if (!existsSync(filePath)) warn(null);

let xml = "";
try {
    xml = readFileSync(filePath, "utf8");
} catch {
    warn(null);
}

const findings = [];

// Tag balance, as a cheap stand-in for well-formedness.
const stack = [];
for (const [, closing, name, selfClosing] of xml.matchAll(
    /<(\/?)([A-Za-z][\w.-]*)[^>]*?(\/?)>/g,
)) {
    if (closing === "/") {
        if (stack.pop() !== name) {
            findings.push(`XML mal formado: </${name}> inesperado`);
            break;
        }
    } else if (selfClosing !== "/") {
        stack.push(name);
    }
}
if (findings.length === 0 && stack.length > 0) {
    findings.push(`XML mal formado: sin cerrar <${stack[stack.length - 1]}>`);
}

// The texture registry lives in the map-level <properties>, before the first
// <tileset>; object textureIDs are looked up in it.
const header = xml.split(/<tileset\b/)[0];
const textures = new Set(
    [...header.matchAll(/<property\s+name="([^"]+)"\s+value="[^"]*"/g)].map(
        (m) => m[1],
    ),
);

// Layers: only the three magic names are ever drawn or collided against.
for (const [, name] of xml.matchAll(/<layer\b[^>]*\bname="([^"]*)"/g)) {
    if (!LAYERS.includes(name)) {
        findings.push(
            `capa "${name}" no reconocida: se carga pero nunca se dibuja ` +
                `(esperadas: ${LAYERS.join(", ")})`,
        );
    }
}

for (const [, encoding] of xml.matchAll(/<data\b[^>]*\bencoding="([^"]*)"/g)) {
    if (encoding !== "base64") {
        findings.push(`<data encoding="${encoding}">: el cargador exige base64`);
    }
}
for (const [, compression] of xml.matchAll(
    /<data\b[^>]*\bcompression="([^"]*)"/g,
)) {
    if (compression !== "zlib") {
        findings.push(
            `<data compression="${compression}">: el cargador exige zlib`,
        );
    }
}
if (/<data\b(?![^>]*\bcompression=)/.test(xml)) {
    findings.push("<data> sin `compression`: el cargador exige zlib");
}

// Objects. The opening tag's attributes are consumed with [^>]* so the match
// cannot stop early: a lazy [\s\S]*? up to /> ends at the first self-closing
// <property/> inside the object, which silently skipped every property check.
let players = 0;
for (const [body] of xml.matchAll(
    /<object\b[^>]*(?:\/>|>[\s\S]*?<\/object>)/g,
)) {
    const name = /\bname="([^"]*)"/.exec(body)?.[1] ?? "(sin nombre)";
    const type = /\btype="([^"]*)"/.exec(body)?.[1];

    if (type === undefined || type === "") {
        findings.push(`objeto "${name}" sin \`type\`: se ignora en silencio`);
        continue;
    }
    if (!TYPES.includes(type)) {
        findings.push(
            `objeto "${name}" con type="${type}" desconocido: se ignora ` +
                `(esperados: ${TYPES.join(", ")})`,
        );
        continue;
    }
    if (type === "Player") players += 1;

    for (const [, prop] of body.matchAll(/<property\s+name="([^"]+)"/g)) {
        if (!PROPS.includes(prop)) {
            findings.push(
                `objeto "${name}": la propiedad "${prop}" no la lee nadie`,
            );
        }
    }

    const textureID = /<property\s+name="textureID"\s+value="([^"]*)"/.exec(
        body,
    )?.[1];
    if (textureID !== undefined && !textures.has(textureID)) {
        findings.push(
            `objeto "${name}": textureID="${textureID}" no está en las ` +
                "<properties> del mapa",
        );
    }
}

if (players === 0) {
    findings.push(
        'ningún objeto con type="Player": el nivel no arranca (la cámara no ' +
            "tiene a quién seguir)",
    );
} else if (players > 1) {
    findings.push(`${players} objetos "Player": solo se usa el primero`);
}

if (findings.length === 0) warn(null);

const rel = relative(root, filePath).split(sep).join("/");
warn(
    `TMX · ${rel}\n` +
        findings.map((f) => `  · ${f}`).join("\n") +
        "\n  (aviso, no bloquea)",
);
