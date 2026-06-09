# Perf-suite CI plan (per-PR + nightly)

Two tiers: a **fast per-PR gate** (soft-fail / warning, non-blocking — design
only, below) and a **full nightly + release-history** time-series, which is now
**implemented** (see "Implemented" section at the end).

## Measured cost (basis for the design)

Per release, 11 workloads × (1 warmup + 5 timed) runs:

| slangc                       | bench wall-clock |
| ---------------------------- | ---------------- |
| pre-regression (v2026.5)     | ~1.4 min         |
| post-regression (v2026.7/.8) | ~2.3 min         |

Benchmarking is **~1.5–2.5 min**; building slangc (~5–20 min) dominates either
tier. So adding perf to a PR that already builds slangc is cheap in wall-clock —
the real constraints are **timing noise** and **runner contention**, not runtime.

## Existing infra to reuse

- `.github/workflows/benchmark.yml` — per-PR MDL benchmark on a dedicated
  `runs-on: [Windows, self-hosted, benchmark]` runner; skips drafts; checks out
  `shader-slang/MDL-SDK` sparse (`.../slangified`). **The dedicated quiesced
  runner is what makes timing trustworthy — reuse it.**
- `.github/workflows/push-benchmark-results.yml` — on push to master, runs the
  MDL benchmark and pushes `benchmarks.json` (+ commit metadata) to a separate
  results repo `shader-slang/slang-material-modules-benchmark`. **Reuse this
  results-repo pattern for the nightly time-series.**
- In CI, get the MDL corpus via `actions/checkout` of MDL-SDK sparse (as those
  workflows do) and copy into `perf-suite/corpus/mdl/`. `fetch_corpus.py`'s API
  download is only needed on this proxy-restricted host, not on GH runners.

## Tier 1 — per-PR fast gate (soft-fail / warning)

> **Status:** the diff script `compare.py` is built and used **locally** by devs
> (bench base + head on one machine, then `compare.py base head`). The automated
> `pull_request` workflow below is **not deployed yet** — design only.

**Trigger:** `pull_request` to `master`, `paths-ignore` docs, skip drafts.
**Runner:** `[self-hosted, benchmark]` (same dedicated machine).
**Behavior on regression:** the perf job **fails (visible red X)** but is **NOT a
required check**, so it never blocks merge; it also posts a PR comment table.
(Implementation: the compare script exits non-zero on regression; leave the job
out of branch-protection required checks.)

**Why baseline-relative:** absolute thresholds drift with hardware/runner state.
Build **both** the PR HEAD and its **merge-base** on the _same_ machine in the
same job and compare — machine variance cancels.

**Steps:**

1. `checkout` PR with `fetch-depth: 0` (need merge-base), submodules.
2. common-setup; build slangc (release) → HEAD slangc.
3. `git worktree add` the merge-base commit; build → BASE slangc.
4. checkout MDL-SDK sparse → `corpus/mdl/`.
5. `bench.py --slangc <BASE> --label base --only <subset> --samples 3`
   then same with `<HEAD> --label head`.
6. `compare.py base head` (new, small): for each workload's primary timers,
   `Δ% = head.min/base.min − 1`; if any `Δ% > 15%`, emit `::warning::`
   annotations + a PR-comment table and `exit 1`.

**Subset + sizes (target ~60 s/build of benchmarking):**
`autodiff`(N=100), `dynamic_dispatch`(N=100), `diagnostics_errors`+`diagnostics_clean`(N=200),
`specialization`(N=150), `inlining`(N=200), `mdl_dxr`. (Add a `--profile pr`
preset to manifest so sizes live in one place.)

**Caveat to document in output:** synthetic numbers are amplified sensitivity
figures, not user-facing slowdowns; `mdl_dxr` is the realistic one.

## Tier 2 — nightly full suite

**Trigger:** `schedule` (nightly cron) + `workflow_dispatch`.
**Runner:** `[self-hosted, benchmark]`.

**Steps:**

1. checkout master; common-setup; build release slangc.
2. checkout MDL-SDK sparse → `corpus/mdl/`.
3. `bench.py --slangc <slangc> --label <YYYY-MM-DD>-<sha> --samples 7`
   (weekly: add `--sweep` for scaling curves → catches O(N)→O(N²)).
4. Push `results.json` (+ commit subject/hash, like push-benchmark-results.yml)
   to a perf results repo / branch, one entry per night.
5. `trend.py` (new): compare tonight's primary timers vs the trailing-N-night
   median; on a step-change beyond threshold, fail the job and open/annotate a
   GitHub issue (or use benchmark-action's alert-comment). This is what catches
   the **gradual drift** a per-PR gate structurally misses (e.g. the parse/sema
   creep at v2026.1, the autodiff jump in the v2026.5–.9 window).

## Why both

- Per-PR alone misses gradual drift — no single PR trips a 15% step.
- Nightly alone can't stop a bad PR before merge.
- Together: PR gate flags gross regressions at review (non-blocking, low
  friction); nightly tracks the long-term trend and alerts on creep.

## Small prerequisites before wiring up

- Add size **presets** to the manifest (`pr` vs `full`) so CI sizes are declared
  once, not hard-coded in YAML.
- Add `compare.py` (two-label diff → annotations + comment, exit code) and
  `trend.py` (series vs trailing median → alert).
- Decide the results-store repo for nightly (reuse the MDL one or a new
  `slang-compile-perf` repo).

---

# Implemented: nightly ToT + release-history resync + tracking series

The time-series tier is wired up as two GitHub Actions workflows on the dedicated
benchmark runner, plus `track.py` (data model) and `trend.py` (drift alert).
`compare.py` (base-vs-head diff) is implemented as a **local dev tool**; the
automated per-PR _workflow_ (Tier 1 above) is intentionally not deployed yet.

## Data model

Absolute compile times are **runner-specific**, so every point in a comparison
must come from the _same_ machine. The **tracking series** is:

    tracking = [per-release history, all on the current runner]
               ++ [daily tip-of-tree (ToT) points dated after the last release]

- **Per-release history** — one swept point per release tag, the stable baseline.
- **Daily ToT** — one sweep of master HEAD per night, appended after the last
  release. When a new release ships, a release-sweep run adds its point and the
  daily tail simply continues after it.

`track.py` (stdlib) owns this: `register` (stamp a daily run + rebuild),
`rebuild` (recompute `_tracking/tracking.json`), `stamp-runner` (record the
runner fingerprint the history was built on), `runner-id`, `summary`. Points are
reduced to per-(workload, timer) `min` via `analyze.canonical_runs`, so swept
(multi-size) runs collapse to `default_size` and history vs daily compare
like-with-like.

## Storage layout (the `slang-compile-perf` results repo)

    index.json                   release manifest {tag,date,version} (fetch_releases.py)
    <tag>/results.json           per-release sweep — the history baseline
    daily/<date>-<sha>/results.json + meta.json   one ToT sweep per night
    runner.json                  {fingerprint,label} the history was built on
    _tracking/tracking.json      derived series consumed by plots / trend.py

## Workflows

- **`.github/workflows/compile-perf-nightly.yml`** — `schedule` (06:00 UTC) +
  `workflow_dispatch`. Builds ToT, sweeps into `daily/<date>-<sha>/`, runs
  `track.py register`, pushes the results repo, then runs `trend.py`. Inputs:
  `samples`, `sweep`, `only`.
- **`.github/workflows/compile-perf-release-sweep.yml`** — `workflow_dispatch`
  only. Downloads prebuilt release `slangc` for this runner's platform
  (`fetch_releases.py`, now platform-aware: Linux `.tar.gz` / Windows `.zip`),
  sweeps each into `<tag>/`, copies `index.json`, `stamp-runner`, `rebuild`,
  pushes. **Run with `force=true` to resync the whole history onto a new
  runner.** Inputs: `since`, `until`, `samples`, `force`.

## Drift alert (`trend.py`)

After each nightly rebuild, `trend.py` compares the latest point's primary timers
(per workload, always incl. `compileInner`) against the trailing-N-point median
(default 7), restricted to the **same runner fingerprint**. A metric past both a
relative (`--rel`, default 1.25×) and absolute (`--abs`, default 2 ms) threshold
is flagged: printed, emitted as a GitHub `::error::` annotation + step-summary
row, and the job exits non-zero (after the push, so data is still stored). If the
latest point's runner differs from the history's, it warns and compares only
same-runner points (prompting a release-sweep resync). This catches the gradual
drift a per-PR step gate misses.

## Runner-change ("recreate the history") procedure

When the benchmark runner is replaced/updated, `track.py runner-id` changes;
the stored `runner.json` no longer matches. Re-run **compile-perf-release-sweep**
with `force=true` to re-measure every release on the new machine and re-stamp
`runner.json`. Until then, daily-vs-history comparisons mix runners and are
invalid (a future `trend.py` should refuse to compare across fingerprints).

## Prerequisites before enabling

- Create the `shader-slang/slang-compile-perf` results repo and a
  `SLANG_COMPILE_PERF_PAT` secret with push access (mirrors the MDL
  `slang-material-modules-benchmark` + `SLANG_MDL_BENCHMARK_RESULTS_PAT`
  pattern). Both workflows read `PERF_RESULTS_REPO` env if you point elsewhere.
- Seed the history once via a manual **compile-perf-release-sweep** run.
