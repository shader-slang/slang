import { computeRanges } from "monaco-editor/esm/vs/editor/contrib/folding/browser/indentRangeProvider.js";

// Allman-style blocks keep their preceding declaration visible instead of the brace line.
export function foldingHeaderLine(model, line) {
    if (!/^\s*\{/.test(model.getLineContent(line))) return line;
    let previous = line - 1;
    while (previous > 0 && !model.getLineContent(previous).trim()) previous--;
    if (!previous) return line;
    const header = model.getLineContent(previous).trim();
    // A standalone scope after a statement or another brace has no declaration to keep.
    return /[;{}]$/.test(header) || /^(\/\/|\/\*|\*|#)/.test(header) ? line : previous;
}

export function registerSlangFolding(monaco) {
    monaco.languages.registerFoldingRangeProvider("slang", {
        provideFoldingRanges(model) {
            // Retain indentation folds for comments, parameter lists, and unfinished code.
            const indentation = computeRanges(model, false);
            const ranges = new Map();
            for (let i = 0; i < indentation.length; i++) {
                const start = indentation.getStartLineNumber(i);
                ranges.set(start, { start, end: indentation.getEndLineNumber(i) });
            }

            // Reuse the pinned Monaco build's token-aware brace matching so braces inside
            // strings/comments cannot end a fold. No compiler or language server is needed.
            model.tokenization.forceTokenization(model.getLineCount());
            const blocks = [];
            model.bracketPairs.getBracketPairsInRange(model.getFullModelRange()).toArray().forEach(pair => {
                if (!pair.closingBracketRange || model.getValueInRange(pair.openingBracketRange) !== "{") return;
                const opening = pair.openingBracketRange;
                const closing = pair.closingBracketRange;
                const prefix = model.getLineContent(opening.startLineNumber).slice(0, opening.startColumn - 1);
                const start = prefix.trim() ? opening.startLineNumber : foldingHeaderLine(model, opening.startLineNumber);
                let end = closing.endLineNumber;
                const suffix = model.getLineContent(end).slice(closing.endColumn - 1);
                // Keep a shared line such as `} else {` visible; closing `});`/`};` belongs to the block.
                if (!/^[\s;),]*(?:\/\/.*)?$/.test(suffix)) end--;
                if (end > start) {
                    ranges.delete(opening.startLineNumber);
                    blocks.push({ start, end });
                }
            });
            for (const block of blocks) {
                const existing = ranges.get(block.start);
                if (!existing || existing.end < block.end) ranges.set(block.start, block);
            }
            return [...ranges.values()].sort((a, b) => a.start - b.start);
        },
    });
}
