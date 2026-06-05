# Slang compile-time performance suite

A harness that measures **compiler time** (how long `slangc` takes to compile),
not GPU/runtime performance — built to **detect and localize compile-time
regressions** and replay the measurement across historical Slang releases.

Motivation: a compile-time slowdown was observed building Slang for Falcor.
This suite reproduces that class of problem in a controlled, attributable way:
each test stresses one compiler stage, so a regression points at a specific
pass; and the same suite runs unchanged against any release binary, so a
regression points at a specific release.

---

## How it measures

`slangc` is run with `-report-perf-benchmark`, which prints per-phase timers as
`[*] <phase> <count> <ms>`. The runner captures **all** of them per run.

- **Headline metric: `compileInner`** — the full compile (front-end + IR +
  codegen). It *excludes* the fixed ~280 ms core-module load, so it is stable
  and comparable across workloads and versions.
- **Localization** uses the Slang-internal stage timers, which are measured
  *before* any downstream tool (spirv-opt), so they stay comparable across 10+
  months of releases even when bundled tools differ. The timers are **nested**:

  ```
  compileInner
    ├─ frontEndExecute ─── parseTranslationUnit, SemanticChecking, generateIR
    └─ generateOutput ──── linkAndOptimizeIR ── specializeModule, simplifyIR,
                                                 linkIR, unrollLoopsInModule
  ```
  Attribution therefore uses **leaf** timers (a jump in `generateOutput` is just
  its child `linkAndOptimizeIR`, whose jump is its child `specializeModule`…).

- **Robustness:** each data point is `1 warmup + 5 timed` runs; the **min** is
  used for cross-version comparison (least perturbed by scheduling noise).

> **Reading the numbers:** the synthetic workloads are *stress tests built to
> amplify* one pass each. A "3.8×" is a sensitivity figure for that pass, **not**
> a user-facing slowdown. `mdl_dxr` (a real shader) is the realistic end-to-end
> signal.

---

## The tests and what they target

11 workloads run per release. Synthetic ones are generated deterministically by
`workloads.py`, scaled by a size knob `N`; `mdl_dxr` is a real shader corpus.

### Suspected-regression features (deepest workloads)

| Test | What it generates | Targets (compiler stage) | Primary timer |
|---|---|---|---|
| **autodiff** | `N` `[Differentiable]` functions in bounded-depth groups, differentiated forward + reverse, plus a differentiable generic | the **autodiff IR transform** | `linkAndOptimizeIR` |
| **dynamic_dispatch** | one interface with `N` implementations, dispatched through a runtime-typed existential (defeats static specialization → real witness-table dispatch) | **dynamic-dispatch lowering / specialization** | `specializeModule` |
| **diagnostics_errors** | `N` functions each with undefined symbols → ~2N diagnostics (compile fails on purpose) | the **diagnostic-emission path** | `SemanticChecking` |
| **diagnostics_clean** | same shape, but compiles | **size-matched control** — `errors − clean` isolates diagnostic cost | `SemanticChecking` |

### Core compiler-stage tests

| Test | What it generates | Targets | Primary timer |
|---|---|---|---|
| **parse** | `N` trivial functions, long expressions | **lexing/parsing** | `parseTranslationUnit` |
| **sema_generics** | `N` generic functions × 3 type instantiations | **semantic checking / generic instantiation** | `SemanticChecking` |
| **specialization** | a generic `Box<T:IVal>` over `N` distinct types | **generic specialization** | `specializeModule` |
| **inlining** | `N` `[ForceInline]` functions (bounded-depth groups) | **inliner + SSA simplify** | `simplifyIR` |
| **codegen_spirv** | one shader, `N` lines of backend math | **target code emission** | `generateOutput` |
| **module_link** | `N` modules precompiled to `.slang-module`, then linked | **module read + IR link** | `linkIR` |

### Real-shader test

| Test | What it is | Targets |
|---|---|---|
| **mdl_dxr** | the real **MDL/DXR** path-tracer shaders (`shader-slang/MDL-SDK`): `hit.slang` + imported `material.slang` (549 KB), `runtime.slang`, etc., compiled monolithically | **holistic real-world compile time** |

**Compile modes** (per workload, see `manifest.py`): `target` (compile to SPIR-V
with an entry point — triggers the full pipeline incl. specialization/autodiff),
`module` (compile to a downstream-free `.slang-module` — front-end only), `link`
(precompile modules then link). Every workload is GPU-free and external-SDK-free
so it runs headless / in CI.

---

## Quickstart

```bash
# 1. (once) fetch the real-shader corpus
python3 fetch_corpus.py --name mdl

# 2. run the whole suite against a slangc build
python3 bench.py --slangc /path/to/slangc --label dev      # -> results/dev/

# 3. (historical) fetch release binaries and sweep them all
python3 fetch_releases.py                                   # v2025.14 .. v2026.9
python3 sweep.py --samples 5                                # -> results/<tag>/

# 4. analyze + visualize
python3 analyze.py                          # ranked regressions + series.csv/flags.csv
python3 plot.py                             # SVG charts
python3 report.py                           # single self-contained HTML report
```

---

## Files in this suite

### Scripts

| File | Role |
|---|---|
| `workloads.py` | deterministic workload generators, `gen_*(n) -> {filename: source}` |
| `manifest.py` | per-workload spec: invocation, compile mode, primary timers |
| `bench.py` | **test runner** — runs slangc, parses all timers, writes per-run JSON/CSV (merge-on-write) |
| `fetch_corpus.py` | downloads the MDL real-shader corpus (GitHub contents API) |
| `fetch_releases.py` | downloads + caches prebuilt `slangc` per release tag |
| `sweep.py` | **release sweep** — runs `bench.py` against every cached release |
| `analyze.py` | per-`(workload,timer)` series, leaf-attributed step-change detection, diagnostics path-cost |
| `plot.py` | self-contained SVG charts (normalized + absolute log) |
| `report.py` | single self-contained **HTML report** (charts inline + tables) |

### Documents

| Document | What it contains |
|---|---|
| `README.md` | this file — overview, the tests and what they target, quickstart |
| `PLAN.md` | the design/methodology: what to measure, why, the metric and phase plan |
| `CI_PLAN.md` | deployment plan for **per-PR** (soft-fail gate) and **nightly** (trend) CI |

### Generated outputs (gitignored)

| Path | What it is |
|---|---|
| `results/<label>/results.{json,csv}` | every phase timer per run, per release |
| `results/_analysis/series.csv` | long-format time-series, one row per `(workload,timer,release)` |
| `results/_analysis/flags.csv` | ranked step-changes with leaf attribution |
| `results/_analysis/*.svg` | charts |
| `results/_analysis/report.html` | the combined HTML report |
| `releases/` | cached prebuilt `slangc` per tag (large) |
| `corpus/` | fetched real-shader corpora (e.g. MDL) |

---

## Notes / caveats

- **`parse` is sema-dominated:** `parseTranslationUnit` is cheap even for huge
  files; use that timer (not `frontEndExecute`) for the pure-parse signal.
- **Summed timers can exceed `compileInner`:** timers invoked many times
  (e.g. `simplifyIR`) sum across invocations and the profiler is re-entrant;
  compare such timers version-over-version, not as a fraction of the total.
- **`diagnostics_errors` exits non-zero by design**; the runner treats "produced
  real errors + timers" as success and ignores benign missing-`spirv-opt`
  (`E00100`) messages on hosts without that tool.
- **Cross-version error formats:** the runner recognizes both modern
  (`error[E30015]:`) and legacy (`error 30015:`) slangc diagnostics.
