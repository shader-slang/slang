# Slang compile-time performance suite — origin & decisions

## Why it exists

A compile-time slowdown was observed building Slang for Falcor
(`nvresearch-gfx/Tools/falcor2`). The suite was built to reproduce that class of
problem in a controlled, attributable way: each workload stresses one compiler
stage, so a regression points at a specific pass; and the suite runs unchanged
against any release binary, so it also points at a specific release.

Suspected areas at the time of inception: **autodiff**, **diagnostics**,
**dynamic dispatch**. These became the "deepest workloads" (see README). The
sweep confirmed a 4× `autodiff` regression entering at v2026.7 (PR #9808) and a
separate `diagnostics_errors` step at v2025.15.

## Locked decisions

| Decision             | Choice                                                  | Rationale                                                                                                                                                                                                                                   |
| -------------------- | ------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Release binaries     | Prebuilt published `linux-x86_64` per tag               | Fast, reproducible, matches shipped artifacts; source builds only needed for commit-level bisect                                                                                                                                            |
| Measurement flag     | `-report-perf-benchmark`                                | Stable across the full v2025.12–v2026.10 window; the `detailed` variant adds sub-timers only on newer builds                                                                                                                                |
| Headline metric      | `compileInner` (median of 5 timed runs)                 | Excludes the fixed core-module load floor, stable across releases. Median over min: reflects the typical run, steadier when run-to-run spread shifts                                                                                        |
| Floor                | `minimal` workload's `compileInner`                     | The N→0 limit — direct measurement of fixed per-compile cost, not a fitted intercept which can go negative on convex curves                                                                                                                 |
| Timer scope          | All nested phase timers                                 | Attribution: a jump in `compileInner` is traced to `generateOutput → linkAndOptimizeIR → specializeModule`, etc., using leaf timers to avoid double-counting                                                                                |
| Phase decomposition  | Mutually-exclusive buckets (top-down budget allocation) | Named leaves + `(self)` residuals; children that overshoot a parent (timer non-additivity observed at v2026.7) are scaled proportionally so the sum always equals `compileInner`                                                            |
| Output               | `results.json` only (no CSV)                            | JSON stores all of median/min/mean/stdev per timer; CSV was unread. Generated sources + compiled outputs go to an auto-removed tempdir (`--gen-dir`) to keep the results dir, which is committed to the results repo, free of build scratch |
| GPU / SDK dependency | None                                                    | Every workload is GPU-free and external-SDK-free; runs headless in CI                                                                                                                                                                       |
| Target               | `-target spirv -emit-spirv-directly`                    | Measures Slang itself, not downstream `spirv-opt`                                                                                                                                                                                           |

## Storage layout

    slang-compile-perf/                (the perf results repo)
      index.json                       release manifest
      releases/<tag>/results.json      per-release sweep — source of truth
      daily/<date>-<sha>/results.json  nightly ToT sweep
      daily/<date>-<sha>/meta.json     {date, commit, runner, kind}
      runner.json                      runner fingerprint (all history must be on same machine)
      _tracking/tracking.json          derived series (trend.py / plots)

Excluded (transient or regenerable — enforced by `perf-results.gitignore`):
`releases/<tag>/gen/`, `_analysis/`, `_sweep/`, `_breakdown/`, `*.html`, `*.svg`.

## Workload expansion history

| Added                   | Workloads                                                                                                                                                                                                              |
| ----------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| inception               | `autodiff`, `dynamic_dispatch`, `diagnostics_errors/clean`, `parse`, `sema_generics`, `specialization`, `inlining`, `codegen_spirv`, `module_link`, `minimal`, `ir_builder`, `serialize`, `conformance`, `loop_unroll` |
| coverage-gap pass       | `resource_aggregate`, `reflection_layout`, `control_flow_ssa`                                                                                                                                                          |
| real-world              | `mdl_dxr` (MDL/DXR path-tracer corpus)                                                                                                                                                                                 |
| source-text backends    | `emit_metal`, `emit_wgsl`                                                                                                                                                                                              |
| realistic scaling       | `complexity_ladder`                                                                                                                                                                                                    |
| type-checking isolation | `operator_typecheck`, `implicit_conversion`, `overload_resolution`                                                                                                                                                     |

See `README.md` for the current workload table and `manifest.py` for the full spec.
