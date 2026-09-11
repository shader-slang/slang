# Compile-perf suite: coverage review and back-end gap analysis

Working document for [#12949](https://github.com/shader-slang/slang/issues/12949)
("Begin compile-time microbenchmark refactoring"), under
[#12941](https://github.com/shader-slang/slang/issues/12941).

Everything below was measured on one machine (macOS arm64, Apple Silicon,
Release `slangc`) against the release binaries named in each table. Absolute
milliseconds are therefore **not** comparable with the Windows perf-pool
series; the _ratios within a row_ are what the argument rests on, and every
one of them was reproduced on at least two independently built binaries.

---

## 1. Summary

The suite is deep on the front end and the target-independent IR pipeline, and
**one shader wide** on the back end.

- 41 workloads. **36 of them are SPIR-V, front-end-only, or API-path.**
- The six that are not (`emit_metal`, `emit_wgsl`, `emit_hlsl`, `emit_glsl`,
  `emit_cuda`, plus `codegen_dxil`/`codegen_ptx`) all compile **the same
  source** — `gen_codegen`, N lines of straight-line float math over a single
  `RWStructuredBuffer`.
- That source triggers essentially none of the target-specific legalization in
  `linkAndOptimizeIR`. Measured: the six targets land within **1.18x** of each
  other, and ~42% of each number is the identical front end.
- Consequence: a **~12x CUDA compile-time regression** that landed between
  v2026.2 and v2026.8.1 is still present in v2026.17 and was never visible to
  the suite (§4).
- A second blind spot is instrumentation, not workload choice: the default
  `-report-perf-benchmark` has **no timer for any back-end pass**, so all of it
  falls into the `linkAndOptimizeIR (self)` residual. `slangc` already supports
  `-report-detailed-perf-benchmark`, which names ~67 passes at **<= 1%**
  overhead. The suite was not using it (§5).

---

## 2. What the existing workloads cover

Categorized by what the workload actually exercises, not by what its name says.

| Category                             | Workloads                                                                                                                                                                                      | Target coverage                  |
| ------------------------------------ | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------- |
| Real-world corpus                    | `mdl_dxr`                                                                                                                                                                                      | SPIR-V only                      |
| Holistic scaling                     | `complexity_ladder`                                                                                                                                                                            | SPIR-V only                      |
| API / integration path               | `api_*` (6), `rt_renderer`, `rt_renderer_specialize`                                                                                                                                           | SPIR-V only                      |
| Per-compile floor                    | `minimal`                                                                                                                                                                                      | SPIR-V only                      |
| Front end (parse / sema / typecheck) | `parse`, `sema_generics`, `generic_nesting`, `generic_nesting_eval`, `interface_depth`, `conformance`, `diagnostics_clean`, `operator_typecheck`, `implicit_conversion`, `overload_resolution` | n/a (`module` mode)              |
| IR infrastructure                    | `ir_builder`, `serialize`, `module_link`                                                                                                                                                       | SPIR-V only                      |
| Target-independent IR passes         | `specialization`, `dynamic_dispatch`, `existential_aggregate`, `autodiff`, `inlining`, `loop_unroll`, `control_flow_ssa`, `resource_aggregate`, `reflection_layout`                            | SPIR-V only                      |
| Source emission                      | `emit_metal`, `emit_wgsl`, `emit_hlsl`, `emit_glsl`, `emit_cuda`                                                                                                                               | 5 targets, **one shared source** |
| Downstream toolchain                 | `codegen_dxil`, `codegen_ptx`                                                                                                                                                                  | win32 only, same shared source   |

**Construct coverage across every generated workload** (regex scan of all 30
generators at their default size; `mdl_dxr` is a fetched corpus and is listed
separately):

| Construct                          | # generated workloads using it | `mdl_dxr` |
| ---------------------------------- | -----------------------------: | :-------: |
| `ConstantBuffer` / `cbuffer`       |                              2 |    yes    |
| Texture / Sampler                  |                              4 |    yes    |
| matrix types                       |                              2 |    yes    |
| `RWTexture`                        |                              2 |    no     |
| ray-tracing stages                 |                              2 |    yes    |
| `ByteAddressBuffer`                |                              0 |    yes    |
| bitcast / reinterpret              |                              0 |    yes    |
| `half` / `float16_t`               |                              0 |    yes    |
| `double`                           |                              0 |    yes    |
| `ParameterBlock`                   |                          **0** |    no     |
| `groupshared`                      |                          **0** |    no     |
| atomics                            |                          **0** |    no     |
| raster stages (vertex / fragment)  |                          **0** |    no     |
| mesh / amplification stages        |                          **0** |    no     |
| wave intrinsics                    |                          **0** |    no     |
| `Optional<T>` / `Result<T,E>`      |                          **0** |    no     |
| `enum`                             |                          **0** |    no     |
| pointers (`Ptr<T>`)                |                          **0** |    no     |
| `Append`/`ConsumeStructuredBuffer` |                          **0** |    no     |
| bindless / `DescriptorHandle`      |                          **0** |    no     |
| cooperative vectors                |                          **0** |    no     |
| 64-bit integers                    |                          **0** |    no     |

`ParameterBlock` is worth calling out on its own: it is Falcor's primary
parameter-passing construct and the suite contains not one use of it.

Compile-option dimensions are covered even less: **every** workload runs at
default flags. No workload sets `-g`, `-O0`, `-O3`, a non-default matrix
layout, or a target profile/capability.

---

## 3. Why the `emit_*` family does not measure a back end

`gen_codegen` produces one compute entry point, one `RWStructuredBuffer`, and
N lines of scalar float math. It has no textures, no samplers, no matrices, no
constant buffers, no `groupshared`, no atomics, no varying parameters beyond
`SV_DispatchThreadID`, and no raster or ray-tracing stage. Almost every
target-specific pass in `linkAndOptimizeIR` is gated on a construct it does not
contain, so each one runs and immediately finds nothing to do.

`compileInner`, `gen_codegen` at N=400, one ToT build, median of 5:

| target                      | spirv | metal |  wgsl |  hlsl |  glsl |  cuda |
| --------------------------- | ----: | ----: | ----: | ----: | ----: | ----: |
| compileInner (ms)           |  95.2 | 106.4 | 105.9 | 100.5 | 104.8 | 112.3 |
| frontEndExecute (ms)        |  43.7 |  44.7 |  44.1 |  43.9 |  44.7 |  44.0 |
| linkAndOptimizeIR (ms)      |  35.6 |  38.0 |  35.0 |  32.2 |  35.9 |  43.1 |
| ... of which named children |  25.6 |  25.8 |  25.7 |  24.3 |  24.9 |  24.8 |

Total spread across six back ends: **1.18x**, with the front end — identical by
construction — accounting for ~42% of every number. The "back-end" dimension of
the suite is, in practice, a measurement of the emitter's print loop.

That is a legitimate thing to measure, and `emit_*` should keep measuring it.
The problem is that nothing else measures the rest of the back end. The README
currently claims `emit_metal`/`emit_wgsl` target "the source-emission backend
(`emitEntryPointsSourceFromIR` + target legalization)"; the legalization half of
that claim is not borne out and should be corrected.

---

## 4. The gap is not hypothetical: a live, unnoticed CUDA regression

Replace the flat math with **N resource loads in one entry point** — N
`Texture2D`s sampled once each, the shape any material system produces after
inlining — and the back ends stop agreeing.

`compileInner` (ms), median of 3 after warmup, same machine, N = 512:

| release    | spirv | metal |  hlsl |  glsl |   **cuda** | cuda / spirv |
| ---------- | ----: | ----: | ----: | ----: | ---------: | -----------: |
| v2026.2    | 180.0 | 217.7 | 108.2 | 108.8 |  **144.1** |        0.80x |
| v2026.8.1  | 283.1 | 368.8 | 237.8 | 217.9 | **2075.1** |         7.3x |
| v2026.16.1 |  81.2 | 190.6 |  72.1 |  72.5 | **1725.1** |        21.2x |
| v2026.17   |  78.1 | 176.8 |  69.0 |  67.7 | **1694.7** |        21.7x |
| ToT        | 102.3 | 221.8 |  93.8 |  94.3 | **1777.5** |        17.4x |

Every other target got 1.5-2.5x _faster_ over this window. CUDA went from 144 ms
to ~1700 ms and stayed there. Scaling on ToT is roughly cubic:

| N (loads in one function) |  128 |   256 |    512 |    1024 |
| ------------------------- | ---: | ----: | -----: | ------: |
| cuda (ms)                 | 71.6 | 135.0 | 1743.8 | 16179.1 |
| spirv (ms)                | 56.5 |  58.4 |   99.6 |   214.1 |

It tracks **load count**, not resource count: 64 textures sampled 8 times each
(512 loads) reproduces it at 8x SPIR-V.

`-report-detailed-perf-benchmark` attributes it in one run (N=512, cuda):

```
deferBufferLoad     950.5 ms
simplifyNonSSAIR    860.5 ms
specializeModule     39.3 ms
simplifyIR           28.7 ms
   ... 63 more passes, all under 1.5 ms
```

and a sampled stack pins the inner loop:

```
deferBufferLoad -> deferBufferLoadInFunc -> removeRedundancyInFunc
  -> eliminateRedundantLoadStore -> tryRemoveRedundantLoad
    -> canInstHaveSideEffectAtAddress -> doesCalleeHaveSideEffect
```

`deferBufferLoad` calls `removeRedundancyInFunc` once per function before doing
its own work, and that path has no callee-side-effect memo — the same class of
problem as the `simplifyIR` DCE memo added in
[#11954](https://github.com/shader-slang/slang/pull/11954), in a path that fix
did not reach. On CUDA the entry point's globals are moved into an explicit
global context, so each resource access becomes a `Load` and N grows with the
material; on SPIR-V they stay globals and the pass has almost nothing to chew.

`simplifyNonSSAIR` is the second half and is a _cross-target_ cost — it is the
same redundancy-removal machinery, run once more after phi elimination.

None of this was reachable from the suite, because no workload puts more than a
handful of loads in one function on a non-SPIR-V target.

---

## 5. The instrumentation gap

`-report-perf-benchmark` emits ~15 timers, and the only `linkAndOptimizeIR`
children among them are `specializeModule`, `simplifyIR`, `linkIR`,
`unrollLoopsInModule`, `legalizeResourceTypes`, `legalizeExistentialTypeLayout`
and the two inliners. Everything target-specific — `deferBufferLoad`,
`simplifyNonSSAIR`, `lowerCombinedTextureSamplers`, `legalizeIRForMetal`,
`legalizeIRForWGSL`, `legalizeEntryPointVaryingParamsForCUDA`,
`lowerBufferElementTypeToStorageType`, ~60 more — has no timer of its own and
lands in the `linkAndOptimizeIR (self)` residual.

It did not have to. Every pass in `linkAndOptimizeIR` goes through the
`SLANG_PASS` macro, which wraps it in `PassHooksRAII`; that already opens a
profiler scope when `CompilerOptionName::ReportDetailedPerfBenchmark` is set.
The CLI flag is `-report-detailed-perf-benchmark`, and it:

- is accepted by every release binary tested back to v2026.1.2;
- emits every timer the base flag does, with identical meaning, plus ~67
  per-pass sub-timers;
- attributes ~100% of `linkAndOptimizeIR`;
- costs **<= 1%** of `compileInner` — a 12-sample interleaved A/B measured
  1.010x on codegen/spirv and 1.001x on a resource-heavy CUDA compile, i.e.
  inside run-to-run noise.

`bench.py` was passing the base flag by choice, on the (then-correct) grounds
that the detailed flag was added mid-window. Since it turns out to be accepted
across the tested window, the fix is to prefer the detailed flag and fall back
to the base one per binary.

---

## 6. Other dimensions nothing measures

**Debug info.** `-g` costs **4.1x** on the real MDL shader (1067 ms -> 4347 ms
to SPIR-V), against only 1.43x on `gen_codegen` — another instance of the flat
synthetic shader under-representing a real cost. The time is in the bundled
SPIRV-Tools optimizer (`EliminateDeadFunctionsPass`), reached through
`emitSPIRVForEntryPointsDirectly`, and so it _is_ inside the suite's headline
`compileInner`. No workload sets `-g`.

**Optimization level.** `-O3` costs 3.3x on the same shader (1067 ms ->
3520 ms); `-O0` is roughly free. No workload sets either.

**Real shaders on non-SPIR-V back ends.** `mdl_dxr` is the suite's only real
shader and compiles to SPIR-V alone. It compiles to HLSL and GLSL today with an
explicit `-entry` (555 ms / 559 ms for `MdlRadianceClosestHitProgram`), which
would give real-shader D3D and GLSL coverage for the cost of two manifest
entries. Metal, WGSL and CUDA reject it — it uses ray-tracing stages.

**A warning about breadth.** A first attempt at closing the gap with one
"kitchen sink" cross-target shader (rich cbuffer + combined samplers + matrices

- image writes + groupshared, all scaled together) **failed**: at N=96-128 the
  cuda/spirv ratio sat at 1.07-1.13x on _every_ release, including the ones where
  the isolated load-chain probe shows 21x. Mixing dimensions let a shared
  super-linear front-end cost dominate and buried the back-end signal. The
  suite's existing one-axis-per-workload discipline is the right one for back
  ends too; this document's proposals follow it.

---

## 7. Proposed work

Ordered by value per unit of risk.

1. **Record detailed pass timers.** Prefer `-report-detailed-perf-benchmark`,
   per-binary fallback to `-report-perf-benchmark`. Turns
   `linkAndOptimizeIR (self)` from an opaque residual into ~67 named passes at
   no measurable cost. _(Implemented on `perf/backend-coverage`.)_
2. **Add isolated back-end stressors**, each one axis, each run on the targets
   where its pass actually fires, with SPIR-V as the control.
   _(Implemented: `backend*loads*_`, `backend*samplers*_`, `backend*matrix*_`
   — 12 entries, ~9 s added to a nightly.)\*
   - `resource_load_chain` - N resource loads in one entry point.
     Catches the CUDA `deferBufferLoad` quadratic; also the sharpest probe for
     `simplifyNonSSAIR`, which is cross-target.
   - `combined_samplers` - N `Sampler2D` reads. Fires
     `lowerCombinedTextureSamplers` on HLSL / Metal / WGSL; measured 2.70x
     spread at N=256, Metal exponent 1.18 against SPIR-V's 0.59.
   - `matrix_chain` - chained `mul` on `float4x4` from a
     `StructuredBuffer`. Measured 2.78x spread at N=256, CUDA exponent 1.32.
     \_(`image_subscript` — N `RWTexture2D` read-modify-writes, for
     `legalizeImageSubscript` — was prototyped and **dropped**. A careful
     sweep put its spread at 1.28x at N=512, barely better than the 1.18x of the
     `emit\__` family it was meant to improve on. Shipping it would have added a
     workload that does not do its stated job.)\*
3. **Give the existing single-target workloads a second target** where it is
   nearly free. `resource_aggregate` turned out to be the single best find
   here: `legalizeResourceTypes` takes a `TargetProgram` and diverges **3.83x**
   at N=160 (exponents cuda 1.74 / glsl 1.47 / metal 1.38 against SPIR-V's
   0.67) — the widest back-end divergence measured anywhere, from a workload
   that was already in the suite and only ever run on the target where its
   pass is cheapest. _(Implemented: `resource_aggregate_{metal,glsl,cuda}`.)_
`reflection_layout`was checked the same way and does **not** justify extra
targets — 1.40x spread, exponents within 0.2 of each other.`mdl_dxr`compiles to HLSL and GLSL today with an explicit`-entry` and is still worth
   adding.
4. **Add the option dimensions**: a `-g` variant and an `-O3` variant of
   `mdl_dxr`, which is where they show their real cost.
5. **Fill the construct gaps**, in rough order of how much real code uses them:
   `ParameterBlock`, raster-stage varying parameters, `groupshared` + atomics,
   `ByteAddressBuffer`, bindless/`DescriptorHandle`, wave intrinsics, mesh
   shaders.
6. **Teach `lib/buckets.py` about the detailed timers** so the new bands reach
   the stacked-area charts. Note that the child relation no longer has to be
   hardcoded for these: every `SLANG_PASS` timer is by construction a direct
   child of `linkAndOptimizeIR`, so the residual can be tiled automatically —
   which also answers the standing criticism that the bucket tree is
   hand-maintained Python.

Compiler-side follow-ups this analysis produced, to be filed separately:

- `deferBufferLoad` -> `removeRedundancyInFunc` has no callee-side-effect memo
  (the CUDA regression above).
- `simplifyNonSSAIR` and the direct-SPIR-V emit path
  (`emitSPIRVForEntryPointsDirectly`) have no `SLANG_PROFILE`, so they are
  invisible even with the detailed flag off... and the SPIR-V emitter is
  invisible with it on.
