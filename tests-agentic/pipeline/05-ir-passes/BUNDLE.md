---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-20T18:00:00+00:00
source_commit: 1e0d460c1cb410005c4f775ba11fbc803cc8c16d
watched_paths_digest: 8749b5a60327ef9aea96c0b02a10d643c2d39d04195e7cbd40904b69dabc7f6e
source_doc: docs/llm-generated/pipeline/05-ir-passes.md
source_doc_digest: 0c1c128f131512193c797738ba0f341f117afd9505bb5ddddf47481f9f70dc7b
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for pipeline/05-ir-passes

## Intent

Tests verify the IR-pass catalog described in
[`docs/llm-generated/pipeline/05-ir-passes.md`](../../../docs/llm-generated/pipeline/05-ir-passes.md):
the categorized inventory of ~325 `slang-ir-*.cpp` passes that run
between AST → IR lowering and code emission, orchestrated by
`linkAndOptimizeIR` in `slang-emit.cpp`. The doc groups the passes
into nine categories (Linking and validation, SSA construction /
basic cleanup, Specialization and generics, Differentiation,
Type and value legalization, Inlining and call-graph, Entry-point
and parameter handling, Layout and binding, Loop transformations,
Target-specific lowering, Instrumentation). Each test anchors on a
named pass in the doc's per-category tables and confirms an
observable consequence — either a `### BEFORE/AFTER <pass>:` IR-dump
diff, a target-text rewrite that the pass causes, or a diagnostic
the validation pass emits.

The bundle exercises every category. Three modes of observation are
used:

- **`-dump-ir-before/-dump-ir-after <pass>`** with FileCheck against
  the IR dump for `### BEFORE <pass>:` / `### AFTER <pass>:` headers,
  used for `specializeModule`, `simplifyIR`, `eliminatePhis`,
  `eliminateDeadCode` (and as the IR-side mode for `dll-export`).
- **Multi-target SIMPLE** with one `//TEST:SIMPLE(filecheck=<NAME>)`
  per text-emit target where the pass is observable; per-target
  CHECK prefixes in the source comments. Used for the dominant
  legalization / emit-shape consequences (DCE, inline, optional
  lowering, collect-global-uniforms, entry-point decorations,
  byte-address legalize, vector-types legalize, etc.).
- **`DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive)`** for the
  validation passes that emit diagnostics: `check-recursion`,
  `missing-return`, `check-optional-none-usage`, `late-require-
  capability` (warning form), `check-differentiability`,
  `operator-shift-overflow`.

This is the largest bundle in the suite by design: `size_cap_files`
is 100; the bundle has 71 tests covering most categories. The
initial 45 functional + negative tests have been extended with 26
boundary / negative / stress probes that pressure the same claims
along documented axes (0 / 1 / many / large-N iterations for
loop-unroll; 0 / 1 / many uniforms for layout-and-binding; narrow
scalar types — uint8, half, bool — through type-legalization;
alternative input shapes for the diagnostic passes).
Differentiation tests are limited to one observable diagnostic
(`check-differentiability`) — the rest of the autodiff family
requires test infrastructure beyond what `slangc` exposes on a
no-GPU runner and is recorded under `## Out of scope`.

## Claims enumerated

| Claim ID | Anchor                                                                                                                                                  | Claim (one line)                                                                                                                  | Tests                                                                  |
| -------- | ------------------------------------------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------- |
| C-01     | [#linking-and-validation](../../../docs/llm-generated/pipeline/05-ir-passes.md#linking-and-validation)                                                  | The check-recursion pass diagnoses unsupported recursion in the IR.                                                               | `recursion-check-rejects-self-recursion.slang`                         |
| C-02     | [#ssa-construction-and-basic-cleanup](../../../docs/llm-generated/pipeline/05-ir-passes.md#ssa-construction-and-basic-cleanup)                          | The missing-return pass diagnoses paths missing a return in a non-void function.                                                  | `missing-return-warns-on-non-void.slang`                               |
| C-03     | [#linking-and-validation](../../../docs/llm-generated/pipeline/05-ir-passes.md#linking-and-validation)                                                  | The check-optional-none-usage pass rejects an `Optional.value` read on an always-`none` value.                                    | `optional-none-usage-check-rejects-always-none.slang`                  |
| C-04     | [#ssa-construction-and-basic-cleanup](../../../docs/llm-generated/pipeline/05-ir-passes.md#ssa-construction-and-basic-cleanup)                          | The DCE pass removes a top-level helper function that is never called.                                                            | `dce-removes-unused-helper-from-emit.slang`                            |
| C-05     | [#specialization-and-generics](../../../docs/llm-generated/pipeline/05-ir-passes.md#specialization-and-generics)                                        | The specialize pass substitutes generic parameters with concrete types.                                                           | `specialize-module-substitutes-generic-with-concrete-type.slang`       |
| C-06     | [#type-and-value-legalization](../../../docs/llm-generated/pipeline/05-ir-passes.md#type-and-value-legalization)                                        | The lower-optional-type pass turns `Optional<T>` into a struct with `value` and `hasValue` fields.                                | `lower-optional-type-emits-struct-with-has-value.slang`                |
| C-07     | [#inlining-and-call-graph](../../../docs/llm-generated/pipeline/05-ir-passes.md#inlining-and-call-graph)                                                | The inline pass inlines `[ForceInline]` callees so the callee name no longer appears in the emit.                                 | `inline-removes-forceinline-callee-from-emit.slang`                    |
| C-08     | [#layout-and-binding](../../../docs/llm-generated/pipeline/05-ir-passes.md#layout-and-binding)                                                          | The collect-global-uniforms pass packages module-scope uniforms into a `GlobalParams` aggregate.                                  | `collect-global-uniforms-builds-global-params-struct.slang`            |
| C-09     | [#entry-point-and-parameter-handling](../../../docs/llm-generated/pipeline/05-ir-passes.md#entry-point-and-parameter-handling)                          | The entry-point-decorations pass surfaces `[numthreads(...)]` as the per-target entry-point marker.                               | `entry-point-decorations-emit-numthreads-marker.slang`                 |
| C-10     | [#entry-point-and-parameter-handling](../../../docs/llm-generated/pipeline/05-ir-passes.md#entry-point-and-parameter-handling)                          | The entry-point-uniforms + collect-global-uniforms passes pack multiple uniforms into the same emit-stage aggregate.              | `entry-point-uniforms-packs-multiple-uniforms-into-one-cbuffer.slang`  |
| C-11     | [#target-specific-lowering](../../../docs/llm-generated/pipeline/05-ir-passes.md#target-specific-lowering)                                              | The GLSL-legalize pass produces a `layout(std430, binding=N) buffer { T _data[]; }` SSBO shape for `RWStructuredBuffer<T>`.       | `glsl-legalize-emits-std430-ssbo-shape.slang`                          |
| C-12     | [#target-specific-lowering](../../../docs/llm-generated/pipeline/05-ir-passes.md#target-specific-lowering)                                              | The SPIR-V legalize pass produces the expected SPIR-V preamble: `OpCapability Shader`, `OpEntryPoint`, `OpExecutionMode LocalSize`. | `spirv-legalize-emits-opcapability-shader.slang`                       |
| C-13     | [#target-specific-lowering](../../../docs/llm-generated/pipeline/05-ir-passes.md#target-specific-lowering)                                              | The Metal-legalize pass attaches positional `[[buffer(N)]]` markers and a `kernel` qualifier to the entry point.                  | `metal-legalize-emits-positional-buffer-marker.slang`                  |
| C-14     | [#target-specific-lowering](../../../docs/llm-generated/pipeline/05-ir-passes.md#target-specific-lowering)                                              | The WGSL-legalize pass attaches `@binding(N) @group(N)` and `@compute @workgroup_size(...)`.                                      | `wgsl-legalize-emits-binding-group-and-workgroup-size.slang`           |
| C-15     | [#target-specific-lowering](../../../docs/llm-generated/pipeline/05-ir-passes.md#target-specific-lowering)                                              | The CUDA-immutable-load pass wraps reads of uniform globals in `__ldg(&...)` on CUDA.                                             | `cuda-immutable-load-wraps-uniform-read-in-ldg.slang`                  |
| C-16     | [#type-and-value-legalization](../../../docs/llm-generated/pipeline/05-ir-passes.md#type-and-value-legalization)                                        | The byte-address-legalize pass turns `ByteAddressBuffer.Load<T>(addr)` into a typed `_data[...]` access on GLSL.                  | `byte-address-legalize-lowers-load-on-glsl.slang`                      |
| C-17     | [#layout-and-binding](../../../docs/llm-generated/pipeline/05-ir-passes.md#layout-and-binding)                                                          | The translate-global-varying-var pass routes `SV_DispatchThreadID` into the per-target stage-input marker.                        | `translate-global-varying-var-emits-input-as-parameter.slang`          |
| C-18     | [#loop-transformations](../../../docs/llm-generated/pipeline/05-ir-passes.md#loop-transformations)                                                      | The loop-unroll pass unrolls a `[ForceUnroll]` loop with a compile-time bound on GLSL.                                            | `loop-unroll-removes-static-loop-on-glsl.slang`                        |
| C-19     | [#type-and-value-legalization](../../../docs/llm-generated/pipeline/05-ir-passes.md#type-and-value-legalization)                                        | The lower-bit-cast pass turns `bit_cast<int>(f)` into per-target intrinsics (`asuint` on HLSL, `floatBitsToInt` on GLSL).          | `lower-bit-cast-emits-target-specific-reinterpret.slang`               |
| C-20     | [#entry-point-and-parameter-handling](../../../docs/llm-generated/pipeline/05-ir-passes.md#entry-point-and-parameter-handling)                          | The lower-out-parameters pass preserves the `out` keyword on HLSL and GLSL parameters.                                            | `lower-out-parameters-preserves-out-keyword.slang`                     |
| C-21     | [#specialization-and-generics](../../../docs/llm-generated/pipeline/05-ir-passes.md#specialization-and-generics)                                        | The specialize-target-switch pass selects the branch matching the active target.                                                  | `specialize-target-switch-resolves-target-conditional.slang`           |
| C-22     | [#layout-and-binding](../../../docs/llm-generated/pipeline/05-ir-passes.md#layout-and-binding)                                                          | The string-hash pass replaces `getStringHash("...")` with the precomputed integer hash.                                           | `string-hash-pass-emits-numeric-hash.slang`                            |
| C-23     | [#type-and-value-legalization](../../../docs/llm-generated/pipeline/05-ir-passes.md#type-and-value-legalization)                                        | The lower-defer pass routes the body of `defer { ... }` to run at scope exit.                                                     | `lower-defer-emits-cleanup-on-scope-exit.slang`                        |
| C-24     | [#type-and-value-legalization](../../../docs/llm-generated/pipeline/05-ir-passes.md#type-and-value-legalization)                                        | The value-legalization passes route `int(fval)` cast to per-target cast intrinsics.                                               | `lower-l-value-cast-routes-through-pointer.slang`                      |
| C-25     | [#ssa-construction-and-basic-cleanup](../../../docs/llm-generated/pipeline/05-ir-passes.md#ssa-construction-and-basic-cleanup)                          | The init-local-var pass allows reading a struct local that was empty-initialized.                                                 | `init-local-var-allows-empty-initializer.slang`                        |
| C-26     | [#layout-and-binding](../../../docs/llm-generated/pipeline/05-ir-passes.md#layout-and-binding)                                                          | The layout pass adds `OpDecorate ... Binding` and `OpDecorate ... DescriptorSet` for resources on SPIR-V.                         | `layout-pass-emits-binding-and-descriptorset-decorations.slang`        |
| C-27     | [#target-specific-lowering](../../../docs/llm-generated/pipeline/05-ir-passes.md#target-specific-lowering)                                              | The Vulkan-invert-Y pass negates the Y component of vertex-shader output position when `-fvk-invert-y` is set.                    | `vk-invert-y-flips-position-y-on-spirv.slang`                          |
| C-28     | [#type-and-value-legalization](../../../docs/llm-generated/pipeline/05-ir-passes.md#type-and-value-legalization)                                        | The lower-buffer-element-type pass renders an `RWStructuredBuffer<int>` element as `int _data[]` on GLSL.                         | `lower-buffer-element-type-renders-int-on-glsl.slang`                  |
| C-29     | [#differentiation-autodiff](../../../docs/llm-generated/pipeline/05-ir-passes.md#differentiation-autodiff)                                              | The check-differentiability pass diagnoses calling a non-`[Differentiable]` function from a differentiable context.               | `check-differentiability-rejects-non-differentiable.slang`             |
| C-30     | [#ssa-construction-and-basic-cleanup](../../../docs/llm-generated/pipeline/05-ir-passes.md#ssa-construction-and-basic-cleanup)                          | The operator-shift-overflow pass warns when a shift count exceeds the operand bit width.                                          | `operator-shift-overflow-warns-on-large-shift.slang`                   |
| C-31     | [#specialization-and-generics](../../../docs/llm-generated/pipeline/05-ir-passes.md#specialization-and-generics)                                        | The specialize-arrays pass resolves a fixed-size-array generic parameter — the helper inlines away.                               | `specialize-arrays-allows-generic-array-parameter.slang`               |
| C-32     | [#layout-and-binding](../../../docs/llm-generated/pipeline/05-ir-passes.md#layout-and-binding)                                                          | The late-require-capability pass propagates capability requirements (e.g. `GL_EXT_debug_printf` for `printf`).                    | `late-require-capability-emits-extension-for-printf.slang`             |
| C-33     | [#type-and-value-legalization](../../../docs/llm-generated/pipeline/05-ir-passes.md#type-and-value-legalization)                                        | The lower-tuple-types pass decomposes `Tuple<...>` so no `Tuple` type name remains in the emit.                                   | `lower-tuple-types-flattens-into-anonymous-values.slang`               |
| C-34     | [#type-and-value-legalization](../../../docs/llm-generated/pipeline/05-ir-passes.md#type-and-value-legalization)                                        | The legalize-vector-types pass renders Slang vectors as `int3` on HLSL and `ivec3` on GLSL.                                       | `legalize-vector-types-emits-target-vector.slang`                      |
| C-35     | [#type-and-value-legalization](../../../docs/llm-generated/pipeline/05-ir-passes.md#type-and-value-legalization)                                        | The legalize-binary-operator pass casts the RHS of a shift to `u32` on WGSL.                                                      | `legalize-binary-operator-routes-int-shift-on-wgsl.slang`              |
| C-36     | [#ssa-construction-and-basic-cleanup](../../../docs/llm-generated/pipeline/05-ir-passes.md#ssa-construction-and-basic-cleanup)                          | The eliminate-phis pass converts SSA back to non-SSA form at the very end of the pipeline.                                        | `eliminate-phis-converts-out-of-ssa-form.slang`                        |
| C-37     | [#ssa-construction-and-basic-cleanup](../../../docs/llm-generated/pipeline/05-ir-passes.md#ssa-construction-and-basic-cleanup)                          | The strip-debug-info pass removes `OpLine` instructions when `-g` is not requested.                                               | `strip-debug-info-keeps-zero-debug-instructions.slang`                 |
| C-38     | [#specialization-and-generics](../../../docs/llm-generated/pipeline/05-ir-passes.md#specialization-and-generics)                                        | The lower-dynamic-dispatch-insts / bind-existentials passes resolve a known-concrete interface receiver to a direct call.         | `lower-com-methods-emit-binds-virtual-call.slang`                      |
| C-39     | [#ssa-construction-and-basic-cleanup](../../../docs/llm-generated/pipeline/05-ir-passes.md#ssa-construction-and-basic-cleanup)                          | The `simplifyIR` pass runs multiple times and preserves the user-named entry-point function.                                      | `simplify-ir-preserves-entry-point.slang`                              |
| C-40     | [#layout-and-binding](../../../docs/llm-generated/pipeline/05-ir-passes.md#layout-and-binding)                                                          | The explicit-global-context pass threads `GlobalParams_0` through the C++ emit for global-uniform reads.                          | `explicit-global-context-passes-globals-to-cpu.slang`                  |
| C-41     | [#inlining-and-call-graph](../../../docs/llm-generated/pipeline/05-ir-passes.md#inlining-and-call-graph)                                                | The dll-export pass attaches an `[export("...")]` decoration to entry-point IR functions.                                         | `dll-export-marks-entry-point-with-export-decoration.slang`            |
| C-42     | [#how-the-passes-are-ordered](../../../docs/llm-generated/pipeline/05-ir-passes.md#how-the-passes-are-ordered)                                          | Multiple invocations of `eliminateDeadCode` are visible in the IR dump (the orchestrator interleaves cleanup).                    | `eliminate-dead-code-runs-multiple-stages.slang`                       |
| C-43     | [#type-and-value-legalization](../../../docs/llm-generated/pipeline/05-ir-passes.md#type-and-value-legalization)                                        | The lower-enum-type pass renders a Slang enum constant as a plain integer in the HLSL emit.                                       | `lower-enum-type-emits-integer-on-hlsl.slang`                          |
| C-44     | [#specialization-and-generics](../../../docs/llm-generated/pipeline/05-ir-passes.md#specialization-and-generics)                                        | The specialize-stage-switch pass selects the branch matching the active shader stage.                                             | `specialize-stage-switch-resolves-stage-conditional.slang`             |
| C-45     | [#linking-and-validation](../../../docs/llm-generated/pipeline/05-ir-passes.md#linking-and-validation)                                                  | Using a feature beyond the requested profile triggers a `profile implicitly upgraded` diagnostic.                                 | `check-unsupported-inst-rejects-non-target-feature.slang`              |

## Tests in this bundle

| File                                                                  | Intent     | Doc anchor                            |
| --------------------------------------------------------------------- | ---------- | ------------------------------------- |
| `byte-address-legalize-lowers-load-on-glsl.slang`                     | functional | `#type-and-value-legalization`        |
| `check-differentiability-rejects-non-differentiable.slang`            | negative   | `#differentiation-autodiff`           |
| `check-unsupported-inst-rejects-non-target-feature.slang`             | negative   | `#linking-and-validation`             |
| `collect-global-uniforms-builds-global-params-struct.slang`           | functional | `#layout-and-binding`                 |
| `cuda-immutable-load-only-on-cuda-not-hlsl.slang`                     | boundary   | `#target-specific-lowering`           |
| `cuda-immutable-load-wraps-uniform-read-in-ldg.slang`                 | functional | `#target-specific-lowering`           |
| `dce-keeps-partially-used-struct-field.slang`                         | boundary   | `#ssa-construction-and-basic-cleanup` |
| `dce-removes-dead-branch-after-const-fold.slang`                      | boundary   | `#ssa-construction-and-basic-cleanup` |
| `dce-removes-transitively-dead-call-chain.slang`                      | boundary   | `#ssa-construction-and-basic-cleanup` |
| `dce-removes-unused-helper-from-emit.slang`                           | functional | `#ssa-construction-and-basic-cleanup` |
| `dce-runs-multiple-times-stress.slang`                                | stress     | `#how-the-passes-are-ordered`         |
| `dll-export-marks-entry-point-with-export-decoration.slang`           | functional | `#inlining-and-call-graph`            |
| `eliminate-dead-code-runs-multiple-stages.slang`                      | functional | `#how-the-passes-are-ordered`         |
| `eliminate-phis-converts-out-of-ssa-form.slang`                       | functional | `#ssa-construction-and-basic-cleanup` |
| `entry-point-decorations-emit-numthreads-marker.slang`                | functional | `#entry-point-and-parameter-handling` |
| `entry-point-decorations-spirv-localsize.slang`                       | boundary   | `#entry-point-and-parameter-handling` |
| `entry-point-uniforms-packs-multiple-uniforms-into-one-cbuffer.slang` | functional | `#entry-point-and-parameter-handling` |
| `explicit-global-context-passes-globals-to-cpu.slang`                 | functional | `#layout-and-binding`                 |
| `glsl-legalize-emits-std430-ssbo-shape.slang`                         | functional | `#target-specific-lowering`           |
| `init-local-var-allows-empty-initializer.slang`                       | functional | `#ssa-construction-and-basic-cleanup` |
| `init-local-var-on-trivial-empty-struct.slang`                        | boundary   | `#ssa-construction-and-basic-cleanup` |
| `inline-many-call-sites-stress.slang`                                 | stress     | `#inlining-and-call-graph`            |
| `inline-removes-forceinline-callee-from-emit.slang`                   | functional | `#inlining-and-call-graph`            |
| `inline-tiny-helper-survives-without-attribute.slang`                 | boundary   | `#inlining-and-call-graph`            |
| `late-require-capability-emits-extension-for-printf.slang`            | functional | `#layout-and-binding`                 |
| `layout-many-uniforms-stress.slang`                                   | stress     | `#layout-and-binding`                 |
| `layout-pass-emits-binding-and-descriptorset-decorations.slang`       | functional | `#layout-and-binding`                 |
| `layout-zero-uniforms-no-globalparams.slang`                          | boundary   | `#layout-and-binding`                 |
| `legalize-binary-operator-routes-int-shift-on-wgsl.slang`             | functional | `#type-and-value-legalization`        |
| `legalize-types-bool-in-struct.slang`                                 | boundary   | `#type-and-value-legalization`        |
| `legalize-types-half-16bit-through-pipeline.slang`                    | boundary   | `#type-and-value-legalization`        |
| `legalize-types-uint8-narrow-through-pipeline.slang`                  | boundary   | `#type-and-value-legalization`        |
| `legalize-vector-types-emits-target-vector.slang`                     | functional | `#type-and-value-legalization`        |
| `legalize-vector-types-length-one-vector.slang`                       | boundary   | `#type-and-value-legalization`        |
| `loop-unroll-large-iteration-stress.slang`                            | stress     | `#loop-transformations`               |
| `loop-unroll-removes-static-loop-on-glsl.slang`                       | functional | `#loop-transformations`               |
| `loop-unroll-single-iteration.slang`                                  | boundary   | `#loop-transformations`               |
| `loop-unroll-zero-iterations.slang`                                   | boundary   | `#loop-transformations`               |
| `lower-bit-cast-emits-target-specific-reinterpret.slang`              | functional | `#type-and-value-legalization`        |
| `lower-buffer-element-type-renders-int-on-glsl.slang`                 | functional | `#type-and-value-legalization`        |
| `lower-com-methods-emit-binds-virtual-call.slang`                     | functional | `#specialization-and-generics`        |
| `lower-defer-emits-cleanup-on-scope-exit.slang`                       | functional | `#type-and-value-legalization`        |
| `lower-enum-type-emits-integer-on-hlsl.slang`                         | functional | `#type-and-value-legalization`        |
| `lower-l-value-cast-routes-through-pointer.slang`                     | functional | `#type-and-value-legalization`        |
| `lower-optional-type-emits-struct-with-has-value.slang`               | functional | `#type-and-value-legalization`        |
| `lower-out-parameters-preserves-out-keyword.slang`                    | functional | `#type-and-value-legalization`        |
| `lower-tuple-types-flattens-into-anonymous-values.slang`              | functional | `#type-and-value-legalization`        |
| `metal-legalize-emits-positional-buffer-marker.slang`                 | functional | `#target-specific-lowering`           |
| `missing-return-on-empty-non-void-body.slang`                         | negative   | `#ssa-construction-and-basic-cleanup` |
| `missing-return-warns-on-non-void.slang`                              | negative   | `#ssa-construction-and-basic-cleanup` |
| `operator-shift-overflow-warns-on-large-shift.slang`                  | negative   | `#ssa-construction-and-basic-cleanup` |
| `optional-none-usage-check-rejects-always-none.slang`                 | negative   | `#linking-and-validation`             |
| `optional-none-usage-on-explicit-none-literal.slang`                  | negative   | `#linking-and-validation`             |
| `recursion-check-rejects-mutual-recursion.slang`                      | negative   | `#linking-and-validation`             |
| `recursion-check-rejects-self-recursion.slang`                        | negative   | `#linking-and-validation`             |
| `shift-overflow-on-uint-32-shift.slang`                               | negative   | `#ssa-construction-and-basic-cleanup` |
| `simplify-ir-preserves-entry-point.slang`                             | functional | `#ssa-construction-and-basic-cleanup` |
| `specialize-arrays-allows-generic-array-parameter.slang`              | functional | `#specialization-and-generics`        |
| `specialize-many-different-arg-instances.slang`                       | stress     | `#specialization-and-generics`        |
| `specialize-module-substitutes-generic-with-concrete-type.slang`      | functional | `#specialization-and-generics`        |
| `specialize-one-instance-emits-concrete-type.slang`                   | boundary   | `#specialization-and-generics`        |
| `specialize-stage-switch-resolves-stage-conditional.slang`            | functional | `#specialization-and-generics`        |
| `specialize-target-switch-resolves-target-conditional.slang`          | functional | `#specialization-and-generics`        |
| `specialize-zero-instances-of-generic.slang`                          | boundary   | `#specialization-and-generics`        |
| `spirv-legalize-emits-opcapability-shader.slang`                      | functional | `#target-specific-lowering`           |
| `string-hash-pass-deterministic-across-runs.slang`                    | boundary   | `#layout-and-binding`                 |
| `string-hash-pass-emits-numeric-hash.slang`                           | functional | `#layout-and-binding`                 |
| `strip-debug-info-keeps-zero-debug-instructions.slang`                | functional | `#ssa-construction-and-basic-cleanup` |
| `translate-global-varying-var-emits-input-as-parameter.slang`         | functional | `#layout-and-binding`                 |
| `vk-invert-y-flips-position-y-on-spirv.slang`                         | functional | `#target-specific-lowering`           |
| `wgsl-legalize-emits-binding-group-and-workgroup-size.slang`          | functional | `#target-specific-lowering`           |

## Doc gaps observed

- The "Differentiation (autodiff)" category lists nine passes
  (`slang-ir-autodiff*.cpp`) but does not enumerate user-observable
  consequences per pass. Only `check-differentiability` has an
  observable surface here (a diagnostic). The other autodiff
  passes (`fwd`, `rev`, `transpose`, `unzip`, `cfg-norm`,
  `loop-analysis`, `pairs`, `primal-hoist`, `region`) would need
  per-pass "what the user sees" rows to be testable from a `.slang`
  file. A pointer to the `[Differentiable]` user surface alongside
  each row would unlock per-pass tests.
- The "Inlining and call-graph" category lists `call-graph`,
  `reachability`, `propagate-func-properties`, `marshal-native-call`,
  `defer-buffer-load` — none of which name a user-observable
  consequence. They are analysis or marshalling helpers that this
  bundle could not anchor cleanly. A "footprint in emit text" row
  per pass would enable tests.
- The "Layout and binding" category includes `user-type-hint`,
  `metadata`, `explicit-global-init` — the doc states the pass
  exists and what it does internally but not how to spot its
  output. `explicit-global-context` was the only one we could
  anchor cleanly (CUDA / C++ pointer threading).
- The "Loop transformations" category lists six passes
  (`loop-unroll`, `loop-inversion`, `fuse-satcoop`, `restructure`,
  `restructure-scoping`, `synthesize-active-mask`, `uniformity`).
  Only `loop-unroll` is observably testable; the rest would need
  per-pass "what to look for in the dump or emit" prose.
- The "Instrumentation" section names `coverage-instrument` /
  `finalize-coverage-metadata` as gated on `-trace-coverage-binding`
  and `-trace-coverage-reserved-space` — those flags' surface is
  not exercised here (recorded under `## Out of scope`). A
  default-on instrumentation pass without flag dependence would
  be testable; the doc could add such an example.
- The "Linking and validation" category lists `slang-ir-validate.cpp`
  as "structural sanity checks". The doc names no user-observable
  symptom of validation failing; we could not produce a clean
  negative test for the validator itself. A short example of a
  malformed IR construct that validation rejects would help.
- The doc cites the `linkAndOptimizeIR` orchestration but says
  "the pipeline is **not** a fixed list" — the only observable
  ordering claim we could anchor was "DCE runs multiple times" (a
  weak claim that the orchestrator interleaves cleanup). Any
  pass-ordering invariant that the doc affirms (e.g. "specialize
  always runs before inline" or "validate runs first and last")
  would unlock ordering-based tests, but no such invariant is
  currently stated.
- "Target-specific lowering" has `slang-ir-translate.cpp` listed
  as "Generic translation step used by some targets" — too vague
  to anchor a test. Naming the specific surface (e.g. "Translate
  routes `printf` to OptiX-shaped trace calls") would help.

## Out of scope (no-GPU runner)

(This heading is used here for "claims not observable through any
allowed `slang-test` directive", consistent with the cross-cutting
bundles. The doc is overwhelmingly about IR-internal pass
behavior; most claims are observable via `-dump-ir` or per-target
emit text. The items below are genuinely outside that
observability.)

- The exact order in which passes run for a given target. The doc
  explicitly defers ordering to `linkAndOptimizeIR` and the
  per-target pipelines under `target-pipelines/`. A textual
  ordering would be brittle and is not a doc claim here; only the
  "multiple invocations" claim is preserved.
- The complete sequence of `### AFTER <pass>:` headers — these
  vary by target (HLSL has ~64 stages, SPIR-V has ~86) and would
  be a snapshot test, not a claim test.
- Per-pass C++ helper functions and class structures (IRBuilder
  usage inside each pass). Internal API.
- Hoistable / global flag bits — internal to `IRInst`'s op
  encoding; not in the `-dump-ir` output.
- IR pass utilities (`Clone`, `Dominators`, `Util`, `Insts info`,
  `Insts stable names`) — they are not transformations and have
  no observable effect of their own.
- The pre-link region documented in `04b-pre-link-passes.md`. If
  a claim is about an explicitly pre-link pass, it belongs in
  that bundle's prompt; tests here are anchored at the post-link
  orchestrator only.
- Coverage instrumentation (`-trace-coverage-binding`,
  `-trace-coverage-reserved-space`) — these are command-line
  surface; the agentic bundle does not exercise them.
- Differentiation passes (`autodiff-fwd`, `-rev`, `-transpose`,
  `-unzip`, `-cfg-norm`, `-loop-analysis`, `-pairs`,
  `-primal-hoist`, `-region`). The doc names the pass family but
  does not specify a per-pass user-observable consequence at a
  level this bundle can anchor cleanly without writing a wider
  autodiff test fixture. Recorded as a doc gap above.
- The `obfuscate-loc` pass — gated on `-obfuscate` and intended
  for distributed modules; not exercised here.
- The `insert-debug-value-store` and `liveness` passes — gated
  on debug-info preservation. We exercised the inverse
  (`strip-debug-info` without `-g`); the positive `-g` path
  would require a debug emitter we do not target.
- The `defunctionalization` pass and `lower-expand-type` pass —
  the source-level constructs (first-class function values,
  variadic packs) are not core enough to anchor a `.slang` test
  without inviting a wider language-feature test.
- `extract-value-from-type`, `bind-existentials`,
  `any-value-inference`, `any-value-marshalling`,
  `synthesize-active-mask`, `propagate-func-properties` — internal
  analysis or marshalling passes with no user-observable footprint
  that the doc names.
- The "Other passes" section's `spirv-snippet` — internal helper
  used by SPIR-V emit; not surface-visible.
- The "Pass utilities" section — `Clone`, `Dominators`, `Util`,
  `Insts info`, `Insts stable names` — these are not
  transformations and have no observable behavior on their own.
- The "Adding a new pass" workflow — developer guide, not a
  user-observable behavior.
