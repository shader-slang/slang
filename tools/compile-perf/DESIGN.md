# Slang compile-time performance suite — design

Measures **how long `slangc` takes to compile** (compiler time, not GPU/runtime),
to detect compile-time regressions, localize them to a specific compiler stage,
and compare a build against historical releases. Each synthetic workload stresses
one compiler stage; `mdl_dxr` (a real shader corpus) is the end-to-end signal.
See `README.md` for the workload list and `manifest.py` for the full spec.

## Local use cases

Every script is runnable directly (`./bench.py …` on macOS/Linux; `python bench.py …`
on Windows). See `README.md` Quickstart for copy-paste commands.

- **Benchmark one build** — `bench.py --slangc <path> --label <name>`: run the
  suite (or a `--only` subset) against a slangc binary; writes
  `results/<name>/results.json`.
- **Branch vs branch — compare a change against a baseline** — bench two
  slangc binaries on the same machine (`bench.py --label base`, then
  `bench.py --label head`) and diff with `compare.py base head`. A
  one-command driver (`compare_branches.py`) that builds both sides via a
  git worktree is planned for a follow-up PR.
- **Scaling of one build (compile time vs size N)** — `bench.py … --sweep` then
  `sweep_report.py --label <name>`: per-workload scaling curves with a
  floor-subtracted power-law fit (`(t − floor) = a·N^k`) + stacked phase
  breakdown, to tell a fixed-cost regression from a per-element or
  super-linear one.
- **Across releases** — `fetch_releases.py` (caches platform-matched release
  binaries) + `sweep.py` to bench them all, then `analyze.py` (ranked
  step-changes, leaf attribution) and `report.py` (self-contained HTML, incl.
  per-benchmark linear charts); `breakdown.py` renders per-phase attribution.

> The synthetic workloads amplify one pass each, so a multiplier is a sensitivity
> figure for that pass, not a user-facing slowdown; `mdl_dxr` is the realistic
> end-to-end number.

## CI workflows

Both run on the dedicated, quiesced NVIDIA RGFX perf pool, reached through the
VM-based GitHub bridge (labels `[Windows, X64, nvrgfx-perf-kernelvm-bridge]`;
the pre-VM `nvrgfx` runner group + `nvrgfx-perf` label pair was retired
2026-07) — a quiesced machine is what makes the timing trustworthy. They store results in a separate repo,
`shader-slang/slang-compile-perf`, authenticated with the `SLANG_COMPILE_PERF_PAT`
secret (the `PERF_RESULTS_REPO` env overrides the target).

- **`nightly-mdl-perf-test.yml`** — builds tip-of-tree, sweeps into
  `daily/<date>-<sha>/`, runs `track.py register` (stamp + rebuild the tracking
  series), pushes the results repo, then runs `trend.py`. **Manual
  `workflow_dispatch` only right now — the daily `schedule` is commented out;**
  enable it once the suite is validated on the runner and the history is seeded.
  Inputs: `ref` (commit SHA or branch to build; blank = master HEAD, useful for
  backfilling historical daily points), `samples`, `sweep` (default `false` — opt-in,
  ~4x runtime: dispatch with `sweep=true` to also collect the multi-size scaling
  ladders), `only`, and `publish` (default `true`). With `publish=false` the run measures only: results are
  uploaded as a run artifact and the results repo, tracking series, pages, and
  trend check are untouched — the mode for one-off measurements (bisect points,
  suspect commits) that must not pollute the series. Because daily labels are
  keyed by the swept commit's date, several points can share a date; the
  workflow therefore passes the label it registered to `trend.py --label` so
  the trend check judges exactly this run's point rather than a same-date
  sibling. The run label and `meta.json` date are derived from the checked-out
  commit's author date, so backfill points sort correctly in the tracking
  series.
- **`compile-perf-release-sweep.yml`** (`workflow_dispatch`) — downloads prebuilt
  release `slangc` for the runner's platform, sweeps each into `releases/<tag>/`,
  writes `index.json`, stamps `runner.json`, rebuilds, and pushes. **Run with
  `force=true` to re-measure the whole history onto a new runner.** Inputs:
  `since`, `until`, `samples`, `sweep` (default `false`, opt-in), `force`.

**Site structure (2026-07 redesign).** `report.py` renders a landing page
(`index.html`: status strip + navigation cards) and four cadence-split pages,
one per family × cadence: `api-tot.html` / `api-releases.html` (api/rt
workloads) and `microbench-tot.html` / `microbench-releases.html` (compiler
workloads). The `*-releases` pages chart the release-only axis (official
prebuilt binaries, minor releases plus patch releases from v2026.13 on); the
`*-tot` pages chart the daily tip-of-tree axis (runner-built, trailing 30
points) and open with the family's top-movers table. The cadences get
separate axes because they differ in build provenance (official toolchain vs
the runner's MSVC): each chart is internally comparable, the boundary is not —
a methodology note on both pages says so. Per-workload detail pages carry the
same two-chart split. trend.py's nightly judgment uses a DAILY-only baseline
for the same reason (`--baseline-kind`). New releases (majors and patch
releases from v2026.13) are swept into the history by the nightly's
new-release check (`new_release_check.py`) the night they ship — no manual
resync; a failed new-release sweep removes its partial results and retries the
next night.

**Sweep publication policy — a landing page plus every archived sweep.**
Sweeping is opt-in on both workflows (`sweep=true`), so sweeps are few and all
of them are served: `sweep_report.py --publish` (run by both workflows' report
steps) renders `analysis/sweep/index.html` — the landing page listing every
archived sweep, dailies newest-first with the newest as the headline, releases
in release order — plus each sweep's full page set under
`analysis/sweep/<label>/`. Everything regenerates from repo data on each
deploy, so old sweeps persist and an empty archive still yields a valid index.
`ladder_scaling.py` remains the cross-release floor/slope fit table over the
same data.

**Per-PR gate and local comparison tools (deferred).** A fast, soft-fail
per-PR gate — build the PR head and its merge-base on the same runner and diff
(baseline-relative, so machine variance cancels; non-blocking, not a required
check) — is **designed but not part of this phase**. `compare.py` and
`compare_branches.py` will be added in a follow-up PR. A per-PR gate catches
gross step regressions at review; the nightly trend catches the gradual drift
no single PR ever trips — together they cover both.

### Data model — the tracking series

Absolute compile times are runner-specific, so the series is assembled per machine:

    tracking = [one swept point per release tag]          # the stable baseline
            + [one tip-of-tree point per night,           # post-release daily tail
               dated after the last release]

`track.py` owns it: `register` (stamp a daily run + rebuild), `rebuild` (recompute
`tracking/tracking.json`), `stamp-runner` (record the fingerprint the history was
built on), `runner-id`, `summary`. Points reduce to per-`(workload, timer)` median
via `analyze.canonical_runs`, so swept multi-size runs collapse to `default_size`
and history vs daily compare like-with-like.

Points sort by `(date, commit_time, label)`. The full committer timestamp
matters because daily labels carry only the commit's DATE, and same-date
siblings are common (master's HEAD is usually committed the previous day;
backfills re-measure old dates) — without it, within-date order would fall to
the short SHA's hex spelling, which is unrelated to code order. The label
remains the deterministic fallback for points registered before `commit_time`
existed.

### Drift alert — `trend.py`

After each nightly rebuild, `trend.py` judges one point's primary timers (per
workload, always including `compileInner`) against the trailing-N-point median
(default 7) of same-runner points strictly before it in series order. The
nightly passes `--label` with the label it just registered, so the judged point
is pinned to this run's registration — daily labels are keyed by the swept
commit's date, so several points can share a date and "the latest point" can be
a same-date sibling. Without `--label` (ad-hoc CLI use) the last point is
judged. A metric past both a relative (`--rel`, default 1.25×) and absolute
(`--abs`, default 2 ms) threshold is flagged — printed, emitted as a GitHub
`::error::` annotation + step-summary row, and the job exits non-zero (after
the push, so the data is still stored). If the judged point's runner differs
from the history's, it warns and compares only same-runner points.

### Runner-change procedure

When the benchmark runner is replaced or updated, `track.py runner-id` changes and
the stored `runner.json` no longer matches. Re-run **compile-perf-release-sweep**
with `force=true` to re-measure every release on the new machine and re-stamp
`runner.json`; until then daily-vs-history comparisons mix runners and are invalid.

## Result layout (the `slang-compile-perf` repo)

    index.json                       release manifest {tag, date, version}
    releases/<tag>/results.json      per-release sweep — the history baseline (source of truth)
    daily/<date>-<sha>/results.json  one tip-of-tree sweep per night
    daily/<date>-<sha>/meta.json     {date, commit, commit_time, runner, kind}
    runner.json                      {fingerprint, label} the history was built on
    tracking/tracking.json          derived series consumed by trend.py / plots

Local / ad-hoc `bench.py` runs (e.g. `bench.py --label dev`) write to a third
layout — `<results>/<label>/results.json` — at the results root rather than under
`releases/` or `daily/`. `analyze.results_dir_for()` searches all three
conventions in order, so local results are handled transparently by all tools.

`results.json` (all of median/min/mean/stdev per timer) is the only measurement
artifact stored — no CSV; the analysis/report tools read it directly. Transient
and regenerable outputs (`gen/`, `analysis/`, `sweep/`, `breakdown/`, `*.html`,
`*.svg`) are excluded via a `.gitignore` committed directly to the
`slang-compile-perf` repo.

## HTML reports — shader-slang.org/slang-compile-perf

Both CI workflows generate and publish an HTML report after each results push.
`report.py` reads **all** data in the results repo (release history + daily ToT
points) and writes a self-contained report to `analysis/` (gitignored from the
data branch). The deploy step pushes that output to the `gh-pages` branch of
`shader-slang/slang-compile-perf`, which GitHub Pages serves at
`https://shader-slang.org/slang-compile-perf/`. `report.py` writes
`index.html` directly (the landing page); `report_per_workload.html` is kept
as a redirect for old bookmarks. Per-workload detail
pages live under `workloads/<name>.html`. Panels render in the CANONICAL order —
the `manifest.WORKLOADS` list order (real-world first, then pipeline stages
front end → back end; see `manifest.display_order`) — so the page layout stays
constant instead of reshuffling with cost drift; workloads present in stored
results but absent from the manifest render at the end rather than vanishing.

Both steps use `continue-on-error: true` — a report failure never blocks the
trend check (nightly) or marks the release sweep red.

**One-time setup**: create a `gh-pages` branch on `shader-slang/slang-compile-perf`
and enable GitHub Pages (Settings → Pages → source: `gh-pages`). The `SLANG_COMPILE_PERF_PAT`
secret already covers pushes to that repo.

## Prerequisites before enabling CI

- Grant `shader-slang/slang` access to the `nvrgfx` runner group (otherwise jobs
  queue forever with no eligible runner).
- Create the `shader-slang/slang-compile-perf` results repo + a
  `SLANG_COMPILE_PERF_PAT` secret with push access (mirrors the MDL
  `slang-material-modules-benchmark` + `SLANG_MDL_BENCHMARK_RESULTS_PAT` pattern).
- Seed the history once via a manual **compile-perf-release-sweep** run, then —
  when ready — uncomment the nightly `schedule`.

## Design decisions

| Decision                  | Choice                                                                          | Rationale                                                                                                                                                                                                     |
| ------------------------- | ------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Release binaries          | Prebuilt published per tag, platform-matched (Linux `.tar.gz` / Windows `.zip`) | Fast, reproducible, matches shipped artifacts; source builds only for commit-level bisect                                                                                                                     |
| Measurement flag          | `-report-perf-benchmark`                                                        | Stable across the supported release window; the `detailed` variant only adds sub-timers on newer builds                                                                                                       |
| Headline metric           | `compileInner`, **median** of N timed runs                                      | Excludes the fixed core-module-load floor, so it is stable across releases. Median over min: reflects the typical run and is steadier when run-to-run spread shifts                                           |
| Per-compile floor         | the `minimal` workload's `compileInner`                                         | The N→0 limit — a direct measurement of fixed per-compile cost, not a fitted intercept (which can go negative on convex curves)                                                                               |
| Timer scope / attribution | all nested phase timers; attribute via **leaf** timers                          | A jump in `compileInner` is traced down `generateOutput → linkAndOptimizeIR → specializeModule`; using leaves avoids double-counting nested timers                                                            |
| Platform-bound workloads  | `WorkloadSpec.platforms` gates the DEFAULT set; `--only` overrides              | The default suite must pass on contributor machines (macOS/Linux) without dxc/nvrtc, but silently hiding a workload misreports coverage — bench prints a `[skip]` note; naming one explicitly runs it anyway  |
| Downstream workloads      | `downstream_required`: missing-toolchain diagnostics are REAL errors            | slangc emits its internal timers before the downstream handoff, so without this a host missing dxc/nvrtc would record timers and report OK with no DXIL/PTX produced — the opposite of the workload's purpose |
| Phase decomposition       | mutually-exclusive buckets (top-down)                                           | Named leaves + `(self)` residuals; if a child timer overshoots its parent it is scaled proportionally so the buckets always sum to `compileInner`                                                             |
| Output                    | `results.json` only                                                             | JSON holds median/min/mean/stdev per timer; generated sources + compiled outputs go to an auto-removed `--gen-dir` tempdir so the results dir stays scratch-free                                              |
| Robustness                | 1 warmup + N timed runs (default 5)                                             | The warmup absorbs cold-cache/first-run effects; multiple timed samples + median tame scheduling noise                                                                                                        |
| Determinism               | generators are deterministic (same N → identical bytes)                         | A release sweep compares like with like, and base/head always compile identical inputs                                                                                                                        |
| GPU / SDK dependency      | none                                                                            | Every workload is GPU-free and external-SDK-free, so it runs headless in CI                                                                                                                                   |
| Target                    | `-target spirv -emit-spirv-directly` (text backends use `-target metal`/`wgsl`) | Measures Slang itself, not a downstream `spirv-opt`                                                                                                                                                           |
| Comparability             | absolute times are **runner-specific**                                          | Every point in a comparison must come from the same machine (see the tracking model + runner fingerprint above)                                                                                               |

Benchmarking the whole suite is ~1.5–2.5 min per build; building `slangc` (minutes)
dominates wall-clock, so the real constraints are timing noise and runner
contention, not the bench runtime.

## Planned workloads — known coverage gaps (updated 2026-07-09)

The suite now covers every source-emitting backend (SPIR-V direct, Metal, WGSL,
HLSL, GLSL, CUDA via the `emit_*` workloads) plus the two downstream compilers
available on the Windows perf runner (`codegen_dxil` through dxc,
`codegen_ptx` through nvrtc), and the API path (`api_*` workloads:
session-setup, many-kernels, module graph, reflection, link-time
specialization — closing the `createGlobalSession` gap from discussion #6579).
Remaining known gaps:

| Planned workload               | Exercises                       | Cost to add                                                                                                              |
| ------------------------------ | ------------------------------- | ------------------------------------------------------------------------------------------------------------------------ |
| host/C++ (`-target cpp`, LLVM) | Host-callable / slang-llvm path | Blocked: nightly builds with `SLANG_SLANG_LLVM_FLAVOR=DISABLE` (prebuilt slang-llvm MSVC CRT link issue); fix that first |

Non-target gaps (no workload covers these at all):

- preprocessor/macro-heavy compile (expansion cost has no dedicated signal)
- capability-checking overhead

Related tooling follow-ups tracked alongside these (from the 2026-07 VM runner
migration measurements): per-run/unit-aware calibration in the trend check
(stamp the host unit into `meta.json`; fixed per-unit offsets span ~3%),
repeat-on-anomaly for the rare 5-8% single-workload spikes, parallel dispatch
support (per-ref concurrency group + push retry), and report label
disambiguation when multiple same-date daily points exist.

## API-path workloads — covering the application-integration dimension (2026-07-07)

An internal real-application compilation benchmark (pytest-driven, compiling a
renderer's shader library through the compilation API via slangpy) catches
regressions this suite structurally cannot, because the suite drives `slangc`
one-shot CLI compiles of single programs. Its 2026-07 data showed two signals:
a ~+32% overall / +60%-on-small-programs Slang-compile jump at the 2026.5.2 →
2026.12 release boundary, and a further +6% overall / +10–14%-on-small-programs
step between 2026.12.2 and master `f4975a7` (2026-07-03) — the latter matching
this suite's own `module_link` +5.6–7.3% finding, but amplified. The relative
regression being largest on ~100–130 ms programs is a fixed per-compile-overhead
signature: session setup, module loading, and link cost paid once per generated
kernel. The plan below builds a public equivalent that covers those dimensions.

**Phase 1 — API-driver workloads (no new dependencies).** A single-file C++
tool, `tools/compile-perf/native/api-driver.cpp`, that `dlopen`s a given
release's `libslang` (so one host-built binary measures every release in the
ladder — the COM ABI is append-only by contract) and emits timers in the same
`[*] name  count  N.NNms` format `bench.py` already parses. The workloads:

| Workload               | Exercises                                                                                                                                                         | Shape                                                                                                                                           |
| ---------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------- |
| `api_session_create`   | `createGlobalSession` + `createSession` (core-module deserialization; the #6579 complaint)                                                                        | N create/destroy iterations, no sources                                                                                                         |
| `api_many_kernels`     | per-compile fixed overhead: N small distinct kernels through loadModule → composite → link → getEntryPointCode in one session                                     | the dimension that amplified the 2026-07 regressions                                                                                            |
| `api_module_graph`     | import resolution + checking + link over a deep, realistic module DAG (interfaces/generics/conformances), loaded through the API                                  | replaces `module_link`'s flat-import shape with a renderer-library-like graph                                                                   |
| `api_module_graph_bin` | the same DAG resolved from serialized `.slang-module` binaries — the import path where the 2026-07-03 regression (#11952) lives; source loads were flat across it | source-load + `writeToFile` setup, then a fresh session re-loads via binaries (guarded: fails loudly if resolution falls back to source)        |
| `api_reflection`       | `getLayout` + full parameter/type-layout walks (50 per program) over parameter-rich kernels — the binding-table query pattern                                     | driver resolves the `spReflection_*` C API via dlsym (the slang.h accessors are inline wrappers over imports a dlopen tool must resolve itself) |
| `api_specialize`       | `IEntryPoint::specialize` per impl type + link + codegen per variant — the one-kernel-per-material pattern                                                        | one module: interface + N conforming structs + generic `computeMain<T>`                                                                         |

`bench.py` gains an `api` mode (timed command = driver + libslang path derived
from `--slangc`) and a build-once step for the driver; everything downstream
(results.json, track/trend/report) is unchanged.

**Enablement status:** the api workloads are part of the TRACKED nightly set
(the nightly passes `--api`) and render in the report's own "API-path
workloads" section, stacked by driver phase with `apiTotal` as the top edge
(they have no `compileInner`, so they are excluded from the compiler grid).
The release history baseline backfills through the normal
**compile-perf-release-sweep** dispatch: `sweep.py --api` counts the api
workloads as required, so releases measured before they existed are simply
re-benched (full suite, same runner) on the next run — no `force` needed.

**Planned API-path extensions (not yet implemented):**

- **Concurrent compilation** — a thread pool issuing `getEntryPointCode` for
  many kernels concurrently (the documented experimental threading surface).
  Engines compile in parallel; a contention/locking regression is invisible to
  every current workload. Needs care: wall-clock per kernel is no longer the
  metric — throughput (kernels/s at k threads vs 1 thread) is.

**Per-PR light / nightly heavy split (planned):**

- All api workloads scale linearly in `n` and are cheap (whole api set ≈ 25 s
  at nightly sizes), so the split is a knob, not a redesign: add a
  `--profile light|heavy` to `bench.py` mapping to per-workload sizes and
  sample counts (light ≈ n/4, 3 samples, ~10 s, resolves ≥5–10% steps; heavy =
  current defaults or larger with per-unit calibration, resolves 2–3%).
- The per-PR gate (deferred above) is bottlenecked by the two builds, not the
  bench: drop LTO for the gate (relative A/B stays valid, ~halves build time)
  and lean on sccache. Capacity on the quiesced pool is the real constraint —
  gate must be opt-in (a `perf-check` label or a path filter on compiler
  sources), never unconditional per-PR.

**Phase 2 — renderer-shaped generated corpus (`rt_renderer`).** The internal
corpus cannot be published, and forking an external shader library (even a
public one) was rejected too: it adds license/attribution surface, fork
maintenance, and corpus-drift risk for no benefit over a generator. Instead, a
close-enough ORIGINAL test case, generated like every other workload
(deterministic: same n → identical bytes, so no pinning or resync-on-drift):

- a renderer-shaped module library (~100 modules at the default n=24
  materials; n is the size dial): a utility layer (math/sampling/color), a scene
  layer (lights, camera, intersection), and a material system — `IMaterial` /
  `IBSDF` interfaces with N conforming material modules carrying
  texture/sampler/cbuffer parameters and eval/sample methods;
- kernels that import the WHOLE library per program (the few×heavy shape that
  dominated the internal data — each of its programs paid 100–700 ms of Slang
  compile because of library imports, unlike `api_many_kernels`' many×tiny);
- raygen/closesthit/miss entry points composed into one program through
  `createCompositeComponentType` (the driver's rt-composite mode — this
  delivers the RT multi-entry-point extension), plus a compute variant so the
  corpus also runs GPU-less;
- one kernel variant per material through `IEntryPoint::specialize` — the
  link-time-specialization pattern against interface-heavy code, which
  `api_specialize`'s single-module version only approximates.

**Phase 3 (deferred) — slangpy-in-CI.** Nightly slangpy kernel generation
against the local slang build (the `SGL_LOCAL_SLANG` hooks). Closest
replication of the customer path but needs a Python env + slangpy build on the
perf runner; only pursue if Phases 1–2 prove insufficient.

**Acceptance criterion.** Sweep the Phase-1 workloads over the same variant
ladder as the internal benchmark's data (2026.5.2, 2026.12, 2026.12.2,
`f4975a7`) and confirm they reproduce both signals (the release-boundary jump
and the 2026-07-03 step). If they do, the dimension is captured.
