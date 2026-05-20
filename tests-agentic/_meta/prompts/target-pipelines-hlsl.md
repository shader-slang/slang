# Prompt: tests-agentic/target-pipelines/hlsl/

See [`_common.md`](_common.md) for universal rules. Those rules apply
to this bundle and override nothing here unless explicitly noted.

## Target

Produce the test bundle at `tests-agentic/target-pipelines/hlsl/`,
anchored to
[`docs/llm-generated/target-pipelines/hlsl.md`](../../../docs/llm-generated/target-pipelines/hlsl.md).

Audience: nightly CI. The bundle exercises the **HLSL target
pipeline**: the ordered IR-pass + emit sequence run by
`linkAndOptimizeIR` and `emitEntryPointsSourceFromIR` when
`CodeGenTarget::HLSL` (or a downstream-HLSL target such as DXIL /
DXBytecode) is requested. The doc enumerates four phases (A: link
and entry-point prep; B: specialization and type legalization; C:
HLSL legalization, lowering, phi elimination; D: HLSL emit +
DXC/fxc). The defining characteristic of this bundle is that almost
every claim is observable in **emitted HLSL text** — DCE, DXC, fxc
are out of band and not invoked by `-target hlsl`.

This bundle is therefore **single-target-heavy by design**: the
default test directive is

```
//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute
```

Multi-backend probes are only added when the claim is explicitly
about "HLSL fires this pass but the sibling targets do not" — i.e.
when the doc names a sibling target as filtered-out. In that case
add one `-target glsl` (or other sibling) directive with `CHECK-GLSL`
patterns to show the construct emits differently on the sibling and
HLSL-specific on HLSL.

## The translation rule: claims to observations

`target-pipelines/hlsl.md` describes:

- The ordered `SLANG_PASS` sequence for HLSL in
  `linkAndOptimizeIR`.
- HLSL-specific gates: `legalizeNonVectorCompositeSelect`,
  `legalizeEmptyRayPayloadsForHLSL`,
  `legalizeNonStructParameterToStructForHLSL`,
  `wrapStructuredBuffersOfMatrices`, `legalizeUniformBufferLoad`,
  `legalizeByteAddressBufferOps` defaults, `legalizeLogicalAndOr`
  (HLSL is in the `isD3DTarget` arm).
- HLSL-specific skips: `lowerCooperativeVectors` (HLSL `break`),
  `lowerAppendConsumeStructuredBuffers` (HLSL has native types).
- Always-on emit shapes: `[numthreads(...)]`, `register(uN)` /
  `register(tN)` / `register(bN)` / `register(sN)`, `#pragma
  pack_matrix(column_major)`, the conditional
  `#ifdef SLANG_HLSL_ENABLE_NVAPI` NVAPI include, the `#line`
  directives the `SourceWriter` emits, `cbuffer ... : register(b)`,
  and the `eliminatePhis` pattern that introduces named temporaries
  for `if`/`else` control flow.
- Downstream: HLSL text is the artifact endpoint for
  `CodeGenTarget::HLSL`; DXC/fxc are not invoked. The bundle
  therefore does not test DXIL/DXBytecode bytecode.

### Observable claims (write tests for these)

- **`[numthreads(X,Y,Z)]` survives the pipeline.** A compute entry
  point with `[numthreads(8,4,2)]` emits the same `numthreads(8, 4, 2)`
  attribute on HLSL. (Anchor: `#phase-d-hlsl-emit-and-downstream-tools`
  or `#hlsl-target-pipeline`.)
- **`RWStructuredBuffer<T>` is annotated `register(u…)`,
  `Texture2D` / `ByteAddressBuffer` is `register(t…)`, `cbuffer` is
  `register(b…)`, `SamplerState` is `register(s…)`.** Each is a
  separate observable. (Anchor: `#phase-d-hlsl-emit-and-downstream-tools`.)
- **HLSL emit begins with `#pragma pack_matrix(column_major)`** and
  the conditional `#ifdef SLANG_HLSL_ENABLE_NVAPI` NVAPI header
  block (so the literal `#include "nvHLSLExtns.h"` is only chosen
  on caller's request). (Anchor: `#phase-d-hlsl-emit-and-downstream-tools`.)
- **`SourceWriter` emits `#line N "source.slang"` directives** in
  HLSL emit so DXC can map errors back. (Anchor:
  `#phase-d-hlsl-emit-and-downstream-tools`; cross-link to
  `pipeline/06-emit.md#source-writer-abstraction`.)
- **`legalizeNonVectorCompositeSelect` (HLSL-only) rewrites
  `cond ? structA : structB`** into a `if/else` block with an
  explicit temporary on HLSL. (Anchor:
  `#legalizenonvectorcompositeselect`.)
- **`legalizeLogicalAndOr` keeps `&&` over vector operands as a
  vector expression** (DXC short-circuit-evaluates scalars only;
  the IR pass rewrites short-circuit forms to element-wise selects,
  so the emitted text shows the legalized form rather than `&&` on
  scalars). (Anchor: `#legalizelogicalandor`.)
- **`wrapStructuredBuffersOfMatrices` (HLSL-only) wraps the matrix
  element type of a `StructuredBuffer`/`RWStructuredBuffer` in a
  single-field struct** so `#pragma pack_matrix` applies; the
  emitted HLSL replaces `RWStructuredBuffer<float4x4>` with a
  `RWStructuredBuffer<_S<N>>` whose field is the matrix.
  (Anchor: `#wrapstructuredbuffersofmatrices`.)
- **`row_major` declared in a `cbuffer` becomes a
  `_MatrixStorage_float4x4natural_*` struct** (storage-type
  lowering for the buffer's matrix element). (Anchor:
  `#wrapstructuredbuffersofmatrices` and
  `#phase-d-hlsl-emit-and-downstream-tools`.)
- **HLSL has native `AppendStructuredBuffer<T>` and
  `ConsumeStructuredBuffer<T>` types**, so
  `lowerAppendConsumeStructuredBuffers` is skipped and the emit
  preserves the type spellings. (Anchor:
  `#phase-b-specialization-and-type-legalization`.)
- **`lowerCombinedTextureSamplers` fires for HLSL**, so a Slang
  `Sampler2D` splits into a separate `Texture2D<...>` and
  `SamplerState` pair in the HLSL emit. (Anchor:
  `#lowercombinedtexturesamplers`.)
- **`legalizeByteAddressBufferOps` runs with default HLSL options**:
  `ByteAddressBuffer.Load<T>(...)` and
  `RWByteAddressBuffer.Store<T>(...)` survive as templated method
  calls on the buffer. (Anchor:
  `#legalizebyteaddressbufferops-for-hlsl`.)
- **`eliminatePhis` with default options** translates an
  if/else-merged value into an explicit local temporary that is
  assigned in each branch (DXC re-SSA's it later). (Anchor:
  `#eliminatephis-with-default-options`.)
- **`moveGlobalVarInitializationToEntryPoints`** runs in the
  HLSL/GLSL/WGSL arm: a `static` module-scope variable's
  initializer fires inside the entry point on HLSL. (Anchor:
  `#phase-c-hlsl-legalization-lowering-phi-elimination`.)
- **`legalizeArrayReturnType` rewrites `T[N] foo()` into
  `void foo(out T[N])`** because DXC disallows array return values.
  (Anchor: `#phase-c-hlsl-legalization-lowering-phi-elimination`.)
- **`lowerEnumType` collapses an `enum` to its underlying integer**
  on HLSL: the emitted text contains the integer literal, not the
  enumerator name. (Anchor:
  `#phase-a-link-and-entry-point-prep` + `#conditional-gates`.)
- **HLSL is `isD3DTarget` and is **non-Khronos**.** A construct
  that is gated on `isKhronosTarget` (e.g. the GLSL SSBO
  lowering) does not emit GLSL `buffer` qualifiers in HLSL output.
  This is a negative observation, cross-checked by compiling the
  same source to `-target glsl` and confirming the GLSL `buffer`
  qualifier appears there.
- **Downstream is delegated to DXC / fxc.** `-target hlsl` stops
  at HLSL text; no SPIR-V tools, no validation pass, no DXIL,
  no bytecode comes out for `-target hlsl`. (Anchor:
  `#downstream-dxc-fxc`.)
- **HLSL has no `linkAndOptimizeIR` loop.** This is a textual
  statement in the doc, not directly testable through compiled
  output; we record it as a non-emit-observable claim in the
  "Out of scope" section.
- **`cbuffer` declarations emit as `cbuffer X : register(bN) { ... }`**
  on HLSL (cbuffer-shaped, with `register(b)` resource binding).
  (Anchor: `#phase-d-hlsl-emit-and-downstream-tools`.)
- **The HLSL entry point function name** preserves the user's
  entry-point name (`main` → `void main(...)`); HLSL does not
  rename the entry point. (Anchor:
  `#phase-d-hlsl-emit-and-downstream-tools`.)

### Not testable through `slangc -target hlsl` (record under
`## Out of scope (no-GPU runner)`)

- **DXC / fxc invocation**, DXIL bytecode emission. Requires a
  Windows DXC build (or a Linux DXC) plus a real graphics
  driver; not exercised on the no-GPU runner.
- **Pass ordering within Phase A/B/C.** Pass _existence_ is
  observable through its effect on emitted text; pass _ordering_
  is an IR-level claim that requires `-dump-ir` annotations
  cross-pass which the doc does not anchor to a specific marker.
  These pass-ordering claims live in `pipeline/05-ir-passes`.
- **`legalizeEmptyRayPayloadsForHLSL` and
  `legalizeNonStructParameterToStructForHLSL`.** Both require a
  DXR (`closesthit` / `anyhit`) entry point. The no-GPU compute
  runner does not exercise the ray-tracing pipeline shape, and
  the doc anchors these to DXR stages.
- **`floatNonUniformResourceIndex`.** Requires the
  `NonUniformResourceIndex(...)` HLSL intrinsic in source, which
  Slang's compute-stage entry point does not have a natural way
  to surface without the broader resource-indexing setup.
- **`invertYOfPositionOutput` and `rcpWOfPositionInput`.** Both
  are gated on Vulkan-cross-API options
  (`-emit-spirv-via-glsl`-style flags) that are out of scope for
  a compute-target HLSL bundle.
- **`collectCooperativeMetadata`.** Requires the cooperative
  matrix or vector capability set.
- **`legalizeUniformBufferLoad` and `legalizeMeshOutputTypes`.**
  Both are conditional and not generally observable as a single
  HLSL text pattern; the doc does not anchor a checkable marker.
- **HLSL's iteration loops in `linkAndOptimizeIR`.** The doc
  explicitly states HLSL has **no** iterative passes; the
  consequence is "no extra simplification loop appears in the
  pass log" which is not a `slangc`-text-emit observable.
- **`profile.getVersion() <= DX_5_0` byte-address-buffer flag.**
  The fxc-era `useBitCastFromUInt=true` option requires the
  `-profile sm_5_0` selection, which then routes through fxc —
  fxc is out of scope as above.

## Required structure

1. `BUNDLE.md` with the structure named in `_common.md`.
2. 20 to 35 `.slang` test files. Bundle is HLSL-target-focused,
   so the typical file carries one `//TEST:SIMPLE` directive
   targeting HLSL. Cross-target probes are reserved for the
   "HLSL fires this pass, sibling does not" subset.

## Doc sources

Primary (every `doc_ref` resolves into this):

- `docs/llm-generated/target-pipelines/hlsl.md`

Secondary (allowed citations; use sparingly and only when the
primary doc explicitly hands off):

- `docs/llm-generated/pipeline/05-ir-passes.md`
- `docs/llm-generated/pipeline/06-emit.md`
- `docs/llm-generated/target-pipelines/index.md`
- `docs/llm-generated/cross-cutting/targets.md`

If you would cite anything else, stop and instead record a
doc-gap finding in `BUNDLE.md`.

## Source files you may consult for _verification only_

Use these to confirm a specific HLSL emit token. Do **not** mine
them for claims that the doc does not state.

- `source/slang/slang-emit.cpp`
- `source/slang/slang-emit-hlsl.cpp`
- `source/slang/slang-emit-hlsl-prelude.cpp`
- `source/slang/slang-emit-c-like.cpp`
- `source/slang/slang-ir-hlsl-legalize.cpp`
- `source/slang/slang-ir-wrap-structured-buffers.cpp`
- `source/slang/slang-ir-legalize-binary-operator.cpp`
- `source/slang/slang-ir-byte-address-legalize.cpp`

## Test directives

Default for this bundle:

```
//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute
```

For an HLSL-fires-vs-sibling-skips claim, add a second directive
with a distinct label and per-target CHECK prefix:

```
//TEST:SIMPLE(filecheck=CHECK):-target hlsl  -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl  -entry main -stage compute
```

## Lessons captured for the HLSL target pipeline

These are in addition to `_common.md` and `pipeline-06-emit.md`.

- **`row_major` does not survive HLSL emit as a literal token.**
  A `row_major` matrix declared in a `cbuffer` is wrapped as a
  `_MatrixStorage_<spelling>natural_<N>` struct holding the
  matrix as a `float<R>[<C>]` array. The CHECK pattern is
  `_MatrixStorage_{{.*}}`, not the literal `row_major`. (Wave-6
  lesson.) GLSL, in contrast, _does_ emit a `layout(row_major)`
  qualifier.
- **`StructuredBuffer<float4x4>` wraps as a single-field struct.**
  HLSL emit becomes `RWStructuredBuffer<_S<N>>` where `_S<N>` is a
  generated anonymous struct with one `float4x4` field. Use a
  wildcard for the generated name.
- **The NVAPI include is conditional.** Match the surrounding
  `#ifdef SLANG_HLSL_ENABLE_NVAPI` rather than the literal
  `#include "nvHLSLExtns.h"`.
- **`#pragma pack_matrix(column_major)` is the first non-blank
  emitted line on every HLSL compile.** Match it as the canonical
  HLSL-prelude marker.
- **`Sampler2D` splits to `combined_texture_<N>` /
  `combined_sampler_<N>` named pair** with `register(t)` and
  `register(s)` respectively.
- **`AppendStructuredBuffer` / `ConsumeStructuredBuffer` retain
  their type names** in HLSL emit — they are not lowered.
- **`ByteAddressBuffer.Load<T>(...)` survives as
  `(buf).Load<T>(...)` in HLSL emit.** `RWByteAddressBuffer.Store`
  lowers to a `Store(offset, val)` form (no template parameter on
  scalar stores).
- **`#line N "<file>"` directives appear in HLSL emit by default**
  and may reference `core.meta.slang` / `hlsl.meta.slang` for
  core-module-sourced items. Use a wildcard for the file name.
- **Enum lowering eats the enumerator name.** `Color.Green` becomes
  `int(1)` in HLSL emit; there is no surviving `Green` identifier.
- **HLSL output always emits `void main(...)`** for an entry point
  named `main` — Slang does not rename the entry point in HLSL.
- **Array-return functions become `void` with `out` parameters.**
  `int[4] foo()` becomes `void foo_0(..., out int _S1[4])` in HLSL.
- **`a && b` over a vector** emits with the operator preserved as
  `vector<bool,3>(...) && vector<bool,3>(...)` in HLSL (the IR
  pass `legalizeLogicalAndOr` rewrites the short-circuit to
  element-wise but HLSL accepts both forms; verify what the
  current compiler actually emits and pin that pattern).
- **DCE strips locally-unused code.** Always write computed values
  to a buffer/return them, or the test pattern will fail to find
  them in the emit.
- **`static` module-scope variables show their initializer fire
  inside `main`** (the side effect of
  `moveGlobalVarInitializationToEntryPoints`); the global itself
  becomes a plain HLSL `static T name_0` declaration but the
  initializer expression appears at function entry.

## Quality checklist (in addition to `_common.md`'s)

- [ ] Every test's `doc_ref` resolves to an anchor in
      `target-pipelines/hlsl.md` (or one of the listed secondary
      docs).
- [ ] Each `.slang` file declares
      `//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute`
      as its primary directive. Add a `-target glsl` (or other)
      sibling directive only for HLSL-fires-but-sibling-skips
      claims.
- [ ] CHECK patterns avoid raw `[[...]]` and use FileCheck
      wildcards for mangled identifiers.
- [ ] No test depends on a GPU. Compute-only entry points; no
      DXR; no graphics pipeline stages requiring rasterization.
- [ ] `## Doc gaps observed` records claims that lack a checkable
      marker in the doc, or behaviors observed in slangc emit
      that the doc does not currently describe.
- [ ] `## Out of scope (no-GPU runner)` enumerates the
      DXC/fxc/DXIL/DXR/profile-gated items.
