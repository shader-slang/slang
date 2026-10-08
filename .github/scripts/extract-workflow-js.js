// Extract a named, inlined JavaScript block from a GitHub workflow YAML and
// load it as a module.
//
// Workflows sometimes inline JS into a `script:` literal (e.g. so a reusable
// workflow needs no checkout when called cross-repo). To keep that JS unit-
// testable, wrap the block in sentinel comments that NAME it:
//
//     // ----- extract-js:<block>:begin -----
//     ...the functions...
//     // ----- extract-js:<block>:end -----
//
// `load({workflow, block})` reads that block, dedents it (the inverse of the
// indent it carries inside the `script:` literal, so the result is byte-
// identical to standalone source), appends `module.exports` for every top-level
// declaration, writes a temp file, and requires it. Tests then exercise exactly
// what the workflow runs. Nothing here is specific to any one workflow or block.
"use strict";

const fs = require("node:fs");
const os = require("node:os");
const path = require("node:path");

// Repo root, so callers can pass a repo-relative workflow path.
const REPO_ROOT = path.join(__dirname, "..", "..");

function markerPair(block) {
  if (!block || !/^[\w.-]+$/.test(block))
    throw new Error(`invalid block name: ${JSON.stringify(block)}`);
  return { begin: `extract-js:${block}:begin`, end: `extract-js:${block}:end` };
}

// The dedented JavaScript source of the named block within `yamlText`.
function extractSource(yamlText, block) {
  const { begin, end } = markerPair(block);
  const bi = yamlText.indexOf(begin);
  const ei = yamlText.indexOf(end);
  if (bi < 0 || ei < 0 || ei < bi)
    throw new Error(`extract-js markers for block "${block}" not found`);
  const between = yamlText.slice(yamlText.indexOf("\n", bi) + 1, yamlText.lastIndexOf("\n", ei) + 1);
  const lines = between.split("\n");
  let min = Infinity;
  for (const l of lines) {
    if (l.trim() === "") continue;
    const indent = l.length - l.replace(/^\s+/, "").length;
    if (indent < min) min = indent;
  }
  if (!isFinite(min)) min = 0;
  return lines.map((l) => l.slice(min)).join("\n").replace(/\s+$/, "") + "\n";
}

function resolveWorkflow(workflow) {
  if (!workflow) throw new Error("workflow path is required");
  return path.isAbsolute(workflow) ? workflow : path.join(REPO_ROOT, workflow);
}

// `extractSource` + a `module.exports` of every top-level function/const,
// ready to write to a .js file.
function buildModule({ workflow, block }) {
  const src = extractSource(fs.readFileSync(resolveWorkflow(workflow), "utf8"), block);
  const names = new Set();
  const re = /^(?:async\s+)?function\s+([A-Za-z_$][\w$]*)|^const\s+([A-Za-z_$][\w$]*)\s*=/gm;
  let m;
  while ((m = re.exec(src))) names.add(m[1] || m[2]);
  return `"use strict";\n${src}\nmodule.exports = { ${[...names].join(", ")} };\n`;
}

// Build, write to a unique temp file, and require it.
function load({ workflow, block }) {
  const code = buildModule({ workflow, block });
  const tmp = path.join(os.tmpdir(), `wf-extract-${block}-${process.pid}-${Date.now()}.js`);
  fs.writeFileSync(tmp, code);
  try {
    return require(tmp);
  } finally {
    fs.unlinkSync(tmp);
  }
}

module.exports = { extractSource, buildModule, load };

// CLI: `node extract-workflow-js.js <workflow-yaml> <block> [out.js]`
if (require.main === module) {
  const [workflow, block, out] = process.argv.slice(2);
  if (!workflow || !block) {
    console.error("usage: extract-workflow-js.js <workflow-yaml> <block> [out.js]");
    process.exit(2);
  }
  const code = buildModule({ workflow, block });
  if (out) {
    fs.writeFileSync(out, code);
    console.log(`wrote ${out}`);
  } else {
    process.stdout.write(code);
  }
}
