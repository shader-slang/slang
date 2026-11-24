// Program output is rendered the way a terminal shows it, and compiler diagnostics in it are
// connected back to the page's cells.
//
// slangvm reports compile errors in Slang's rich layout, colored with ANSI SGR sequences:
//
//   error[E30019]: expected an expression of type 'int', got 'float'
//     ╭╼ main.slang:7:13
//     │
//   7 │     int x = 1.5;
//     │             ━━━ expected 'int'
//
// Colored output draws the gutter with box-drawing characters; plain output uses ASCII, with
// ` --> ` before a diagnostic's location and ` --- ` before a note's. The box-drawing layout
// uses ` ╭╼ ` for both, so the preceding header decides whether a location is a note's.
//
// renderTerminal turns the colors into styled spans and each location in a page cell, such as
// `main.slang:7:13`, into a button that reveals that position in the cell's editor.
// findDiagnostics returns the errors and warnings at those locations, which the cells show as
// editor markers. Both also accept the one-line `main.slang(7): error 30019: message` layout and
// the `(at main.slang:7:13)` suffix of runtime failures.

const SGR = /\x1b\[([0-9;]*)m/g;
const LOCATION = /([\w.\/-]+\.slang)(?::(\d+)(?::(\d+))?|\((\d+)(?:,\s*(\d+))?\))/g;
const COLORS = {
  30: "black", 31: "red", 32: "green", 33: "yellow", 34: "blue", 35: "magenta", 36: "cyan", 37: "white",
  90: "bright-black", 91: "bright-red", 92: "bright-green", 93: "bright-yellow", 94: "bright-blue",
  95: "bright-magenta", 96: "bright-cyan", 97: "bright-white",
};

export const stripTerminalStyles = text => text.replace(SGR, "");

// Splits `text` into runs that share one SGR state, such as `{ text: "error", color: "red" }`.
function styledRuns(text) {
  const runs = [];
  let bold = false;
  let color = null;
  let start = 0;
  for (const match of text.matchAll(SGR)) {
    if (match.index > start) runs.push({ text: text.slice(start, match.index), bold, color });
    start = match.index + match[0].length;
    for (const code of (match[1] || "0").split(";").map(Number)) {
      if (code === 0) { bold = false; color = null; }
      else if (code === 1) bold = true;
      else if (code === 22) bold = false;
      else if (code === 39) color = null;
      else if (COLORS[code]) color = COLORS[code];
    }
  }
  if (start < text.length) runs.push({ text: text.slice(start), bold, color });
  return runs;
}

// Returns `{ filename, line, column }` for one LOCATION match, or null when `resolve` reports
// that the path is not a page cell. A missing column means the start of the line.
function locationOf(match, resolve) {
  const filename = resolve(match[1]);
  const line = Number(match[2] ?? match[4]);
  if (!filename || !line) return null;
  return { filename, line, column: Number(match[3] ?? match[5] ?? 1) };
}

// Renders `text` into a fragment of styled spans and location buttons.
//
// `resolve(path)` returns the cell filename for a diagnostic path, or null for files such as
// standard modules that have no cell; `reveal(location)` handles a location button's click.
export function renderTerminal(text, { resolve, reveal }) {
  const fragment = document.createDocumentFragment();
  for (const run of styledRuns(text)) {
    let parent = fragment;
    if (run.bold || run.color) {
      parent = document.createElement("span");
      if (run.bold) parent.classList.add("ansi-bold");
      if (run.color) parent.classList.add(`ansi-${run.color}`);
      fragment.append(parent);
    }
    let start = 0;
    for (const match of run.text.matchAll(LOCATION)) {
      const location = locationOf(match, resolve);
      if (!location) continue;
      parent.append(run.text.slice(start, match.index));
      const button = document.createElement("button");
      button.type = "button";
      button.className = "snippet-location";
      button.textContent = match[0];
      button.title = `Show line ${location.line} of ${location.filename}`;
      button.addEventListener("click", () => reveal(location));
      parent.append(button);
      start = match.index + match[0].length;
    }
    parent.append(run.text.slice(start));
  }
  return fragment;
}

const RICH_HEADER = /^(internal error|fatal error|error|warning|note|help)(?:\[E(\d+)\])?: (.*)$/;
const RICH_LOCATION = /^\s*(?:-->|---|╭╼) (.+?):(\d+):(\d+)\s*$/;
const LEGACY = /^(.+?)\((\d+)(?:,\s*(\d+))?\): (internal error|fatal error|error|warning)(?: (\d+))?: (.*)$/;
const RUNTIME = /\s*\(at ([\w.\/-]+\.slang):(\d+):(\d+)\)/;
const TRACE_LINE = /^\s+(?:at \S|\.\.\. more frames$)/;
const TRACE_FRAME = /^\s+at .+? \(([\w.\/-]+\.slang):(\d+):(\d+)\)\s*$/;

// Returns the errors and warnings in `text` that point into page cells, as
// `{ filename, line, column, severity, code, message }`, at most one per location.
//
// A rich diagnostic's primary location is the first location line after its header; a note's
// location only adds context, so it produces no entry of its own.
//
// A runtime failure ends with its call stack, innermost call first:
//
//   assertion failed
//       at assert
//       at check (main.slang:4:5)
//       at main (main.slang:9:5)
//
// Its first frame in a page cell, `check` here, is the user code nearest the failure and gets the
// failure's entry. `failureMessage` names the failure when the caller knows it; otherwise the
// line before the stack does.
export function findDiagnostics(text, resolve, { failureMessage } = {}) {
  const diagnostics = new Map();
  const add = (path, line, column, severity, code, message) => {
    const filename = resolve(path);
    if (!filename || !Number(line)) return;
    const key = `${filename}:${line}:${column}`;
    if (!diagnostics.has(key)) diagnostics.set(key, {
      filename, line: Number(line), column: Number(column) || 1,
      severity: severity === "warning" ? "warning" : "error",
      code: code ? `E${code.padStart(5, "0")}` : undefined, message: message.trim(),
    });
  };
  let header = null;
  let previousLine = "";
  let trace = null;
  for (const line of stripTerminalStyles(text).split("\n")) {
    if (TRACE_LINE.test(line)) {
      trace ??= { message: failureMessage || previousLine, located: false };
      const frame = TRACE_FRAME.exec(line);
      if (frame && !trace.located && resolve(frame[1])) {
        add(frame[1], frame[2], frame[3], "error", undefined, trace.message);
        trace.located = true;
      }
      continue;
    }
    trace = null;
    if (line.trim()) previousLine = line.trim();
    const rich = RICH_HEADER.exec(line);
    if (rich) {
      header = { severity: rich[1], code: rich[2], message: rich[3], located: false };
      continue;
    }
    const location = RICH_LOCATION.exec(line);
    if (location) {
      if (header && !header.located && header.severity !== "note" && header.severity !== "help") {
        add(location[1], location[2], location[3], header.severity, header.code, header.message);
      }
      if (header) header.located = true;
      continue;
    }
    const legacy = LEGACY.exec(line);
    if (legacy) {
      add(legacy[1], legacy[2], legacy[3], legacy[4], legacy[5], legacy[6]);
      continue;
    }
    const runtime = RUNTIME.exec(line);
    if (runtime) add(runtime[1], runtime[2], runtime[3], "error", undefined, line.replace(RUNTIME, ""));
  }
  return [...diagnostics.values()];
}
