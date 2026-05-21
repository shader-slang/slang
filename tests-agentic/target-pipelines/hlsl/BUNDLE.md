---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-21T12:00:00+00:00
source_commit: 1106750632bd5fb062ea9e50319f7763d34f78d5
watched_paths_digest: ee6e5a27774a8a9c7ea3024cc5842f7bc1919255c4c0b9751400d25195f0b7bc
source_doc: docs/llm-generated/target-pipelines/hlsl.md
source_doc_digest: 2afce914bae96fe2e024bf6b3685b7339df010398abc456d4a58cb329326fa50
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for target-pipelines/hlsl

## Intent

Tests verify the HLSL target pipeline described in
[`docs/llm-generated/target-pipelines/hlsl.md`](../../../docs/llm-generated/target-pipelines/hlsl.md):
the ordered IR-pass sequence run by `linkAndOptimizeIR` and
`emitEntryPointsSourceFromIR` when `CodeGenTarget::HLSL` is
requested, and the HLSL-shaped text the emit phase produces.
The bundle exercises Phase D emit invariants (the
`#pragma pack_matrix(column_major)` prelude header, the
conditional `#ifdef SLANG_HLSL_ENABLE_NVAPI` NVAPI block,
`#line` directives, `register(u|t|b|s N)` resource
annotations, `[numthreads(...)]` entry-point attributes,
`SV_DispatchThreadID` semantic survival, `cbuffer` shape,
struct emission, vector spelling, `groupshared` qualifier
survival, HLSL native `AppendStructuredBuffer` /
`ConsumeStructuredBuffer` retention, and the entry-point name
preservation), plus the HLSL-specific IR-pass effects
(`legalizeNonVectorCompositeSelect`,
`wrapStructuredBuffersOfMatrices` for both matrix-element
StructuredBuffers and cbuffer matrices,
`lowerCombinedTextureSamplers`, `legalizeByteAddressBufferOps`
default options, `legalizeArrayReturnType`, `lowerEnumType`,
`eliminatePhis` default options, atomic-operation survival,
and the HLSL-vs-GLSL non-Khronos contrast). The bundle stops
at HLSL text — DXC, fxc, DXIL, DXBytecode, DXR, and the
fxc-era profile-gated paths are out of scope on the no-GPU
runner.

Coverage strategy: one test per concrete claim in the doc's
Phase A/B/C/D tables that can be observed in
`slangc -target hlsl` text. Default directive is
`//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute`.
One file (`non-khronos-hlsl-vs-glsl-buffer.slang`) carries a
second `-target glsl` directive because the claim is
specifically about HLSL being non-Khronos.

## Claims enumerated

| Claim ID | Anchor                                                                                                                                                                                       | Claim (one line)                                                                                                                                                  | Tests                                                                                                                                  |
| -------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------- |
| C-01     | [#phase-d-hlsl-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools)                                                       | The `[numthreads(X,Y,Z)]` attribute survives the pipeline as `numthreads(X, Y, Z)` on the HLSL entry point.                                                       | `numthreads-attribute-survives.slang`                                                                                                  |
| C-02     | [#phase-d-hlsl-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools)                                                       | An `RWStructuredBuffer<T>` is bound `register(uN)` in HLSL emit.                                                                                                  | `register-u-for-uav.slang`, `multiple-resources-distinct-registers.slang`                                                              |
| C-03     | [#phase-d-hlsl-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools)                                                       | A read-only resource (`ByteAddressBuffer`, `Texture2D`) is bound `register(tN)`.                                                                                  | `register-t-for-srv.slang`                                                                                                             |
| C-04     | [#phase-d-hlsl-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools)                                                       | A `cbuffer` (or `ConstantBuffer<T>`) is bound `register(bN)` and emits as a `cbuffer X : register(bN) { ... }` block.                                             | `register-b-for-cbuffer.slang`, `cbuffer-shape-struct-then-cbuffer.slang`, `constant-buffer-emits-as-cbuffer.slang`                    |
| C-05     | [#phase-d-hlsl-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools)                                                       | A `SamplerState` is bound `register(sN)`.                                                                                                                         | `register-s-for-sampler.slang`                                                                                                         |
| C-06     | [#phase-d-hlsl-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools)                                                       | HLSL emit opens with `#pragma pack_matrix(column_major)` to fix matrix layout for the downstream compiler.                                                        | `prelude-pack-matrix-pragma.slang`                                                                                                     |
| C-07     | [#phase-d-hlsl-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools)                                                       | HLSL emit wraps `nvHLSLExtns.h` in `#ifdef SLANG_HLSL_ENABLE_NVAPI` so the NVAPI include is opt-in.                                                               | `prelude-nvapi-include-conditional.slang`                                                                                              |
| C-08     | [#phase-d-hlsl-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools)                                                       | The `SourceWriter` emits `#line N "file.slang"` directives so DXC/fxc can map errors back to Slang source.                                                        | `sourcewriter-emits-line-directives.slang`, `line-directive-references-core-meta.slang`                                                |
| C-09     | [#legalizenonvectorcompositeselect](../../../docs/llm-generated/target-pipelines/hlsl.md#legalizenonvectorcompositeselect)                                                                   | `legalizeNonVectorCompositeSelect` (HLSL-only) rewrites `cond ? structA : structB` into an explicit `if/else` with a temporary because DXC's `select` is vector-only. | `legalize-non-vector-composite-select.slang`                                                                                           |
| C-10     | [#wrapstructuredbuffersofmatrices](../../../docs/llm-generated/target-pipelines/hlsl.md#wrapstructuredbuffersofmatrices)                                                                     | `wrapStructuredBuffersOfMatrices` wraps a matrix element type into a single-field struct (`_S<N>`) for the StructuredBuffer.                                      | `wrap-structured-buffer-of-matrix.slang`                                                                                               |
| C-11     | [#wrapstructuredbuffersofmatrices](../../../docs/llm-generated/target-pipelines/hlsl.md#wrapstructuredbuffersofmatrices)                                                                     | A `row_major` matrix declared in a `cbuffer` is rewritten into a `_MatrixStorage_<spelling>natural_<N>` storage struct on HLSL emit; the literal `row_major` does not survive. | `row-major-matrix-storage-struct.slang`                                                                                                |
| C-12     | [#wrapstructuredbuffersofmatrices](../../../docs/llm-generated/target-pipelines/hlsl.md#wrapstructuredbuffersofmatrices)                                                                     | `wrapStructuredBuffersOfMatrices` does NOT wrap a non-matrix element type; `RWStructuredBuffer<int>` stays as-is.                                                 | `structured-buffer-of-int-no-wrap.slang`                                                                                               |
| C-13     | [#phase-b-specialization-and-type-legalization](../../../docs/llm-generated/target-pipelines/hlsl.md#phase-b-specialization-and-type-legalization)                                           | `lowerAppendConsumeStructuredBuffers` is skipped for HLSL because HLSL has native `AppendStructuredBuffer` / `ConsumeStructuredBuffer`; both type spellings survive emit. | `append-structured-buffer-survives.slang`                                                                                              |
| C-14     | [#lowercombinedtexturesamplers](../../../docs/llm-generated/target-pipelines/hlsl.md#lowercombinedtexturesamplers)                                                                           | `lowerCombinedTextureSamplers` splits a Slang `Sampler2D` into a `combined_texture_<N>` (`register(t)`) + `combined_sampler_<N>` (`register(s)`) pair.            | `lower-combined-texture-sampler.slang`                                                                                                 |
| C-15     | [#legalizebyteaddressbufferops-for-hlsl](../../../docs/llm-generated/target-pipelines/hlsl.md#legalizebyteaddressbufferops-for-hlsl)                                                         | HLSL uses the default `legalizeByteAddressBufferOps` options, so `.Load<T>(offset)` survives as a templated method call.                                          | `byte-address-buffer-load-template.slang`                                                                                              |
| C-16     | [#eliminatephis-with-default-options](../../../docs/llm-generated/target-pipelines/hlsl.md#eliminatephis-with-default-options)                                                               | HLSL uses `eliminatePhis` default options: an `if/else`-merged value becomes a named local temporary assigned in each branch.                                     | `eliminate-phis-default-options.slang`                                                                                                 |
| C-17     | [#phase-c-hlsl-legalization-lowering-phi-elimination](../../../docs/llm-generated/target-pipelines/hlsl.md#phase-c-hlsl-legalization-lowering-phi-elimination)                               | `legalizeArrayReturnType` rewrites `T[N] foo()` into `void foo(out T[N])` because DXC disallows array return values.                                              | `legalize-array-return-type.slang`                                                                                                     |
| C-18     | [#phase-a-link-and-entry-point-prep](../../../docs/llm-generated/target-pipelines/hlsl.md#phase-a-link-and-entry-point-prep)                                                                 | `lowerEnumType` collapses an `enum` to its underlying integer in HLSL emit; the enumerator name does not survive.                                                 | `enum-lowering-to-integer.slang`                                                                                                       |
| C-19     | [#hlsl-specific-runtime-predicates](../../../docs/llm-generated/target-pipelines/hlsl.md#hlsl-specific-runtime-predicates)                                                                   | HLSL is non-Khronos: the same `RWStructuredBuffer<int>` emits `register(uN)` on HLSL and `layout(...) buffer` on GLSL.                                            | `non-khronos-hlsl-vs-glsl-buffer.slang`                                                                                                |
| C-20     | [#downstream-dxc-fxc](../../../docs/llm-generated/target-pipelines/hlsl.md#downstream-dxc-fxc)                                                                                               | `-target hlsl` stops at the HLSL text artifact; no DXC / fxc / DXIL / DXBytecode invocation.                                                                      | `downstream-stops-at-hlsl-text.slang`                                                                                                  |
| C-21     | [#phase-d-hlsl-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools)                                                       | The HLSL entry-point function name preserves the user's entry name (`main` → `void main(...)`).                                                                   | `entry-point-name-main-preserved.slang`                                                                                                |
| C-22     | [#phase-d-hlsl-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools)                                                       | The `SV_DispatchThreadID` system-value semantic survives the pipeline onto the HLSL entry-point parameter list.                                                   | `sv-dispatch-thread-id-survives.slang`                                                                                                 |
| C-23     | [#phase-d-hlsl-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools)                                                       | A user-declared `struct` keeps its `struct Name { ... };` shape on HLSL emit (shared `CLikeSourceEmitter` convention).                                            | `struct-declaration-emitted-as-struct.slang`                                                                                           |
| C-24     | [#phase-d-hlsl-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools)                                                       | HLSL emit spells vectors as `float<N>` — not `vec<N>` (GLSL) and not `vec<N><f32>` (WGSL).                                                                        | `float-vector-keeps-hlsl-spelling.slang`                                                                                               |
| C-25     | [#phase-d-hlsl-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools)                                                       | A `groupshared` array survives the pipeline as `groupshared T name[N];` on HLSL emit.                                                                             | `groupshared-memory-survives.slang`                                                                                                    |
| C-26     | [#phase-c-hlsl-legalization-lowering-phi-elimination](../../../docs/llm-generated/target-pipelines/hlsl.md#phase-c-hlsl-legalization-lowering-phi-elimination)                               | An atomic operation on a UAV passes `validateAtomicOperations` and emits as the matching HLSL `Interlocked*` intrinsic.                                           | `atomic-operation-survives.slang`                                                                                                      |
| C-27     | [#phase-c-hlsl-legalization-lowering-phi-elimination](../../../docs/llm-generated/target-pipelines/hlsl.md#phase-c-hlsl-legalization-lowering-phi-elimination)                               | A `static` module-scope variable is emitted as a `static T name = init;` declaration on HLSL.                                                                     | `move-global-var-init-to-entry.slang`                                                                                                  |
| C-28     | [#phase-d-hlsl-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools)                                                       | Control-flow statements (for / while / do-while / switch / break / continue) emit as their HLSL spellings through the shared CLikeSourceEmitter.                  | `for-loop-statement-emission.slang`, `while-loop-statement-emission.slang`, `do-while-loop-statement-emission.slang`, `switch-case-statement-emission.slang`, `continue-in-loop-emission.slang`, `break-in-loop-emit.slang` |
| C-29     | [#phase-d-hlsl-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools)                                                       | Binary / unary / bitwise / shift operators ride through the shared C-like emitter and emit as HLSL operator spellings.                                            | `expression-binary-precedence-emit.slang`, `expression-bitwise-operators-emit.slang`, `expression-shift-operators-emit.slang`, `expression-unary-negate-emit.slang`                                                                |
| C-30     | [#phase-d-hlsl-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools)                                                       | Explicit primitive type conversions emit as the HLSL constructor-style cast `T(expr)`.                                                                            | `cast-constructor-style-emit.slang`, `cast-uint-to-float-emit.slang`                                                                                                                                                              |
| C-31     | [#phase-d-hlsl-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools)                                                       | User-defined helper functions emit as named HLSL functions with the `_<N>` suffix, callable from the entry point.                                                 | `function-call-user-defined-emit.slang`, `stress-many-function-parameters.slang`                                                                                                                                                  |
| C-32     | [#phase-d-hlsl-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools)                                                       | Local fixed-size array declarations emit as `T name[N]` in HLSL.                                                                                                  | `local-array-declaration-emit.slang`                                                                                                                                                                                              |
| C-33     | [#phase-d-hlsl-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools)                                                       | Texture3D and TextureCube resources keep their HLSL spellings and bind to the `t` register class.                                                                 | `texture3d-sample-emit.slang`, `texturecube-sample-emit.slang`, `texture1d-emit.slang`, `texture1d-load-emit.slang`, `texture1d-explicit-register-zero.slang`, `texture3d-emit.slang`, `texture3d-samplelevel-emit.slang`, `texture3d-explicit-t-high-index.slang`, `texturecube-samplegrad-emit.slang`, `texture2d-samplegrad-emit.slang`, `texturecubearray-emit.slang`, `texture2darray-emit.slang`, `texture2dms-emit.slang`, `texture2dms-sample-count-eight.slang`, `texture2dms-load-sample-index.slang`, `rwtexture1d-emit.slang`, `rwtexture2d-emit.slang`, `rwtexture3d-emit.slang`, `rwtexture3d-explicit-u0.slang`, `rwtexture1d-explicit-u-high-index.slang`, `samplercmp-emit.slang`, `samplercmp-explicit-s0.slang`, `samplercmp-samplecmplevelzero-emit.slang`, `texture2d-gatherred-emit.slang`, `texture2d-gathergreen-emit.slang`, `texture2darray-load-emit.slang`, `multiple-textures-distinct-t-registers.slang`, `rwtexture-and-srv-texture-distinct-registers.slang`, `texturecube-element-float-emit.slang`, `texture-and-samplercmp-distinct-s-registers.slang`, `texture2d-sample-emit.slang`, `texture3d-and-rwtexture3d-share-binding-space.slang` |
| C-34     | [#phase-d-hlsl-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools)                                                       | Nested struct fields (a struct containing another user struct) emit both `struct` declarations in HLSL.                                                           | `struct-nested-fields-emit.slang`                                                                                                                                                                                                 |
| C-35     | [#phase-d-hlsl-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools)                                                       | Vector swizzle (`v.xy`) survives HLSL emit using the canonical `.xyzw` spelling.                                                                                  | `vector-swizzle-xyzw-emit.slang`                                                                                                                                                                                                  |
| C-36     | [#phase-b-specialization-and-type-legalization](../../../docs/llm-generated/target-pipelines/hlsl.md#phase-b-specialization-and-type-legalization)                                           | `checkForMissingReturns` (Phase B, `reqSet.missingReturn`) rejects non-void functions whose control flow can fall off the end before HLSL emit.                   | `missing-return-diag.slang`                                                                                                                                                                                                       |

## Tests in this bundle

| File                                                  | Intent     | Doc anchor                                                |
| ----------------------------------------------------- | ---------- | --------------------------------------------------------- |
| `numthreads-attribute-survives.slang`                 | functional | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `register-u-for-uav.slang`                            | functional | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `register-t-for-srv.slang`                            | functional | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `register-b-for-cbuffer.slang`                        | functional | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `register-s-for-sampler.slang`                        | functional | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `prelude-pack-matrix-pragma.slang`                    | functional | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `prelude-nvapi-include-conditional.slang`             | functional | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `sourcewriter-emits-line-directives.slang`            | functional | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `line-directive-references-core-meta.slang`           | functional | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `legalize-non-vector-composite-select.slang`          | functional | `#legalizenonvectorcompositeselect`                       |
| `wrap-structured-buffer-of-matrix.slang`              | functional | `#wrapstructuredbuffersofmatrices`                        |
| `row-major-matrix-storage-struct.slang`               | functional | `#wrapstructuredbuffersofmatrices`                        |
| `structured-buffer-of-int-no-wrap.slang`              | functional | `#wrapstructuredbuffersofmatrices`                        |
| `append-structured-buffer-survives.slang`             | functional | `#phase-b-specialization-and-type-legalization`           |
| `lower-combined-texture-sampler.slang`                | functional | `#lowercombinedtexturesamplers`                           |
| `byte-address-buffer-load-template.slang`             | functional | `#legalizebyteaddressbufferops-for-hlsl`                  |
| `eliminate-phis-default-options.slang`                | functional | `#eliminatephis-with-default-options`                     |
| `legalize-array-return-type.slang`                    | functional | `#phase-c-hlsl-legalization-lowering-phi-elimination`     |
| `enum-lowering-to-integer.slang`                      | functional | `#phase-a-link-and-entry-point-prep`                      |
| `entry-point-name-main-preserved.slang`               | functional | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `cbuffer-shape-struct-then-cbuffer.slang`             | functional | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `constant-buffer-emits-as-cbuffer.slang`              | functional | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `move-global-var-init-to-entry.slang`                 | functional | `#phase-c-hlsl-legalization-lowering-phi-elimination`     |
| `non-khronos-hlsl-vs-glsl-buffer.slang`               | functional | `#hlsl-specific-runtime-predicates`                       |
| `downstream-stops-at-hlsl-text.slang`                 | functional | `#downstream-dxc-fxc`                                     |
| `multiple-resources-distinct-registers.slang`         | functional | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `sv-dispatch-thread-id-survives.slang`                | functional | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `struct-declaration-emitted-as-struct.slang`          | functional | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `float-vector-keeps-hlsl-spelling.slang`              | functional | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `groupshared-memory-survives.slang`                   | functional | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `atomic-operation-survives.slang`                     | functional | `#phase-c-hlsl-legalization-lowering-phi-elimination`     |
| `register-u-explicit-zero.slang`                      | boundary   | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `register-u-explicit-high-index.slang`                | boundary   | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `register-t-explicit-zero.slang`                      | boundary   | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `register-b-explicit-zero.slang`                      | boundary   | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `register-conflicting-bindings-diag.slang`            | negative   | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `cbuffer-single-scalar.slang`                         | boundary   | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `cbuffer-array-of-row-major-matrix.slang`             | boundary   | `#wrapstructuredbuffersofmatrices`                        |
| `cbuffer-many-mixed-fields.slang`                     | stress     | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `uint-literal-max-emits-decimal-u.slang`              | boundary   | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `uint-literal-zero.slang`                             | boundary   | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `int-literal-min-emits-decimal.slang`                 | boundary   | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `int-literal-max-emits-decimal.slang`                 | boundary   | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `float-positive-infinity-divide.slang`                | boundary   | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `float-nan-divide.slang`                              | boundary   | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `float-largest-finite.slang`                          | boundary   | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `float-smallest-subnormal.slang`                      | boundary   | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `float-negative-zero-literal.slang`                   | boundary   | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `numthreads-one-one-one.slang`                        | boundary   | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `numthreads-max-x-1024.slang`                         | boundary   | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `numthreads-zero-rejected-diag.slang`                 | negative   | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `cbuffer-column-major-matrix.slang`                   | boundary   | `#wrapstructuredbuffersofmatrices`                        |
| `cbuffer-default-matrix-no-qualifier.slang`           | boundary   | `#wrapstructuredbuffersofmatrices`                        |
| `row-major-inout-function-param.slang`                | boundary   | `#wrapstructuredbuffersofmatrices`                        |
| `nvapi-guard-present-without-nv-intrinsic.slang`      | boundary   | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `nvapi-guard-with-wave-intrinsic.slang`               | boundary   | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `logical-and-short-circuit-side-effect.slang`         | boundary   | `#phase-c-hlsl-legalization-lowering-phi-elimination`     |
| `logical-or-short-circuit-side-effect.slang`          | boundary   | `#phase-c-hlsl-legalization-lowering-phi-elimination`     |
| `constant-buffer-scalar-element.slang`                | boundary   | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `texture2d-sample-origin-corner.slang`                | boundary   | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `vk-binding-ignored-on-hlsl.slang`                    | boundary   | `#hlsl-specific-runtime-predicates`                       |
| `profile-cs-6-0-emits-prelude.slang`                  | boundary   | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `profile-cs-6-8-emits-prelude.slang`                  | boundary   | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `profile-unknown-rejected-diag.slang`                 | negative   | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `stress-resource-array-eight-uavs.slang`              | stress     | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `stress-deeply-nested-if-else.slang`                  | stress     | `#eliminatephis-with-default-options`                     |
| `stress-all-paths-return.slang`                       | stress     | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `stress-structured-buffer-of-double-matrix.slang`     | stress     | `#wrapstructuredbuffersofmatrices`                        |
| `structured-buffer-of-float-no-wrap.slang`            | boundary   | `#wrapstructuredbuffersofmatrices`                        |
| `register-s-explicit-zero.slang`                      | boundary   | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `register-s-explicit-high-index.slang`                | boundary   | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `register-t-explicit-high-index.slang`                | boundary   | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `register-b-explicit-high-index.slang`                | boundary   | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `for-loop-statement-emission.slang`                   | expansion  | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `while-loop-statement-emission.slang`                 | expansion  | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `do-while-loop-statement-emission.slang`              | expansion  | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `switch-case-statement-emission.slang`                | expansion  | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `continue-in-loop-emission.slang`                     | expansion  | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `break-in-loop-emit.slang`                            | expansion  | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `expression-binary-precedence-emit.slang`             | expansion  | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `expression-bitwise-operators-emit.slang`             | expansion  | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `expression-shift-operators-emit.slang`               | expansion  | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `expression-unary-negate-emit.slang`                  | expansion  | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `cast-constructor-style-emit.slang`                   | expansion  | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `cast-uint-to-float-emit.slang`                       | expansion  | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `function-call-user-defined-emit.slang`               | expansion  | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `local-array-declaration-emit.slang`                  | expansion  | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `vector-float4-largest-dim.slang`                     | boundary   | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `vector-swizzle-xyzw-emit.slang`                      | expansion  | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `int-vector-keeps-hlsl-spelling.slang`                | boundary   | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `consume-structured-buffer-survives.slang`            | boundary   | `#phase-b-specialization-and-type-legalization`           |
| `wrap-structured-buffer-of-half-matrix.slang`         | boundary   | `#wrapstructuredbuffersofmatrices`                        |
| `structured-buffer-of-vector-no-wrap.slang`           | boundary   | `#wrapstructuredbuffersofmatrices`                        |
| `structured-buffer-of-user-struct-no-wrap.slang`      | boundary   | `#wrapstructuredbuffersofmatrices`                        |
| `stress-many-case-switch.slang`                       | stress     | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `stress-deeply-nested-for-loops.slang`                | stress     | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `stress-many-function-parameters.slang`               | stress     | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `numthreads-y-max-1024.slang`                         | boundary   | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `byte-address-buffer-load-float.slang`                | boundary   | `#legalizebyteaddressbufferops-for-hlsl`                  |
| `missing-return-diag.slang`                           | negative   | `#phase-b-specialization-and-type-legalization`           |
| `texture3d-sample-emit.slang`                         | expansion  | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `texturecube-sample-emit.slang`                       | expansion  | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `empty-function-body-emit.slang`                      | boundary   | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `cbuffer-vector-field.slang`                          | boundary   | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `struct-nested-fields-emit.slang`                     | expansion  | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `texture1d-emit.slang`                                | expansion  | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `texture1d-load-emit.slang`                           | expansion  | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `texture1d-explicit-register-zero.slang`              | boundary   | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `texture3d-emit.slang`                                | expansion  | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `texture3d-samplelevel-emit.slang`                    | expansion  | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `texturecube-samplegrad-emit.slang`                   | expansion  | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `texture2d-samplegrad-emit.slang`                     | expansion  | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `texturecubearray-emit.slang`                         | expansion  | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `texture2darray-emit.slang`                           | expansion  | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `texture2dms-emit.slang`                              | expansion  | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `texture2dms-sample-count-eight.slang`                | boundary   | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `rwtexture1d-emit.slang`                              | expansion  | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `rwtexture3d-emit.slang`                              | expansion  | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `rwtexture2d-emit.slang`                              | expansion  | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `rwtexture3d-explicit-u0.slang`                       | boundary   | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `samplercmp-emit.slang`                               | expansion  | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `samplercmp-explicit-s0.slang`                        | boundary   | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `samplercmp-samplecmplevelzero-emit.slang`            | expansion  | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `texture2d-gatherred-emit.slang`                      | expansion  | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `texture2d-gathergreen-emit.slang`                    | expansion  | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `texture2darray-load-emit.slang`                      | expansion  | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `multiple-textures-distinct-t-registers.slang`        | boundary   | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `rwtexture-and-srv-texture-distinct-registers.slang`  | boundary   | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `texture3d-explicit-t-high-index.slang`               | boundary   | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `rwtexture1d-explicit-u-high-index.slang`             | boundary   | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `texturecube-element-float-emit.slang`                | boundary   | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `texture2dms-load-sample-index.slang`                 | boundary   | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `texture-and-samplercmp-distinct-s-registers.slang`   | boundary   | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `texture2d-sample-emit.slang`                         | expansion  | `#phase-d-hlsl-emit-and-downstream-tools`                 |
| `texture3d-and-rwtexture3d-share-binding-space.slang` | boundary   | `#phase-d-hlsl-emit-and-downstream-tools`                 |

## Doc gaps observed

- The doc's Phase D table mentions the `HLSLSourceEmitter` and the
  emit step but does not name `#pragma pack_matrix(column_major)`
  as the canonical HLSL prelude marker, or describe the
  `#ifdef SLANG_HLSL_ENABLE_NVAPI` guarded `nvHLSLExtns.h` include.
  A one-line statement of these two emit-prelude facts would
  let a test anchor them precisely; this bundle anchors them
  to the general `#phase-d-hlsl-emit-and-downstream-tools`
  section.
- The doc's `## wrapStructuredBuffersOfMatrices` section
  describes wrapping the matrix element of a `StructuredBuffer`
  but does not state the emit-time naming convention
  (`_S<N>` for the wrapper struct, `_MatrixStorage_<spelling>natural_<N>`
  for the storage form picked up via `cbuffer` with `row_major`).
  Documenting these names would let a test pin the exact
  spelling rather than a regex; this bundle uses regex
  wildcards.
- The doc does not state that `legalizeUniformBufferLoad` is
  observable through any specific HLSL emit pattern. We have
  no test for it because the doc does not anchor a checkable
  marker.
- The doc's `## legalizeLogicalAndOr` notes that DXC
  short-circuits `&&` / `||` on scalars only, and that the IR
  pass rewrites short-circuit ops over vectors. The doc does
  not specify what the emitted text looks like in the rewritten
  form (e.g. element-wise `select` calls vs. preserved `&&` on
  vectors that the underlying HLSL accepts). We do not test
  this claim because the emit-side observation is not
  pinned by the doc.
- The doc's `## floatNonUniformResourceIndex` mentions the
  `NonUniformResourceIndex(...)` HLSL intrinsic but does not
  give a Slang-language way to surface it on a compute-stage
  entry point. The natural surface is a graphics-pipeline
  bindless workflow that the no-GPU compute bundle cannot
  exercise; no test in this bundle.
- The doc's Phase A table lists `addDenormalModeDecorations`
  as always-on, but does not state any user-observable HLSL
  emit marker for denormal mode. No test.
- The doc lists `applyVariableScopeCorrection` as running for
  HLSL but does not name an observable emit pattern (HLSL
  scoping is implicit in the surrounding `if`/`while`
  blocks). No test.
- The doc's Phase C diagram mentions `unexportNonEmbeddableIR`
  as gated on `EmbedDownstreamIR`. The `-embed-downstream-ir`
  switch is a command-line surface; the doc does not anchor a
  text-emit marker.
- The doc's `## Source` table cites line numbers
  (`linkAndOptimizeIR` at line ~893, `emitEntryPointsSourceFromIR`
  at line ~2418, the `HLSLSourceEmitter` constructor at line
  ~2507). These are navigation aids and not user-observable;
  no test.
- The doc states "HLSL has no iterative passes in
  `linkAndOptimizeIR`" but the consequence ("no extra
  simplification loop in the pass log") is not observable
  through `slangc -target hlsl` text. No test.
- The doc's Phase D table mentions `HLSLSourceEmitter` walks the
  IR and writes HLSL text, but does not enumerate which texture
  variants survive (`Texture1D`, `Texture2DArray`,
  `TextureCubeArray`, `Texture2DMS<T,N>`, `RWTexture1D`,
  `RWTexture2D`, `RWTexture3D`, `SamplerComparisonState`) nor
  the canonical method spellings (`Sample`, `SampleLevel`,
  `SampleGrad`, `Load`, `GatherRed`/`GatherGreen`/`GatherBlue`/`GatherAlpha`,
  `SampleCmp`, `SampleCmpLevelZero`). A one-line statement that
  each Slang texture/sampler variant emits as the same HLSL
  native type name with the matching `register(t|u|s)` class
  would let tests anchor more precisely than the general
  `#phase-d-hlsl-emit-and-downstream-tools` section.

## Out of scope (no-GPU runner)

- **DXC / fxc invocation** and **DXIL / DXBytecode** output
  (`#downstream-dxc-fxc`). Requires a DXC binary; not invoked
  by `-target hlsl`.
- **`legalizeEmptyRayPayloadsForHLSL`** and
  **`legalizeNonStructParameterToStructForHLSL`**
  (`#legalizeemptyraypayloadsforhlsl`,
  `#legalizenonstructparametertostructforhlsl`). Both require
  DXR (`closesthit` / `anyhit` / `miss`) entry points; the
  no-GPU compute runner does not exercise DXR.
- **`legalizeUniformBufferLoad`**
  (`#legalizeuniformbufferload`). The doc anchors it as an
  IR-level canonicalization without naming an emit marker.
- **`legalizeMeshOutputTypes`**. Requires mesh-shader
  entry points.
- **`invertYOfPositionOutput` / `rcpWOfPositionInput`**.
  Requires Vulkan-cross-API option flags
  (`VulkanInvertY` / `VulkanUseDxPositionW`); orthogonal to
  a compute-stage HLSL target.
- **`collectCooperativeMetadata`**. Requires the
  `cooperative_matrix` / `cooperative_vector` capability set.
- **`profile.getVersion() <= DX_5_0` byte-address-buffer
  flag** (`#legalizebyteaddressbufferops-for-hlsl`). Setting
  `useBitCastFromUInt=true` requires `-profile sm_5_0`,
  which routes through fxc — out of scope.
- **`floatNonUniformResourceIndex`**. The
  `NonUniformResourceIndex(...)` intrinsic is a
  bindless-resource feature whose natural source surface
  is a graphics pipeline.
- **`coverageTracing`-gated passes**
  (`instrumentCoverage`,
  `finalizeCoverageInstrumentationMetadata`). Coverage
  instrumentation is a debugging flag, not user-observable
  through text emit.
- **`autodiff` / `higherOrderFunc` passes**
  (`checkAutodiffPatterns`,
  `specializeHigherOrderParameters`,
  `finalizeAutoDiffPass`, etc.). The doc anchors them to
  Phase B but the emit-stage observable is a downstream
  language feature; covered by other bundles.
- **`dynamicResourceHeap`**. Requires the SM 6.6 dynamic
  resource heap setup.
- **Pass-ordering claims** (Phase A passes 1-20, Phase B
  passes 1-63, Phase C passes 1-31). The doc enumerates the
  ordered list; pass _existence_ is observable through emit
  side effects, but pass _ordering_ would require
  `-dump-ir` cross-pass comparison without doc-anchored
  ordering markers. Covered by `pipeline/05-ir-passes`.
