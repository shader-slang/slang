# slang-coverage-html

Render LCOV `.info` coverage files to static HTML. Zero-install on
any platform with Python 3 — no `pip`, no Perl (`genhtml`), no .NET
runtime (ReportGenerator).

## Quick start

```bash
python3 tools/coverage-html/slang-coverage-html.py coverage.lcov
# → writes ./coverage-html/index.html
open coverage-html/index.html       # macOS
xdg-open coverage-html/index.html   # Linux
start coverage-html\index.html      # Windows
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

## Options

```
slang-coverage-html <input.lcov> [options]

--output-dir PATH      Output directory (default: ./coverage-html/)
--title TEXT           Page title (default: "Coverage report")
--source-root PATH     Directory used as a root when resolving SF: paths
--filter-include GLOB  Include-only glob, applied to SF: path (repeatable)
--filter-exclude GLOB  Exclude glob, applied to SF: path (repeatable)
--quiet                Suppress progress output
```

Function and branch columns render automatically when the input LCOV
carries `FN:` / `FNDA:` / `BRDA:` records. No flag needed to enable
them — they disappear again when the LCOV has none.

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
| **2**  | Branch coverage (`BRDA:`) summary (header + index column); function coverage (`FN:`/`FNDA:`) summary (header + index column + per-file "Functions" table with hit counts and line anchor links)                                                                                                 | **Shipped**                   |
| **2b** | Inline per-line branch column between hit count and source. Collapsed `(hit/total)` summary with tier-colored span (`branchAll` green, `branchPart` amber, `branchNone` red) and per-branch tooltip (`title="br0: N; br1: N; br2: -"`). Gutter widens only on files that carry any BRDA records | **Shipped**                   |
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

37 unit + integration tests cover: LCOV parsing (incl. TN: max-
aggregation, corrupt-input detection, unknown-record tolerance, BRDA
with `-` tokens, FN/FNDA join-by-name), source resolution (path
variants, caching, miss → placeholder), filter globs, tier
thresholds, function/branch percent calcs, per-line branch-cell
rendering (empty / all-taken / partial / none / not-evaluated), CLI
round-trip, empty-input rendering, idempotency modulo timestamp,
foreign-dir overwrite guard, phase-1-regression check against the
real-data phase-2 fixture.

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
├── slang-coverage-html.py         # the renderer (single file)
├── README.md                      # this file
└── tests/
    ├── test_renderer.py           # unit + integration tests
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
