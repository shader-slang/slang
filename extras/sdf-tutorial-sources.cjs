// Read both lesson programs and indented support modules inside collapsed extras.
const fs = require("node:fs");
const path = require("node:path");

exports.readSdfTutorial = function () {
    const markdown = fs.readFileSync(path.join(__dirname, "../docs/heterogeneity/sdf-path-tracer.md"), "utf8");
    const cells = [...markdown.matchAll(/^([ \t]*)<!-- (runnable|module): ([a-z0-9-]+)([^\n]*?) -->\s*```slang\r?\n([\s\S]*?)\r?\n[ \t]*```/gm)]
        .map(([, indent, kind, name, options, source]) => ({
            kind, name, options,
            source: source.replaceAll("\r\n", "\n").split("\n")
                .map(line => line.startsWith(indent) ? line.slice(indent.length) : line)
                .join("\n").replaceAll(" //!FOLD", ""),
        }));
    return { cells, sources: Object.fromEntries(cells.map(cell => [`${cell.name}.slang`, cell.source])) };
};
