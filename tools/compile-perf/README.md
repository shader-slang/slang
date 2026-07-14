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

## Measurement

`slangc` is run with `-report-perf-benchmark`, which prints per-phase timers as
`[*] <phase> <count> <ms>`. The runner captures **all** of them per run.

- **Headline metric: `compileInner`** — the full compile (front-end + IR +
  codegen). It _excludes_ the fixed ~280 ms core-module load, so it is stable
  and comparable across workloads and versions.
- **Localization** uses the Slang-internal stage timers, which are measured
  _before_ any downstream tool (spirv-opt), so they stay comparable across 10+
  months of releases even when bundled tools differ. The timers are **nested**:

  ```
  compileInner
    ├─ frontEndExecute ─── parseTranslationUnit, SemanticChecking, generateIR
    └─ generateOutput ──── linkAndOptimizeIR ── specializeModule, simplifyIR,
                                                 linkIR, unrollLoopsInModule
  ```

  Attribution therefore uses **leaf** timers (a jump in `generateOutput` is just
  its child `linkAndOptimizeIR`, whose jump is its child `specializeModule`…).

- **Robustness:** each data point is `1 warmup + 5 timed` runs; the **median** is
  saved and used for cross-version comparison (reflects the typical run, and is
  steadier than the min when a build's run-to-run spread shifts). All of
  `median`/`min`/`mean`/`stdev` are kept in `results.json`; the reporting tools
  take `--metric` to switch (default `median`).
- **Memory:** when GNU `/usr/bin/time` is present, peak RSS per compile is also
  captured (`rss_kb`) — a heavier core module inflates memory, not just time.
- **Floor + slope:** `ladder_scaling.py --workload <name>` fits
  `time = floor + slope·N` per release from `--sweep` (multi-size) runs,
  separating a fixed-cost regression (heavier stdlib) from a per-element one
  (a pass got slower) from a scaling one (a rising final-step ratio).

> **Reading the numbers:** the synthetic workloads are _stress tests built to
> amplify_ one pass each. A "3.8×" is a sensitivity figure for that pass, **not**
> a user-facing slowdown. `mdl_dxr` (a real shader) is the realistic end-to-end
> signal.

---

## Workloads

Workloads run per release. Synthetic ones are generated deterministically by
`workloads.py`, scaled by a size knob `N`; `mdl_dxr` is a real shader corpus.

### Suspected-regression features (deepest workloads)

| Test                      | What it generates                                                                                                                                    | Targets (compiler stage)                                                                                                                                                      | Primary timer                                       |
| ------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------- |
| **autodiff**              | `N` `[Differentiable]` functions in bounded-depth groups, differentiated forward + reverse, plus a differentiable generic                            | the **autodiff IR transform**                                                                                                                                                 | `linkAndOptimizeIR`                                 |
| **dynamic_dispatch**      | one interface with `N` implementations, dispatched through a runtime-typed existential (defeats static specialization → real witness-table dispatch) | **dynamic-dispatch lowering / specialization**                                                                                                                                | `specializeModule`                                  |
| **existential_aggregate** | an interface-typed **field** inside a struct (`Scene { IMat m; }`) + `N` impls selected via a switch                                                 | boxing the existential in an aggregate forces **existential-layout legalization** + a witness-per-case specialization blowup (uncovered by the bare-local `dynamic_dispatch`) | `legalizeExistentialTypeLayout`, `specializeModule` |
| **diagnostics_clean**     | `N` functions with distinct compile-time constants, no errors                                                                                        | **semantic checking** at scale — same shape as the old error workload but compiling cleanly, so `SemanticChecking` reflects pure checking cost without diagnostic emission    | `SemanticChecking`                                  |

### Core compiler-stage tests

| Test                           | What it generates                                                       | Targets                                                                                                                                                           | Primary timer                 |
| ------------------------------ | ----------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------- |
| **parse**                      | `N` trivial functions, long expressions                                 | **lexing/parsing**                                                                                                                                                | `parseTranslationUnit`        |
| **sema_generics**              | `N` generic functions × 3 type instantiations                           | **semantic checking / generic instantiation**                                                                                                                     | `SemanticChecking`            |
| **specialization**             | a generic `Box<T:IVal>` over `N` distinct types                         | **generic specialization**                                                                                                                                        | `specializeModule`            |
| **inlining**                   | `N` `[ForceInline]` functions (bounded-depth groups)                    | **inliner + SSA simplify**                                                                                                                                        | `simplifyIR`                  |
| **codegen_spirv**              | one shader, `N` lines of backend math                                   | **target code emission** (SPIR-V, direct)                                                                                                                         | `generateOutput`              |
| **emit_metal** / **emit_wgsl** | the same shader as `codegen_spirv`, emitted to **textual** Metal / WGSL | the **source-emission backend** (`emitEntryPointsSourceFromIR` + target legalization) that `-emit-spirv-directly` skips entirely — no other workload exercises it | `emitEntryPointsSourceFromIR` |
| **module_link**                | `N` modules precompiled to `.slang-module`, then linked                 | **module read + IR link**                                                                                                                                         | `linkIR`                      |

### Type-checking tests

The quietly expensive part of semantic checking: every binary operator and every
cross-type assignment runs **overload resolution** over a candidate set and ranks
**implicit-conversion** costs. `parse` deliberately uses uniform-`float`
arithmetic (each operator matches one overload trivially) and `sema_generics` is
dominated by generic-constraint cost, so neither isolates this. These compile to
`.slang-module` (front-end only), so the signal is `SemanticChecking`, not codegen.

| Test                    | What it generates                                                                                                         | Targets                                                                                                                          | Primary timer      |
| ----------------------- | ------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------- | ------------------ |
| **operator_typecheck**  | `N` functions, each a long arithmetic expression with operands alternating among `float`/`int`/`uint` (typed + literal)   | **operator overload resolution + coercion at every node** — the "checking the types of `1` and `2` in `1 + 2`" cost, ×many nodes | `SemanticChecking` |
| **implicit_conversion** | `N` functions, each a cascade of conversions: scalar widening, splats, mixed initializer lists, vector compose/truncate   | the **coercion / conversion-cost engine** (distinct from operator overloading)                                                   | `SemanticChecking` |
| **overload_resolution** | a large user-defined overload set (`pick`/`pick2`, scalar + vector + 2-arg) called from `N` sites with rotating arg types | **candidate enumeration + conversion-rank comparison** to choose the best match                                                  | `SemanticChecking` |

### Shared-infrastructure & scaling tests

Added after a real investigation (PR #9808) showed that a _fixed per-compile_
regression — the standard module growing, inflating `linkIR`/deserialization for
**every** compile — was nearly invisible to feature-targeted tests. These
isolate the shared machinery and scaling behavior directly.

| Test            | What it generates                                        | Targets                                            | Primary timer                                           |
| --------------- | -------------------------------------------------------- | -------------------------------------------------- | ------------------------------------------------------- |
| **minimal**     | a near-empty shader                                      | the **per-compile floor**: core-module load + link | `linkIR`, `readSerializedModuleIR`, `loadBuiltinModule` |
| **ir_builder**  | one giant straight-line function (`N` trivial int ops)   | **IR construction / dedup / SSA simplify**         | `generateIR`, `simplifyIR`                              |
| **serialize**   | a large module of `N` public functions → `.slang-module` | **IR/AST serialization (write)**                   | `writeSerializedModuleAST/IR`                           |
| **conformance** | `N` structs conforming to a shared interface             | **conformance checking / witness synthesis**       | `SemanticChecking`                                      |
| **loop_unroll** | a `[ForceUnroll]` loop of `N` iterations                 | **loop unrolling + simplify**                      | `unrollLoopsInModule`                                   |

`minimal` is the **regression canary**: cheap enough to run on every PR, and the
single best early warning for "the stdlib got heavier"-class regressions.

### Coverage-gap tests

Each exercises a pass or output path that **no other workload reaches** — found
by probing the dev `slangc` with `-report-perf-benchmark` and noting timers that
never appeared in the suite. All scale by breadth (number of constructs).

| Test                   | What it generates                                                                                                                                 | Targets (compiler stage)                                                                                                                                                                                                                    | Primary timer                                          |
| ---------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------ |
| **resource_aggregate** | `N` structs bundling textures + a sampler + a `StructuredBuffer`, all read live                                                                   | **resource-type legalization**: nesting resources in an aggregate forces `legalizeResourceTypes` to flatten them into bindings (every other workload's only resource is a bare `RWStructuredBuffer`); the timer grows super-linearly in `N` | `legalizeResourceTypes`                                |
| **reflection_layout**  | `N` constant buffers with rich payloads (vectors, matrices, nested `Light[]`/`Material` structs, scalar arrays), compiled with `-reflection-json` | the **parameter binding / layout engine** + reflection serializer — the only large, deeply-typed shader **parameter interface** in the suite (the layout/reflection path no other workload covers)                                          | `compileInner` (+ `frontEndExecute`, `generateOutput`) |
| **control_flow_ssa**   | one entry point with `N` stacked control-flow blocks (nested if/else + bounded loop with break/continue + switch) mutating carried locals         | **SSA construction / CFG simplify**: reassigning locals across branches and back-edges forces phi insertion (`constructSSA` inside `simplifyIR`) — the axis `complexity_ladder` only touches as one of several                              | `simplifyIR`, `frontEndExecute`                        |

### Complexity-scaling test

The single-axis stressors above each isolate **one** pass. `complexity_ladder`
instead ramps _several_ realistic dimensions together — branchy control flow,
generic calls, bounded inner loops, resource reads, dynamic dispatch, and
call-graph depth — so the size knob `N` models a real shader growing from
**simple to highly complex**. Sweep it (`bench.py --only complexity_ladder
--sweep`) and fit with `ladder_scaling.py` (its default workload) to get the holistic
complexity → compile-time curve, separating the fixed **floor** from the
per-unit **slope** and surfacing super-linear bends at high complexity.

| Test                  | What it generates                                                                                       | Targets                                                             | Primary timer                                             |
| --------------------- | ------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------- | --------------------------------------------------------- |
| **complexity_ladder** | a mixed-feature shader (control flow + generics + loops + dispatch + resources, scaled together by `N`) | the **whole pipeline at once**, as a realistic-shader scaling curve | `compileInner` (+ `frontEndExecute`, `linkAndOptimizeIR`) |

### Real-shader test

| Test        | What it is                                                                                                                                                          | Targets                              |
| ----------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------ |
| **mdl_dxr** | the real **MDL/DXR** path-tracer shaders (`shader-slang/MDL-SDK`): `hit.slang` + imported `material.slang` (549 KB), `runtime.slang`, etc., compiled monolithically | **holistic real-world compile time** |

**Compile modes** (per workload, see `manifest.py`): `target` (compile to SPIR-V
with an entry point — triggers the full pipeline incl. specialization/autodiff),
`module` (compile to a downstream-free `.slang-module` — front-end only), `link`
(precompile modules then link). Every workload is GPU-free and external-SDK-free
so it runs headless / in CI.

---

## Quickstart

> Examples use `python3` (macOS/Linux); on **Windows** use `python` (or `py`).
> The scripts are marked executable, so on macOS/Linux you can also run them
> directly — `./bench.py …` instead of `python3 bench.py …`.

```bash
# 1. (once) fetch the real-shader corpus
cp /path/to/MDL-SDK/examples/mdl_sdk/dxr/content/slangified/*.slang corpus/mdl/

# 2. run the whole suite against a slangc build
python3 bench.py --slangc /path/to/slangc --label dev      # -> results/dev/

# 3. (historical) fetch release binaries and sweep them all
python3 fetch_releases.py --since 2025-08-01                # default window start
python3 sweep.py --samples 5                                # -> results/releases/<tag>/

# 4. visualize
python3 report.py                           # per-workload history + phase breakdown HTML
python3 breakdown.py --label <tag>          # phase attribution for one label (stdout table)

# 5. complexity sweep of one build (compile time vs size N), with HTML report
python3 bench.py --slangc /path/to/slangc --label dev --sweep \
    --only resource_aggregate,reflection_layout,control_flow_ssa
python3 sweep_report.py --label dev         # -> results/dev/sweep/sweep_report.html

# 6. (coming in follow-up) check a change for a compile-time regression:
#    bench two slangc binaries, diff with compare.py
```

---

## Files in this suite

### Scripts

| File                                 | Role                                                                                                                                                                                                                                                                                                                                                                                                                                                                             |
| ------------------------------------ | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `lib/workloads.py`                   | deterministic workload generators, `gen_*(n) -> {filename: source}`                                                                                                                                                                                                                                                                                                                                                                                                              |
| `lib/manifest.py`                    | per-workload spec: invocation, compile mode, primary timers                                                                                                                                                                                                                                                                                                                                                                                                                      |
| `bench.py`                           | **test runner** — runs slangc, parses all timers, writes `results.json` (merge-on-write); generated sources go to an auto-removed `--gen-dir` scratch, not the results dir                                                                                                                                                                                                                                                                                                       |
| `fetch_releases.py`                  | downloads + caches prebuilt `slangc` per release tag                                                                                                                                                                                                                                                                                                                                                                                                                             |
| `sweep.py`                           | **release sweep** — runs `bench.py` against every cached release                                                                                                                                                                                                                                                                                                                                                                                                                 |
| `compare.py` / `compare_branches.py` | **local regression check** — base-vs-head diff + one-command branch driver; planned for a follow-up PR                                                                                                                                                                                                                                                                                                                                                                           |
| `track.py`                           | maintains the CI **tracking series** (release history ++ post-release daily ToT points) + runner fingerprint                                                                                                                                                                                                                                                                                                                                                                     |
| `daily_movers.py`                    | **daily progress summary** — suite-wide day boundaries ranked by headline movement with leaf-timer attribution and the commit range to bisect; `--workload <name>` for one workload's per-timer net + biggest step (date + commits)                                                                                                                                                                                                                                              |
| `trend.py`                           | nightly **drift alert** — latest point vs trailing-median, same-runner; GitHub annotations + non-zero exit on regression                                                                                                                                                                                                                                                                                                                                                         |
| `lib/analyze.py`                     | per-`(workload,timer)` series helpers — used as a library by `report.py`, `track.py`, etc. (no standalone CLI yet)                                                                                                                                                                                                                                                                                                                                                               |
| `breakdown.py`                       | **phase attribution** — splits `compileInner` into mutually-exclusive buckets (named leaves + `(self)` residuals); aggregate + per-workload tree; stacked-area **per-release history** (index + per-workload detail pages in `report_per_workload.html`) and **per-sweep** (stacked-area vs N in `sweep_report.html`); standalone CLI: `--label <tag>` prints aggregate + per-workload tables; `--html` writes SVG/HTML; `--workload <name>` prints the full indented timer tree |
| `report.py`                          | single self-contained **HTML report**, cross-release (charts inline + tables)                                                                                                                                                                                                                                                                                                                                                                                                    |
| `ladder_scaling.py`                  | cross-release `floor + slope·N` fit table for any swept workload                                                                                                                                                                                                                                                                                                                                                                                                                 |
| `compare_repro.py`                   | **reproducibility check** — compare a fresh sweep against a saved baseline: per-(release, workload, size) `compileInner` drift, worst-first, with a summary and over-threshold flags                                                                                                                                                                                                                                                                                             |
| `sweep_report.py`                    | **complexity-sweep HTML report** for one build — compileInner scaling curves (index) linking to per-workload pages with the stacked sub-counter-vs-N chart, scaling analysis (floor/k/top-2×), and raw per-size numbers; `--publish DIR` renders the site's sweep landing page + every archived sweep                                                                                                                                                                            |

### Documents

| Document    | What it contains                                                       |
| ----------- | ---------------------------------------------------------------------- |
| `README.md` | this file — overview, the tests and what they target, quickstart       |
| `DESIGN.md` | design decisions, local use cases, CI workflows, and the result layout |

### Generated outputs (gitignored)

The first group is stored in `slang-compile-perf` (the CI results repo). The
second group is regenerable on demand and is gitignored from that repo — generate
them locally by pointing the report scripts at a checkout of `slang-compile-perf`
via `--results <checkout-path>`.

**Stored in `slang-compile-perf`:**

| Path                                           | What it is                                                         |
| ---------------------------------------------- | ------------------------------------------------------------------ |
| `results/releases/<tag>/results.json`          | per-release measurements — source of truth (all timers, all stats) |
| `results/daily/<label>/results.json+meta.json` | nightly ToT sweeps                                                 |
| `results/tracking/tracking.json`               | derived tracking series (release history ++ daily tail)            |

**Regenerable / local only (gitignored from the results repo):**

| Path                                        | What it is                                                |
| ------------------------------------------- | --------------------------------------------------------- |
| `results/analysis/*.svg`                    | charts                                                    |
| `results/analysis/report_per_workload.html` | per-workload stacked-area history + drill-down pages      |
| `results/releases/<tag>/sweep/`             | complexity-sweep report for swept releases                |
| `releases/`                                 | cached prebuilt `slangc` per tag (large, gitignored)      |
| `corpus/`                                   | fetched real-shader corpora, e.g. MDL (large, gitignored) |

---

## Notes / caveats

- **`parse` is sema-dominated:** `parseTranslationUnit` is cheap even for huge
  files; use that timer (not `frontEndExecute`) for the pure-parse signal.
- **Summed timers can exceed `compileInner`:** timers invoked many times
  (e.g. `simplifyIR`) sum across invocations and the profiler is re-entrant;
  compare such timers version-over-version, not as a fraction of the total.
- **Cross-version error formats:** the runner recognizes both modern
  (`error[E30015]:`) and legacy (`error 30015:`) slangc diagnostics.
