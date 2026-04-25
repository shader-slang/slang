# slang-coverage-html + slang-coverage-merge

> **Shipping scope: per-OS reports OK; merged cross-OS report gated
> on branch counting.**
>
> - **Single-OS Windows**: ready to use. Windows LCOV carries no
>   BRDA records anyway (OpenCppCoverage limitation), so the
>   methodology gap below doesn't apply. The merger gives Windows
>   a publishable HTML report it doesn't have today on
>   <https://shader-slang.org/slang-coverage-reports/>.
> - **Single-OS Linux / macOS**: lines match CI within 0.5 pp,
>   functions match exactly. Branch numbers run ~8 pp below CI's
>   published values because `llvm-cov report` applies a different
>   collapse to BRDA records than we can derive from the LCOV
>   alone. Fine for ad-hoc use; **don't publish these as Slang's
>   official coverage** alongside or instead of the existing CI
>   per-OS dashboards.
> - **Merged cross-OS**: same branch concern, plus the merge
>   itself surfaces the discrepancy more visibly. Held until the
>   branch metric matches CI; see `tmp/coverage-renderer/plan.md`
>   § "CI changes needed to enable matching" for the concrete
>   path (small CI workflow change to upload an `llvm-cov report`
>   summary as an extra artifact, then a small consumer-side flag
>   in the merger).

Two cooperating Python 3 tools for Slang-ecosystem coverage
workflows:

- **`slang-coverage-html`** — render an LCOV `.info` file to static
  HTML. Zero-install on any platform — no `pip`, no Perl
  (`genhtml`), no .NET runtime (ReportGenerator).
- **`slang-coverage-merge`** — combine multiple LCOV inputs (e.g.
  one per CI host) into a single merged LCOV using max-aggregation.
  Pipe the result through the renderer for a unified report.

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
--slangc-filter        Restrict to the slangc compiler-only file set
                       CI uses (mirrors slangc-ignore-patterns.sh)
--quiet                Suppress progress output
```

Function and branch columns render automatically when the input LCOV
carries `FN:` / `FNDA:` / `BRDA:` records. No flag needed to enable
them — they disappear again when the LCOV has none.

## Options — `slang-coverage-merge`

```
slang-coverage-merge <LCOV ...> [options]

-o, --output PATH        Output path (default: stdout, "-")
--strip-prefix PREFIX    Extra path prefix to strip from SF: paths
                         (repeatable). Built-in defaults already cover
                         the three Slang CI runner roots.
--no-default-prefixes    Skip the built-in path prefixes
--slangc-filter          Restrict the merged output to the slangc
                         compiler-only file set CI uses (mirrors
                         tools/coverage/slangc-ignore-patterns.sh).
                         Applied per-input after path normalization.
--synthesize-functions   EXPERIMENTAL. Fill in FN/FNDA on inputs that
                         emit DA but no FN/FNDA (notably the Windows
                         OpenCppCoverage → LCOV converter), using
                         sibling inputs' (name, first_line) map and
                         this input's own DA at first_line as the
                         hit-count proxy. Treat as upper bound — DA
                         at entry can over-count for tail-merged or
                         fall-through entry lines.
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
- Built-in defaults strip the three Slang CI runner roots:
  `/__w/slang/slang/`, `/Users/runner/work/slang/slang/`,
  `D:\a\slang\slang\`. Use `--strip-prefix` to add more.
- Unmatched paths pass through unchanged.

Auto-detect: inputs ending in `.gz` are decompressed transparently.

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

For Linux slangc-filtered (PR-10884 run as a stable reference):

| Metric    | Ours    | CI (`llvm-cov report`) | Gap     |
| --------- | ------- | ---------------------- | ------- |
| Lines     | 77.75 % | 78.21 %                | -0.5 pp |
| Branches  | 65.79 % | 74.08 %                | -8.3 pp |
| Functions | 79.42 % | 79.45 %                | ~0 ✅   |

Functions match exactly after the line-dedup. Lines are within 0.5
pp (residual is generated headers like `hlsl.meta.slang.h` that
llvm-cov treats as non-instrumented). Branches are still 8 pp
below — `llvm-cov report` appears to collapse some BRDA records
that `llvm-cov export` emits faithfully; matching it exactly
without re-running `llvm-cov` ourselves is hard. Numbers we
publish are correct **per the LCOV** that CI hands us; if a
direct apples-to-apples branch number is needed, report CI's
per-OS branch %.

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

58 unit + integration tests cover: LCOV parsing (incl. TN: max-
aggregation, corrupt-input detection, unknown-record tolerance, BRDA
with `-` tokens, FN/FNDA join-by-name), source resolution (path
variants, caching, miss → placeholder), filter globs, tier
thresholds, function/branch percent calcs, per-line branch-cell
rendering (empty / all-taken / partial / none / not-evaluated), CLI
round-trip, empty-input rendering, idempotency modulo timestamp,
foreign-dir overwrite guard, phase-1-regression check against the
real-data phase-2 fixture, plus merge-tool path normalization
(forward-slash, longest-prefix-wins, custom prefixes), max-
aggregation (lines, branches, functions), absent-vs-zero handling,
gzipped-input auto-decompression, file-output mode, and end-to-end
"merge → render" smoke.

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
├── slang-coverage-html.py         # the renderer
├── slang-coverage-merge.py        # the multi-LCOV merger
├── lcov_io.py                     # shared parser / writer / data model
├── README.md                      # this file
└── tests/
    ├── test_renderer.py           # renderer unit + integration tests
    ├── test_merge.py              # merge unit + integration tests
    └── fixtures/
        ├── demo-cpu.info          # real LCOV from shader-coverage-demo
        ├── demo-cpu.coverage-mapping.json
        ├── genhtml-reference/     # visual target for parity
        ├── VISUAL-PARITY.md       # acceptance checklist
        ├── slangc-llvm-cov-sample.info  # real-data BRDA + FN/FNDA fixture
        ├── branches-and-functions.info  # small hand-crafted phase-2 fixture
        ├── empty.info
        ├── corrupt-bad-da.info
        ├── mixed-paths.info
        ├── tn-groups.info
        └── unknown-records.info
```
