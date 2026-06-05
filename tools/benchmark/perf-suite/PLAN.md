# Slang compile-time performance suite — plan

## Goal

Detect and localize **compile-time** performance regressions in the Slang
compiler. Originally surfaced while building Slang for Falcor
(`nvresearch-gfx/Tools/falcor2` pipeline). Suspected regressed areas:
**autodiff**, **diagnostics**, **dynamic dispatch**. We also want general
compile-perf coverage of the main compiler stages, and the ability to replay
the suite against every Slang release over the last ~10 months to find *which
release* a regression entered.

## Decisions (locked)

1. **Release source:** prebuilt published `linux-x86_64` binaries per tag (fast,
   reproducible, matches shipped artifacts). Source builds only later, if we
   need commit-level bisect inside a flagged release range.
2. **Scope (first cut):** deep workloads for the 3 suspected features +
   ~6 core compiler-stage buckets. Expand after it lands.
3. **Home:** new `tools/benchmark/perf-suite/`. Leave the existing MDL
   `tools/benchmark/compile.py` intact; reuse its `[*]`-line parsing idiom.

## What we measure

`slangc` is run with `-report-detailed-perf-benchmark`, which prints per-phase
timers as `[*] <phase> <count> <ms>`. We capture **all** timers, not just the
headline, so a regression can be attributed to a stage.

- **Headline metric:** `compileInner` — full compile (front-end + IR + codegen),
  excludes the fixed ~286 ms `loadBuiltinModule` cost, so it is stable and
  comparable across workloads and versions.
- **`loadBuiltinModule`** is recorded separately (it is per-process fixed cost
  and differs by release because each `slangc` bundles its own core module).
- **Stage timers** used to localize: `frontEndExecute`, `parseTranslationUnit`,
  `SemanticChecking`, `generateIR`, `generateOutput`, `linkAndOptimizeIR`,
  `linkIR`, `simplifyIR`, `specializeModule`, `unrollLoopsInModule`,
  inlining timers, serialized-module read timers.

### Methodology

- Each workload generator is **deterministic** and **scaled by a size knob N**,
  so the compile cost dominates measurement noise and we can dial difficulty.
- Per `(slangc, workload, target)`: **W warmup runs + K timed samples**
  (default W=1, K=5); report median + min + stdev of each phase timer.
- Avoid downstream tool dependence (e.g. `spirv-opt`); use
  `-target spirv -emit-spirv-directly` so we measure Slang itself. Targets that
  need an external toolchain are skipped on hosts that lack it.
- One tidy row per `(version, workload, target, phase)` → JSON + CSV.

## Feature / workload matrix (first cut)

Suspected-regression features (deepest workloads):

| Bucket | Workload (param N) | Primary signal |
|---|---|---|
| **autodiff** | many `[Differentiable]` fns, nested fwd/bwd calls, custom `bwd_diff`/`fwd_diff` derivatives, differentiable generics & arrays | `compileInner`, autodiff transform within `linkAndOptimizeIR` |
| **dynamic dispatch** | one interface, M implementations, existential/`dyn` params, witness-table-heavy call sites; also run with `-report-dynamic-dispatch-sites` | `specializeModule`, `linkIR` |
| **diagnostics** | source emitting many errors+warnings, plus a size-matched **clean control** to isolate diagnostic-path cost | `frontEndExecute`, `SemanticChecking` |

Core compiler-stage buckets:

| Bucket | Workload (param N) | Primary signal |
|---|---|---|
| lex/parse | very large but semantically trivial source | `parseTranslationUnit` |
| sema / generics / overload | deep generic instantiation, large overload sets | `SemanticChecking` |
| specialization | generic + interface specialization blowup | `specializeModule` |
| inlining / SSA | deep call graphs, large function bodies | `simplifyIR`, inlining |
| codegen targets | one shader → SPIR-V / DXIL / Metal / CPU (host-available only) | `generateOutput` |
| module precompile + link | multi-module separate compile then link | `linkIR`, module-read |

(Real-world MDL/DXR already covered by `compile.py`; a Falcor-representative
shader and reflection/layout + loop-unroll buckets are deferred to the expansion.)

## Phases

- **Phase 1 — design (this doc).** ✅
- **Phase 2 — build suite + harness.**
  - `perf-suite/workloads/` deterministic generators (one per bucket), each
    emitting `.slang` source scaled by N.
  - `perf-suite/bench.py`: cross-platform; runs workloads × targets, parses all
    phase timers, writes per-run JSON/CSV. Takes an explicit `slangc` path so it
    can drive any version.
  - `perf-suite/manifest.*`: the workload list + invocation flags + which timers
    are "primary" for each, so reporting is data-driven.
- **Phase 3 — release sweep.**
  - `perf-suite/fetch_releases.py`: download + cache prebuilt `slangc` for each
    tag in window (v2025.14 → v2026.9, ~monthly + patch).
  - Run full suite per release; aggregate into a long-format table.
  - Per-workload time-series + **step-change detector** flagging the release
    boundary where a phase timer jumps. Optional source `git bisect` within a
    flagged range to pin the commit.

## Open items / risks

- glibc variant: some tags ship `-glibc-2.27` builds; pick the variant that runs
  on the sweep host.
- Phase-timer **name drift** across 10 months of releases — the parser must
  tolerate added/removed/renamed timers and align on stable names
  (`compileInner` is present throughout the window; verify in Phase 3).
- Workloads must stay GPU-free and external-SDK-free to run in CI and headless.
