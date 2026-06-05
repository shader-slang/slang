# Perf-suite CI plan (per-PR + nightly)

Design only — not yet wired up. Two tiers, per the decision: a **fast per-PR
gate** (soft-fail / warning, non-blocking) and a **full nightly** trend job.

## Measured cost (basis for the design)

Per release, 11 workloads × (1 warmup + 5 timed) runs:

| slangc | bench wall-clock |
|---|---|
| pre-regression (v2026.5) | ~1.4 min |
| post-regression (v2026.7/.8) | ~2.3 min |

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

**Trigger:** `pull_request` to `master`, `paths-ignore` docs, skip drafts.
**Runner:** `[self-hosted, benchmark]` (same dedicated machine).
**Behavior on regression:** the perf job **fails (visible red X)** but is **NOT a
required check**, so it never blocks merge; it also posts a PR comment table.
(Implementation: the compare script exits non-zero on regression; leave the job
out of branch-protection required checks.)

**Why baseline-relative:** absolute thresholds drift with hardware/runner state.
Build **both** the PR HEAD and its **merge-base** on the *same* machine in the
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
