# slang-coverage-html + slang-coverage-merge

Two cooperating Python 3 tools for static-HTML coverage reports
from LCOV `.info` files:

- **`slang-coverage-html`** — render one LCOV to a directory of
  static HTML. Zero-install on any platform — no `pip`, no Perl
  (`genhtml`), no .NET runtime (ReportGenerator).
- **`slang-coverage-merge`** — combine multiple LCOV inputs (e.g.
  one per CI host) into a single merged LCOV using max-aggregation.
  Pipe the result through the renderer for a unified report.

Both are **deliberately project-neutral**: they consume any standard
LCOV file. Project-specific filtering (e.g. exclude rules for a
particular code base) belongs in a thin wrapper around them — see
`tools/coverage/` for the Slang wrapper that injects slangc-only
filters and Slang's GitHub Actions runner path roots.

> **Branch coverage from `llvm-cov export -format=lcov` runs
> below `llvm-cov report`.** The export emits more BRDA records
> than the report counts (the report tool collapses some
> templated/inlined entries that the LCOV form keeps separate),
> so LCOV-derived branch numbers come in lower. The fix is the
> `--auth-summary` flag: pass an `llvm-cov report` text dump and
> per-file totals are taken from the report instead of derived
> from the LCOV. With it, branch / function / line numbers match
> the report exactly. Without it, branches are still useful but
> not directly comparable to a `llvm-cov report` number.

## Quick start

### Render one LCOV

```bash
python3 tools/coverage-html/slang-coverage-html.py coverage.lcov
# → writes ./coverage-html/index.html
open coverage-html/index.html       # macOS
xdg-open coverage-html/index.html   # Linux
start coverage-html\index.html      # Windows
```

### Merge per-OS LCOVs into one report

```bash
# Three inputs from CI artifacts (gzipped or plain).
python3 tools/coverage-html/slang-coverage-merge.py \
    linux.lcov.gz macos.lcov.gz windows.lcov.gz \
    -o merged.lcov

# Render as usual.
python3 tools/coverage-html/slang-coverage-html.py merged.lcov \
    --output-dir merged-html/
```

The output directory is fully static and self-contained. Double-click
`index.html`, upload the directory as a CI artifact, tar it up for
air-gapped viewing — it all works.

## Intended audience

This is a **narrow tool for Slang-ecosystem coverage workflows**, not
a general-purpose `genhtml` replacement. Three concrete inputs are
supported today:

1. **Shader coverage** — `examples/shader-coverage-demo` produces
   `coverage.lcov`; this renderer turns it into HTML.
2. **Compiler C++ coverage on Linux/macOS** — `llvm-cov export
-format=lcov` → this renderer.
3. **Compiler C++ coverage on Windows** — `OpenCppCoverage` →
   Cobertura → (converter) → LCOV → this renderer.

If outside users adopt it for other LCOV producers, great — but we
don't promise support for every LCOV extension in the wild.

## Options — `slang-coverage-html`

```
slang-coverage-html <input.lcov> [options]

--output-dir PATH      Output directory (default: ./coverage-html/)
--title TEXT           Page title (default: "Coverage report")
--source-root PATH     Directory used as a root when resolving SF: paths
--filter-include GLOB  Include-only glob, applied to SF: path (repeatable)
--filter-exclude GLOB  Exclude glob, applied to SF: path (repeatable)
--filter-include-regex REGEX  Include-only Python regex, matched
                              anywhere in SF: path (repeatable).
--filter-exclude-regex REGEX  Exclude Python regex (repeatable).
--auth-summary FILE    Override LCOV-derived per-file totals with the
                       authoritative numbers from an `llvm-cov report`
                       text dump (e.g. CI's source of truth).
--quiet                Suppress progress output
```

Function and branch columns render automatically when the input LCOV
carries `FN:` / `FNDA:` / `BRDA:` records. No flag needed to enable
them — they disappear again when the LCOV has none.

## Options — `slang-coverage-merge`

```
slang-coverage-merge <LCOV ...> [options]

-o, --output PATH        Output path (default: stdout, "-")
--strip-prefix PREFIX    Path prefix to strip from SF: paths
                         (repeatable). Use this to collapse per-OS
                         CI runner roots into stable repo-relative
                         paths before merging.
--filter-include-regex REGEX  Include-only Python regex (repeatable;
                              matched anywhere in SF: path). Applied
                              after path normalization.
--filter-exclude-regex REGEX  Exclude Python regex (repeatable).
                              Applied after path normalization, so
                              it always sees the same shape regardless
                              of input.
--synthesize-functions   EXPERIMENTAL. Fill in FN/FNDA on inputs that
                         emit DA but no FN/FNDA (notably the Windows
                         OpenCppCoverage → LCOV converter), using
                         sibling inputs' (name, first_line) map and
                         this input's own DA at first_line as the
                         hit-count proxy. Treat as upper bound — DA
                         at entry can over-count for tail-merged or
                         fall-through entry lines.
--auth-summary FILE      `llvm-cov report` text dump (repeatable, one
                         per OS). Combined into one merged summary by
                         max(total) / min(missed) per file across
                         inputs.
--auth-summary-out PATH  Output path for the merged summary; required
                         when --auth-summary is given. Pass the result
                         to the renderer's --auth-summary flag.
--quiet                  Suppress progress output on stderr
```

Aggregation rules (max-across-inputs):

- `DA:` line hit count → max.
- `BRDA:` taken count → max; an integer beats `-` (None); "absent on
  this input" is treated as "no information", **not** as 0. So a
  branch with data on one OS and nothing on another counts as one
  branch with that OS's data, not two branches half-uncovered.
- `FN`/`FNDA` → first-FN line declaration wins (with fallback if one
  input has `first_line=0`); max FNDA hit count.
- `LF/LH/BRF/BRH/FNF/FNH` are **recomputed** from merged data; the
  inputs' totals are ignored (they'd be wrong after merging anyway).
- `TN:` is dropped from output. The merged file isn't attributable
  to a single test name.

Path normalization:

- Backslashes → forward slashes (Windows artifacts use `D:\...`).
- Each `--strip-prefix` is matched left-anchored; the longest
  matching prefix wins. No project defaults are baked in — pass
  the prefixes your CI uses (e.g. `--strip-prefix=/__w/repo/repo/`).
- Unmatched paths pass through unchanged.

Auto-detect: inputs ending in `.gz` are decompressed transparently.

## Project-specific defaults

The tools take **no project-specific defaults** — no
`--slangc-filter`, no built-in CI path prefixes. To pre-inject a
project's defaults (file-set filters, runner path roots), wrap
the tool. Slang's wrapper is at `tools/coverage/slang-render.py`
and `tools/coverage/slang-merge.py`; it consumes
`tools/coverage/slang_filters.py` and forwards the rest of the
arguments to the generic tool.

## Methodology vs `llvm-cov report`

Our percentages are computed from raw LCOV records, with one
deliberate adjustment so the numbers line up with CI's published
per-platform metrics:

- **Functions** are deduped by `(file, first_line)`. Each LCOV `FN:`
  record carries a mangled name, and the `llvm-cov export` step
  emits one record per template instantiation / compiler-generated
  duplicate of a single source-level function — so a single source
  function can appear as dozens of FN records, all sharing the
  same `first_line`. We collapse them: any instantiation hit counts
  the underlying source function as hit. This mirrors what
  `llvm-cov report` reports on the same `.profdata`.

  Additionally, a function counts as covered if **any of its body
  lines was hit**, even when its FNDA call count is 0. LLVM's
  coverage instrumentation tracks function entry via a dedicated
  counter that no inlined call ever bumps, so RAII helpers /
  constructors / destructors that the compiler inlines at every
  call site report FNDA=0 even when their bodies clearly ran.
  Pure FNDA accounting was reporting 0/N function coverage on
  files showing 100 % line coverage — confusing. We fall back to
  line-derived coverage when FNDA is silent.

- **Lines** are counted one per `DA:` record. Matches genhtml.
- **Branches** are counted one per `BRDA:` outcome. Matches genhtml.

As a reference, on a representative slangc-filtered Linux LCOV
of ~530 files:

| Metric    | LCOV-derived | `llvm-cov report` | Gap     |
| --------- | ------------ | ----------------- | ------- |
| Lines     | 77.75 %      | 78.21 %           | -0.5 pp |
| Branches  | 65.79 %      | 74.08 %           | -8.3 pp |
| Functions | 79.42 %      | 79.45 %           | ~0 ✅   |

Functions match exactly after the line-dedup. Lines are within
0.5 pp (residual is generated headers that `llvm-cov` treats as
non-instrumented). Branches run 8 pp below because `llvm-cov
report` collapses some BRDA records the export emits faithfully.

To match `llvm-cov report` exactly, pass `--auth-summary
<coverage-report.txt>` — per-file Lines/Functions/Branches totals
on the index, directory aggregates and per-file pages then come
from the report instead of being derived from LCOV.

### Source resolution

LCOV `SF:` paths come in three flavors:

| Flavor                 | Example                      | How we resolve                                      |
| ---------------------- | ---------------------------- | --------------------------------------------------- |
| Absolute               | `/home/user/slang/src/x.cpp` | Open directly                                       |
| Relative (to LCOV dir) | `src/x.cpp`                  | Open from the LCOV's directory                      |
| Arbitrary              | anything else                | Fall back to `--source-root/<basename>` if provided |

When a source file can't be located, the per-file page is still
rendered but shows a **`source unavailable`** banner and a table of
`(line, hits)` instead of annotated source. No sources on disk →
still-useful coverage numbers in the index.

### Output-dir safety

The first write drops a `slang-coverage-html.marker` sentinel. On
later runs:

- Empty directory → safe to write.
- Directory with the marker → overwrite freely.
- Non-empty directory _without_ the marker → refuse and exit 1
  (protects against accidentally pointing at someone's work tree).

## Phase matrix

| Phase  | Features                                                                                                                                                                                                                                                                                        | Status                        |
| ------ | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------- |
| **1**  | Line coverage (`DA:` records); overall + per-file %; index + annotated source view; source resolution; marker-guarded overwrite                                                                                                                                                                 | **Shipped**                   |
| **2**  | Branch coverage (`BRDA:`) summary (header + index column); function coverage (`FN:`/`FNDA:`) summary (header + index column)                                                                                                                                                                    | **Shipped**                   |
| **2b** | Inline per-line branch column between hit count and source. Collapsed `(hit/total)` summary with tier-colored span (`branchAll` green, `branchPart` amber, `branchNone` red) and per-branch tooltip (`title="br0: N; br1: N; br2: -"`). Gutter widens only on files that carry any BRDA records | **Shipped**                   |
| **2c** | Per-file Functions table moved into the index as an expandable row (chevron + hidden `<tr class="fileFunctions">` revealed by a tiny inline JS toggle). Per-file pages drop the Functions table; their header still shows the Functions summary metric                                          | **Shipped**                   |
| 3      | Per-test `TN:` drill-down; per-test tabs on file pages                                                                                                                                                                                                                                          | Aggregated (max-across) today |

Phase 2b preserves phase-1 byte-identical rendering when a file has
no branches — the branch column only appears on per-file pages whose
LCOV carries BRDA records. Lines without branches on such files
render their branch column as plain spaces, keeping the gutter
aligned down the whole page.

## Explicit non-goals

- **No coverage collection.** This tool consumes LCOV; it does not
  produce it. Pair with `llvm-cov export`, OpenCppCoverage, or
  `tools/shader-coverage/slang-coverage-to-lcov.py`.
- **No CI-service integration.** No Codecov upload, no GitHub-check
  posting.
- **No certified-coverage claims.** DO-178C / ISO 26262 tool
  qualification (DO-330) is incompatible with open-source governance.
- **No syntax highlighting.** Adds language-specific complexity; a
  clean coverage overlay is sufficient.
- **No interactive filtering.** Static pages only. Filter LCOV
  upstream with `lcov --extract` / `--remove` if you need to narrow
  the report.

## Comparison to alternatives

| Tool                            | Install on Windows                                         | Install on Linux/macOS                   | Input format      |
| ------------------------------- | ---------------------------------------------------------- | ---------------------------------------- | ----------------- |
| `genhtml` (from `lcov` package) | WSL / MSYS2 / Cygwin                                       | `apt install lcov` / `brew install lcov` | LCOV              |
| `reportgenerator` (.NET)        | `dotnet tool install -g dotnet-reportgenerator-globaltool` | same                                     | many (incl. LCOV) |
| **this tool**                   | **nothing**                                                | **nothing** (Python 3 ships w/ OS)       | LCOV              |

`genhtml` has more features (per-test views, MC/DC, branch rendering,
TLA baseline-diff overlays). Our wedge is zero-install consistency
across every platform our customers run.

## Development

### Running tests

```bash
python3 -m unittest discover -s tools/coverage-html/tests -v
```

108 unit + integration tests across `tests/test_lcov_io.py`,
`tests/test_renderer.py`, and `tests/test_merge.py` cover: LCOV
parsing (incl. TN: max-aggregation, corrupt-input detection,
unknown-record tolerance, BRDA with `-` tokens, FN/FNDA join-by-
name), `llvm-cov report` text parsing + auth-summary override,
source resolution (path variants, caching, miss → placeholder),
filter globs + regexes (composition rules), tier thresholds,
function/branch percent calcs, per-line branch-cell rendering
(empty / all-taken / partial / none / not-evaluated), CLI round-
trip, empty-input rendering, idempotency modulo timestamp,
foreign-dir overwrite guard, phase-1-regression check against the
real-data phase-2 fixture, plus merge-tool path normalization
(forward-slash, longest-prefix-wins), max-aggregation (lines,
branches, functions, auth summaries), absent-vs-zero handling,
gzipped-input auto-decompression, file-output mode, and end-to-end
"merge → render" smoke.

The Slang wrapper at `tools/coverage/` carries an additional 13
tests (`tools/coverage/tests/test_slang_filters.py`) for the
slangc-ignore pattern set and the wrapper-CLI plumbing.

### Updating the demo fixture

`tests/fixtures/demo-cpu.info` is the primary test input — a real
LCOV from running `shader-coverage-demo --mode=dispatch --backend=cpu`.
To regenerate after the demo changes:

```bash
cmake --build --preset debug --target shader-coverage-demo
cd examples/shader-coverage-demo
./../../build/Debug/bin/shader-coverage-demo --mode=dispatch --backend=cpu
cp coverage.lcov             ../../tools/coverage-html/tests/fixtures/demo-cpu.info
cp simulate.coverage-mapping.json ../../tools/coverage-html/tests/fixtures/demo-cpu.coverage-mapping.json
```

### Updating the genhtml visual target

`tests/fixtures/genhtml-reference/` is the visual target we track
against. Regenerate if genhtml changes substantively:

```bash
cd examples/shader-coverage-demo
genhtml $(git rev-parse --show-toplevel)/tools/coverage-html/tests/fixtures/demo-cpu.info \
    -o $(git rev-parse --show-toplevel)/tools/coverage-html/tests/fixtures/genhtml-reference/
```

See `tests/fixtures/VISUAL-PARITY.md` for the acceptance checklist
that our output is designed to match.

## Files

```
tools/coverage-html/
├── slang-coverage-html.py         # the renderer (CLI)
├── slang-coverage-merge.py        # the multi-LCOV merger (CLI)
├── lcov_io.py                     # shared parser / writer / data model
│                                  #   + SourceResolver, function-coverage
│                                  #   helpers, llvm-cov report parser /
│                                  #   AuthSummary merger / writer
├── style.css                      # inlined into every page at render time
├── script.js                      # toggle / chrome-measure JS, same
├── README.md                      # this file
└── tests/
    ├── test_lcov_io.py            # parser + data model + helpers
    ├── test_renderer.py           # HTML rendering + CLI integration
    ├── test_merge.py              # merger unit + integration tests
    └── fixtures/
        ├── demo-cpu.info          # real LCOV from shader-coverage-demo
        ├── demo-cpu.coverage-mapping.json
        ├── genhtml-reference/     # visual target for parity
        ├── VISUAL-PARITY.md       # acceptance checklist
        ├── slangc-llvm-cov-sample.info  # real-data BRDA + FN/FNDA fixture
        ├── branches-and-functions.info  # small hand-crafted phase-2 fixture
        ├── llvm-cov-report-sample.txt   # auth-summary parser fixture
        ├── empty.info
        ├── corrupt-bad-da.info
        ├── mixed-paths.info
        ├── tn-groups.info
        └── unknown-records.info

tools/coverage/
├── slang_filters.py               # canonical Python copy of slangc patterns
│                                  #   + Slang CI runner path roots
├── slang-render.py                # wrapper: slang-coverage-html with
│                                  #   slangc filter injected
├── slang-merge.py                 # wrapper: slang-coverage-merge with
│                                  #   filter + CI prefixes injected
└── tests/test_slang_filters.py    # filter set + wrapper CLI tests
```
