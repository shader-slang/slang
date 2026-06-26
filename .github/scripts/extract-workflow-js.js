// Extract the inlined committer-signal / assignment JavaScript from the
// pr-board-sync.yml workflow and load it as a module.
//
// The workflow is the single source of truth for that logic (it is inlined so
// the reusable workflow needs no checkout when called cross-repo). To keep the
// logic unit-testable, the same block is delimited by sentinel comments:
//
//     // ----- extracted-assignment-js:begin -----
//     ...the ported functions...
//     // ----- extracted-assignment-js:end -----
//
// `load()` reads that block, dedents it (the exact inverse of the 12-space
// indent it carries inside the YAML `script:` literal, so the result is byte-
// identical to the original modules), appends `module.exports` for every
// top-level declaration, writes a temp file, and requires it. The test files
// (pr-signal.test.js, pr-assign.test.js) consume this instead of standalone
// source modules, so they test exactly what the workflow runs.
"use strict";

const fs = require("node:fs");
const os = require("node:os");
const path = require("node:path");

const BEGIN = "// ----- extracted-assignment-js:begin -----";
const END = "// ----- extracted-assignment-js:end -----";
const DEFAULT_WORKFLOW = path.join(__dirname, "..", "workflows", "pr-board-sync.yml");

// The dedented JavaScript source between the sentinel markers.
function extractSource(yamlText) {
  const bi = yamlText.indexOf(BEGIN);
  const ei = yamlText.indexOf(END);
  if (bi < 0 || ei < 0 || ei < bi)
    throw new Error("extracted-assignment-js markers not found in workflow");
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

// `extractSource` + a `module.exports` of every top-level function/const, ready
// to write to a .js file.
function buildModule(yamlPath) {
  const src = extractSource(fs.readFileSync(yamlPath || DEFAULT_WORKFLOW, "utf8"));
  const names = new Set();
  const re = /^(?:async\s+)?function\s+([A-Za-z_$][\w$]*)|^const\s+([A-Za-z_$][\w$]*)\s*=/gm;
  let m;
  while ((m = re.exec(src))) names.add(m[1] || m[2]);
  return `"use strict";\n${src}\nmodule.exports = { ${[...names].join(", ")} };\n`;
}

// Build, write to a unique temp file, and require it.
function load(yamlPath) {
  const code = buildModule(yamlPath);
  const tmp = path.join(os.tmpdir(), `pr-board-assignment-${process.pid}-${Date.now()}.js`);
  fs.writeFileSync(tmp, code);
  try {
    return require(tmp);
  } finally {
    fs.unlinkSync(tmp);
  }
}

module.exports = { extractSource, buildModule, load };

// CLI: `node extract-workflow-js.js [out.js]` writes (or prints) the module.
if (require.main === module) {
  const out = process.argv[2];
  const code = buildModule();
  if (out) {
    fs.writeFileSync(out, code);
    console.log(`wrote ${out}`);
  } else {
    process.stdout.write(code);
  }
}
