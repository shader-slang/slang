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
- **Branch vs branch — compare a change against a baseline** —
  `compare_branches.py --base master`: builds your working tree + the base ref (in
  a throwaway `git worktree`) on the same machine, benches both, and diffs. Or do
  it by hand with two prebuilt binaries: `bench.py --label base`,
  `bench.py --label head`, `compare.py base head`. Exits non-zero on a regression
  past threshold, so it doubles as a pre-push check.
- **Scaling of one build (compile time vs size N)** — `bench.py … --sweep` then
  `sweep_report.py --label <name>`: per-workload `floor + k·N` curves + stacked
  phase breakdown, to tell a fixed-cost regression from a per-element or
  super-linear one.
- **Across releases** — `fetch_releases.py` (caches platform-matched release
  binaries) + `sweep.py` to bench them all, then `analyze.py` (ranked
  step-changes, leaf attribution) and `report.py` (self-contained HTML, incl.
  per-benchmark linear charts); `breakdown.py` renders per-phase attribution.

> The synthetic workloads amplify one pass each, so a multiplier is a sensitivity
> figure for that pass, not a user-facing slowdown; `mdl_dxr` is the realistic
> end-to-end number.

## CI workflows

Both run on the dedicated, quiesced NVIDIA RGFX perf pool (runner group `nvrgfx`,
labels `[Windows, X64, nvrgfx-perf]`) — a quiesced machine is what makes the
timing trustworthy. They store results in a separate repo,
`shader-slang/slang-compile-perf`, authenticated with the `SLANG_COMPILE_PERF_PAT`
secret (the `PERF_RESULTS_REPO` env overrides the target).

- **`compile-perf-nightly.yml`** — builds tip-of-tree, sweeps into
  `daily/<date>-<sha>/`, runs `track.py register` (stamp + rebuild the tracking
  series), pushes the results repo, then runs `trend.py`. **Manual
  `workflow_dispatch` only right now — the daily `schedule` is commented out;**
  enable it once the suite is validated on the runner and the history is seeded.
  Inputs: `ref` (commit SHA or branch to build; blank = master HEAD, useful for
  backfilling historical daily points), `samples`, `sweep`, `only`. The run label
  and `meta.json` date are derived from the checked-out commit's author date, so
  backfill points sort correctly in the tracking series.
- **`compile-perf-release-sweep.yml`** (`workflow_dispatch`) — downloads prebuilt
  release `slangc` for the runner's platform, sweeps each into `releases/<tag>/`,
  writes `index.json`, stamps `runner.json`, rebuilds, and pushes. **Run with
  `force=true` to re-measure the whole history onto a new runner.** Inputs:
  `since`, `until`, `samples`, `force`.

**Per-PR gate (deferred).** A fast, soft-fail per-PR gate — build the PR head and
its merge-base on the same runner and diff with `compare.py` (baseline-relative,
so machine variance cancels; non-blocking, not a required check) — is **designed
but not part of this phase**. Its engine (`compare.py` / `compare_branches.py`)
exists for local use today. A per-PR gate catches gross step regressions at
review; the nightly trend catches the gradual drift no single PR ever trips —
together they cover both.

### Data model — the tracking series

Absolute compile times are runner-specific, so the series is assembled per machine:

    tracking = [one swept point per release tag]          # the stable baseline
            + [one tip-of-tree point per night,           # post-release daily tail
               dated after the last release]

`track.py` owns it: `register` (stamp a daily run + rebuild), `rebuild` (recompute
`_tracking/tracking.json`), `stamp-runner` (record the fingerprint the history was
built on), `runner-id`, `summary`. Points reduce to per-`(workload, timer)` median
via `analyze.canonical_runs`, so swept multi-size runs collapse to `default_size`
and history vs daily compare like-with-like.

### Drift alert — `trend.py`

After each nightly rebuild, `trend.py` compares the latest point's primary timers
(per workload, always including `compileInner`) against the trailing-N-point
median (default 7), restricted to the **same runner fingerprint**. A metric past
both a relative (`--rel`, default 1.25×) and absolute (`--abs`, default 2 ms)
threshold is flagged — printed, emitted as a GitHub `::error::` annotation +
step-summary row, and the job exits non-zero (after the push, so the data is still
stored). If the latest point's runner differs from the history's, it warns and
compares only same-runner points.

### Runner-change procedure

When the benchmark runner is replaced or updated, `track.py runner-id` changes and
the stored `runner.json` no longer matches. Re-run **compile-perf-release-sweep**
with `force=true` to re-measure every release on the new machine and re-stamp
`runner.json`; until then daily-vs-history comparisons mix runners and are invalid.

## Result layout (the `slang-compile-perf` repo)

    index.json                       release manifest {tag, date, version}
    releases/<tag>/results.json      per-release sweep — the history baseline (source of truth)
    daily/<date>-<sha>/results.json  one tip-of-tree sweep per night
    daily/<date>-<sha>/meta.json     {date, commit, runner, kind}
    runner.json                      {fingerprint, label} the history was built on
    _tracking/tracking.json          derived series consumed by trend.py / plots

`results.json` (all of median/min/mean/stdev per timer) is the only measurement
artifact stored — no CSV; the analysis/report tools read it directly. Transient
and regenerable outputs (`gen/`, `_analysis/`, `_sweep/`, `_breakdown/`, `*.html`,
`*.svg`) are excluded via a `.gitignore` committed directly to the
`slang-compile-perf` repo.

## HTML reports — shader-slang.org/slang-compile-perf

Both CI workflows generate and publish an HTML report after each results push.
`report.py` reads **all** data in the results repo (release history + daily ToT
points) and writes a self-contained report to `_analysis/` (gitignored from the
data branch). The deploy step pushes that output to the `gh-pages` branch of
`shader-slang/slang-compile-perf`, which GitHub Pages serves at
`https://shader-slang.org/slang-compile-perf/`. `report_per_workload.html` is
renamed to `index.html` so that URL is the landing page. Per-workload detail
pages live under `workloads/<name>.html`.

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

| Decision                  | Choice                                                                          | Rationale                                                                                                                                                           |
| ------------------------- | ------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Release binaries          | Prebuilt published per tag, platform-matched (Linux `.tar.gz` / Windows `.zip`) | Fast, reproducible, matches shipped artifacts; source builds only for commit-level bisect                                                                           |
| Measurement flag          | `-report-perf-benchmark`                                                        | Stable across the supported release window; the `detailed` variant only adds sub-timers on newer builds                                                             |
| Headline metric           | `compileInner`, **median** of N timed runs                                      | Excludes the fixed core-module-load floor, so it is stable across releases. Median over min: reflects the typical run and is steadier when run-to-run spread shifts |
| Per-compile floor         | the `minimal` workload's `compileInner`                                         | The N→0 limit — a direct measurement of fixed per-compile cost, not a fitted intercept (which can go negative on convex curves)                                     |
| Timer scope / attribution | all nested phase timers; attribute via **leaf** timers                          | A jump in `compileInner` is traced down `generateOutput → linkAndOptimizeIR → specializeModule`; using leaves avoids double-counting nested timers                  |
| Phase decomposition       | mutually-exclusive buckets (top-down)                                           | Named leaves + `(self)` residuals; if a child timer overshoots its parent it is scaled proportionally so the buckets always sum to `compileInner`                   |
| Output                    | `results.json` only                                                             | JSON holds median/min/mean/stdev per timer; generated sources + compiled outputs go to an auto-removed `--gen-dir` tempdir so the results dir stays scratch-free    |
| Robustness                | 1 warmup + N timed runs (default 5)                                             | The warmup absorbs cold-cache/first-run effects; multiple timed samples + median tame scheduling noise                                                              |
| Determinism               | generators are deterministic (same N → identical bytes)                         | A release sweep compares like with like, and base/head always compile identical inputs                                                                              |
| GPU / SDK dependency      | none                                                                            | Every workload is GPU-free and external-SDK-free, so it runs headless in CI                                                                                         |
| Target                    | `-target spirv -emit-spirv-directly` (text backends use `-target metal`/`wgsl`) | Measures Slang itself, not a downstream `spirv-opt`                                                                                                                 |
| Comparability             | absolute times are **runner-specific**                                          | Every point in a comparison must come from the same machine (see the tracking model + runner fingerprint above)                                                     |

Benchmarking the whole suite is ~1.5–2.5 min per build; building `slangc` (minutes)
dominates wall-clock, so the real constraints are timing noise and runner
contention, not the bench runtime.
