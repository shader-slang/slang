# Visual-parity checklist — `slang-coverage-html` vs `genhtml`

Acceptance criteria for the HTML-generator step. Target the genhtml
output captured in `genhtml-reference/`. Match the **look**, not every
genhtml feature — branch/function view, test-case detail, TLA/baseline
diff, and extra sort-variant pages are all explicit non-goals for
phase 1.

Reference files to mirror:

- `genhtml-reference/index.html` — top-level directory summary.
- `genhtml-reference/shader-coverage-demo/index.html` — per-directory
  file summary.
- `genhtml-reference/shader-coverage-demo/physics.slang.gcov.html` —
  per-file annotated source view.
- `genhtml-reference/gcov.css` — source of class names; inline a
  trimmed subset into every page (phase 1 decision).

## Three page types

genhtml produces a three-level hierarchy when the LCOV spans multiple
directories:

1. **Top-level `index.html`** — rows are directories.
2. **Per-directory `index.html`** — rows are files under that dir.
3. **Per-file `<name>.gcov.html`** — annotated source view.

For phase 1 **collapse levels 1+2 into a single `index.html`** unless
the LCOV has multiple directories. Most of our inputs (shader demo,
shader-coverage-to-lcov output) ship files from one directory. If we
see >1 dir, emit the dir-grouped layout genhtml uses. Deferrable
polish — two levels initially, nesting only if needed.

## Common chrome (all pages)

- `<table class="ruler">` horizontal bars above and below the header
  block (genhtml uses a 3-px blue image; we'll use a `border-top`
  style, no image).
- Title bar: `<td class="title">LCOV - code coverage report</td>`.
- **Header summary table** with class names genhtml uses:
  - `headerItem` / `headerValue` for label/value pairs.
  - `headerCovTableHead` for column heads ("Coverage", "Total", "Hit").
  - `headerCovTableEntry` for numeric cells.
  - Rate cells pick a tier class:
    - `headerCovTableEntryHi` (≥90%, green `#a7fc9d`)
    - `headerCovTableEntryMed` (≥75%, amber `#ffea20`)
    - `headerCovTableEntryLo` (<75%, red `#ff0000`)
  - Thresholds: genhtml's defaults are 90/75. Match them; make them
    CLI-overridable later if anyone asks.
- Header shows: `Current view:` breadcrumb (with back-links on sub
  pages), `Test:` (derived from input filename), `Test Date:` (UTC
  timestamp), `Lines:` rate+total+hit, `Functions:` (phase 2 — emit
  the row with `-` / `0` / `0` like genhtml does today).
- Footer: single-row table with `<td class="versionInfo">Generated
by: slang-coverage-html</td>` — mirror genhtml's versionInfo pattern
  but with our name.

## Index page (`index.html`)

- `<center><table width="80%">` layout with width-sized column spacers
  (genhtml's 40/15/15/15/15 pattern).
- `<td class="tableHead" rowspan=2>File</td>` + `<td colspan=4>Line
Coverage</td>` header, then sub-row `Rate | Total | Hit`.
- Each row:
  - `<td class="coverFile">` with `<a href="...">filename</a>`.
    (Use `coverDirectory` + `#b8d0ff` background for directory rows.)
  - `<td class="coverBar">`: a bar made of two fixed-height `<span>`s
    (no images). Outer has `coverBarOutline` background
    (black `#000000`), inner has amber/emerald/ruby based on tier.
    Width proportional to coverage pct.
  - `<td class="coverPerHi/Med/Lo">` rate cell — tiered color.
  - Two `<td class="coverNumDflt">` cells for Total and Hit.
- **No sort variants** (no `index-sort-l.html`, `index-sort-f.html`).
  Plain unsorted-by-name render; defer client-side sort to phase 2.

## Per-file source view (`<name>.html`)

- Same header chrome (with breadcrumb back-link to index).
- Pre-source heading:
  ```
  <pre class="sourceHeading">            Line data    Source code</pre>
  ```
  Two-column gutter (line no., hit count) then source.
- Source block wrapped in `<pre class="source">`.
- Each physical source line rendered as:
  ```
  <span id="L{n}"><span class="lineNum">{n:>8}</span>{GUTTER}{LINE}</span>
  ```
  where:
  - `lineNum` span is the line-number gutter, background `#efe383`.
  - Covered lines: `<span class="tlaGNC">{hits:>12} : {code}</span>`
    (class `tlaGNC`, background `#CAD7FE`). "GNC" = "Gained New
    Coverage" — genhtml's default TLA when no baseline is given.
  - Uncovered lines: `<span class="tlaUNC">{0:>12} : {code}</span>`
    (class `tlaUNC`, background `#FF6230`).
  - Non-executable lines: plain text between `lineNum` and the
    `\n` — 14 spaces of padding then `: {code}`.
- HTML-escape `<`, `>`, `&` in the source text.
- Preserve exact indentation (tabs → spaces per tab size 8 is what
  genhtml does; match or keep raw tabs — raw is fine in `<pre>`).
- For sources we couldn't resolve: emit the source block as a
  table of `(line, hits)` tuples with a `source unavailable` banner
  at the top. Keep all other chrome identical.

## CSS strategy

Ship **one** inlined `<style>` block per page containing only the
classes actually used (probably ~30 selectors vs. genhtml's ~300
once TLA states, branches, MC/DC and test-owner styling are stripped).
Rationale:

- Self-contained requirement (M2).
- No external `gcov.css` reference; no image assets (amber/emerald/
  snow/glass pngs replaced with CSS `background-color`).
- Smaller pages, faster to load.
- Consistent class names → tooling that grep'd genhtml output can
  often continue to grep ours.

## What we're explicitly NOT matching

- `amber.png`/`snow.png` bar graphic → replace with two CSS `<span>`s.
- `glass.png` 3-px separators → CSS `border-top`.
- `updown.png` sort icons → omit (no sort variants in phase 1).
- `gcov.css` external link → inline.
- Sort-variant pages (`index-sort-l.html`, `index-sort-f.html`) → omit.
- `coverLegend*` legend strip → omit (deferred polish).
- Branch / function columns → omit (phase 2).
- Test-owner / TLA-baseline TLA states beyond `tlaGNC` / `tlaUNC` →
  omit.
- Per-test drill-down (`testName`, `testPer`, `testNum`) → omit
  (phase 3).

## Color palette (lifted verbatim from `gcov.css`)

| Purpose                   | Color     | Class                                |
| ------------------------- | --------- | ------------------------------------ |
| High rate background      | `#a7fc9d` | `coverPerHi`, `tlaGNC` (header mode) |
| Medium rate background    | `#ffea20` | `coverPerMed`                        |
| Low rate background       | `#ff0000` | `coverPerLo`                         |
| Covered line background   | `#CAD7FE` | `tlaGNC`                             |
| Uncovered line background | `#FF6230` | `tlaUNC`                             |
| Line number gutter        | `#efe383` | `lineNum`                            |
| Default row bg (numbers)  | `#dae7fe` | `coverNumDflt`                       |
| File-name link color      | `#284fa8` | `coverFile a`                        |
| Ruler                     | `#6688d4` | `ruler`                              |
| Table head                | `#6688d4` | `tableHead`                          |

## Verification procedure

Once the renderer is implemented:

1. Run renderer on `demo-cpu.info` into `/tmp/slang-coverage-html-out/`.
2. Open `/tmp/slang-coverage-html-out/index.html` and
   `genhtml-reference/index.html` side-by-side.
3. Compare:
   - Overall coverage rate (84.6%) matches.
   - File list (`physics.slang`, `simulate.slang`) present.
   - Per-file rates (85.7%, 83.3%) match.
   - Color tier for each rate matches (both should be "Med"/amber).
4. Open each per-file page and compare:
   - Line 17 of `physics.slang`: 688 hits, covered (blue).
   - Lines 45-46 of `physics.slang`: 0 hits, uncovered (red).
   - Line numbers on the gutter match the source.
5. `wc -c` the reference `.html` files and ours — expect ours smaller
   (no image tags, inlined CSS). Within 2x is fine; 10x bigger is a
   regression worth investigating.
