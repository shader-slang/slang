---
generated: true
model: claude-opus-5[1m]
generated_at: 2026-08-05T00:00:00Z
source_commit: 5634a0ea1b
watched_paths_digest: 7c8fbec89ad4217847f03ab88cd9895b0e9ec0d88dc602baa08bac9cb716a0e1
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for coverage/emit

## Intent

White-box characterization tests for the target back-end emitters
`source/slang/slang-emit-c-like.cpp`, `slang-emit-glsl.cpp` and
`slang-emit-spirv.cpp`. They pin the **current observed emitted text** of
CLI-reachable, otherwise-untested emit arms, not a spec.

The first pass (six tests, 2026-06-11) picked constructs whose lowering _diverges
per target_ so one shader exercises arms in all three files at once: the
`MakeMatrixFromScalar` element fan-out, the bitfield-intrinsic split, matrix-times-vector
`mul` operand reordering, region-based `switch` emission with shared case labels,
matrix-narrowing reshape, and struct-by-value return / `FieldExtract`.

The second pass (seven tests, 2026-08-05) went after the gaps that survived the
whole shipped suite. Its strategy was different: instead of divergence, it targets
**translation tables and legalization rewrites that only a fan of inputs makes
observable** — one declaration per table row, or one operator per legalization arm,
in a single shader. Measured against the full shipped suite (see the note below),
the seven new tests move `slang-emit-glsl.cpp` from 78.69% to 81.01% line coverage
and `slang-emit-spirv.cpp` from 87.28% to 88.45%, 188 previously-unexecuted lines in
total.

**Note on the coverage figures used to select these tests.** The profile the
second-pass sweep was handed (`build/cov-agentic-clean/slang-test.profdata`,
reported as glsl 56.03% / c-like 80.48% / spirv 73.68%) is **incomplete**: it was
collected through `slang-test -use-test-server`, and coverage from the test-server
child processes is largely lost, so it under-reports what `tests/` already covers.
Several arms it showed as untouched are in fact exercised by hand-written tests —
GLSL image atomics by `tests/bugs/vk-image-atomics.slang`, ray query by
`tests/glsl-intrinsic/raytracing/glsl-rayCompute.slang`, conservative depth by
`tests/glsl/fragment-depth-greater-less.slang`, quad control by
`tests/hlsl-intrinsic/quad-control/`, `packoffset` by `tests/hlsl/packoffset.slang`,
mesh-shader GLSL layout by `tests/diagnostics/mesh-shader-invalid-output-topology.slang`,
and the `[format("bgra8")]` warning by
`tests/diagnostics/image-format-unsupported-by-backend.slang`. Candidate tests were
therefore re-screened against a rebuilt reference profile (that profile, plus a
per-directory in-process run of the whole of `tests/`), which reads glsl 78.69% /
c-like 83.03% / spirv 87.28%; only candidates that still executed unexecuted lines
against **that** reference were kept. Two drafted tests (GLSL image atomics, and the
explicit `unknown`/`r64ui` image-format arms) were measured at zero new lines and
deleted rather than committed.

All emitted tokens were copied verbatim from the local `slangc` / `slangi` at
`source_commit`. Where a target's current output is believed wrong it is
deliberately **not** pinned, and a finding is filed instead — see
`## Unreachable gaps`. No test here carries `characterization-unverified=true`.

## Functional coverage

| Test                                                                                                     | What it pins (current behaviour)                                                                                                                                                                                                                                                                                                                                                                             | covers=                            |
| -------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ | ---------------------------------- |
| [`bitfield-extract-insert.slang`](bitfield-extract-insert.slang)                                         | `bitfieldExtract`/`bitfieldInsert` emit native `OpBitFieldUExtract`/`OpBitFieldInsert` on SPIR-V, GLSL `bitfieldExtract`/`bitfieldInsert`, and expanded shift/mask arithmetic on HLSL/Metal/WGSL.                                                                                                                                                                                                            | source/slang/slang-emit-spirv.cpp  |
| [`glsl-bool-vector-logical-legalization.slang`](glsl-bool-vector-logical-legalization.slang)             | `&&`, `\|\|` and `^` on `bool3` are legalized for GLSL by round-tripping both operands through `uvec3` (`bvec3(uvec3(a)&uvec3(b))`), while SPIR-V uses `OpLogicalAnd`/`OpLogicalOr`/`OpLogicalNotEqual` on `%v3bool`, HLSL/Metal keep the operator on the boolean vector, and WGSL lowers the short-circuiting forms to `select`.                                                                            | source/slang/slang-emit-glsl.cpp   |
| [`glsl-image-format-inference-by-element-type.slang`](glsl-image-format-inference-by-element-type.slang) | With no `[format(...)]`, the GLSL format is inferred from the element type: component count picks `r`/`rg`/`rgba` (3 components widen to `rgba`) and the base type picks `32f`/`16f`/`32ui`/`32i`/`16i`/`8ui`, with a matching `image2D`/`uimage2D`/`iimage2D`/`i16image2D`/`u8image2D` type name.                                                                                                           | source/slang/slang-emit-glsl.cpp   |
| [`glsl-interpolation-modifiers.slang`](glsl-interpolation-modifiers.slang)                               | On a fragment varying, `nointerpolation` becomes GLSL `flat` and `linear` becomes `smooth`, while `noperspective`/`centroid`/`sample` keep their names; SPIR-V uses `Flat`/`NoPerspective`/`Centroid`/`Sample` decorations and emits none for `linear`; HLSL round-trips all five.                                                                                                                           | source/slang/slang-emit-glsl.cpp   |
| [`glsl-structured-buffer-data-layouts.slang`](glsl-structured-buffer-data-layouts.slang)                 | `Std140DataLayout`/`Std430DataLayout`/`ScalarDataLayout` select `layout(std140)`/`layout(std430)`/`layout(scalar)` (the last also requiring `GL_EXT_scalar_block_layout`), a read-only `StructuredBuffer` gets `readonly`, and `globallycoherent` becomes `coherent`; SPIR-V carries the same information as `ArrayStride`/`Offset` decorations plus `OpDecorate ... Coherent`.                              | source/slang/slang-emit-glsl.cpp   |
| [`glsl-track-liveness-spirv-intrinsics.slang`](glsl-track-liveness-spirv-intrinsics.slang)               | `-track-liveness` on the GLSL target declares one `spirv_instruction(id = 256)` / `(id = 257)` function per marked local type (`spirv_by_reference` + `spirv_literal` parameters) and calls `livenessStart_N`/`livenessEnd_N` at the range boundaries.                                                                                                                                                       | source/slang/slang-emit-glsl.cpp   |
| [`make-matrix-from-scalar-fanout.slang`](make-matrix-from-scalar-fanout.slang)                           | `float3x3(scalar)` fans the scalar out to 9 constructor args on HLSL/GLSL/Metal/WGSL, a nested `OpCompositeConstruct` on SPIR-V, and a single-arg `makeMatrix<float,3,3>` on CUDA.                                                                                                                                                                                                                           | source/slang/slang-emit-c-like.cpp |
| [`matrix-reshape-row-truncation.slang`](matrix-reshape-row-truncation.slang)                             | `(float3x3)float4x4` emits a constructor of the first three rows each `.xyz`-truncated on HLSL/GLSL/Metal/WGSL, and the same shape as three `OpVectorShuffle`s + `OpCompositeConstruct` on SPIR-V.                                                                                                                                                                                                           | source/slang/slang-emit-c-like.cpp |
| [`matrix-vector-mul-operand-order.slang`](matrix-vector-mul-operand-order.slang)                         | `mul(M, v)` stays `mul(M, v)` on HLSL, reverses to `v * M` on GLSL/Metal/WGSL (column-major), and emits `OpVectorTimesMatrix` on SPIR-V.                                                                                                                                                                                                                                                                     | source/slang/slang-emit-glsl.cpp   |
| [`spirv-image-format-table.slang`](spirv-image-format-table.slang)                                       | Each of 27 otherwise-undeclared `[format("...")]` attributes selects its SPIR-V image format in `OpTypeImage` (including the `Rgb10A2` vs `Rgb10a2ui` capitalisation difference), and the module declares `StorageImageExtendedFormats`.                                                                                                                                                                     | source/slang/slang-emit-spirv.cpp  |
| [`spirv-unorm-snorm-image-format-by-width.slang`](spirv-unorm-snorm-image-format-by-width.slang)         | A `unorm`/`snorm` storage image with no `[format(...)]` infers an 8-bit normalized SPIR-V format sized by component count (`R8`/`Rg8`/`Rgba8` and the `Snorm` variants), with 3 components widening onto the 4-component format.                                                                                                                                                                             | source/slang/slang-emit-spirv.cpp  |
| [`struct-return-value-roundtrip.slang`](struct-return-value-roundtrip.slang)                             | A by-value struct returned from a helper and read field-by-field round-trips its fields (`makePair(7)` → 7+14=21) under the interpreter.                                                                                                                                                                                                                                                                     | source/slang/slang-emit-c-like.cpp |
| [`switch-fallthrough-shared-case.slang`](switch-fallthrough-shared-case.slang)                           | Adjacent empty cases share one label; a non-`break` case falls through. Three shapes are pinned: HLSL and WGSL duplicate the default body inline (WGSL also merges the shared cases into one comma-separated selector list and reports E41026), GLSL and Metal keep a natural C fall-through with the default body emitted once, and SPIR-V keeps one `OpSwitch` with 0 and 1 mapped to the same case label. | source/slang/slang-emit-c-like.cpp |

## Untested claims

Per-target rows recording the shader text targets an emission claim is not
fanned out to (`_claims.md` §2 "Meaningful back-ends"). Every other
target/claim pair carries a real `//TEST:SIMPLE` directive pinned to output
copied verbatim from `slangc`; nothing here was opted out to make a directive
green. The Claim cell repeats the test's `//META: purpose` verbatim.

Two distinct situations share the `unsupported-on-target` reason token, and the
Why cell always says which applies: the target has no way to express the claim
at all, or the target does express it and its current output is believed wrong,
in which case the row names the finding rather than pinning the bad output.

| Claim                                                                                                                                                                                                                                   | Reason                | Anchor                                                    | Why untested                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              |
| --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | --------------------- | --------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Pins the GLSL format that is inferred for an `RWTexture2D` with no `[format(...)]` attribute, across component counts 1/2/3/4 and element types float/half/uint/int/int16/uint8.                                                        | unsupported-on-target | [#backends](../../../design/pipeline/06-emit.md#backends) | Absent targets: hlsl , metal , wgsl . Cannot express: neither hlsl nor metal carries a storage-image format at all — they emit `RWTexture2D<float>` and `texture2d<float, access::read_write>` — so there is no token to assert; wgsl cannot compile the declarations, failing on the `int16_t4` and `uint8_t4` rows with E41400 ("static assertion failed") and E38203 ("disallowed vector element type"). glsl and spirv-asm both carry directives, and the SPIR-V one pins that this target infers nothing for the 32-bit float rows.                                  |
| Pins the GLSL spelling of each HLSL interpolation modifier on a fragment varying input -- `nointerpolation` becomes `flat`, `linear` becomes `smooth`, and `noperspective`/`centroid`/`sample` keep their names.                        | unsupported-on-target | [#backends](../../../design/pipeline/06-emit.md#backends) | Absent targets: metal , wgsl . Believed-wrong output, not inexpressible: metal maps `linear` to `[[sample_no_perspective]]` and `centroid` to `[[center_perspective]]`, changing sampling behaviour rather than spelling (`_meta/findings/metal-interpolation-modifier-linear-centroid-mismapped.yaml`), and wgsl drops `centroid`/`sample` entirely and maps `linear` onto `@interpolate(linear)` (`_meta/findings/wgsl-interpolation-sampling-qualifier-dropped.yaml`). Pinning either would codify the bug. glsl, hlsl and spirv-asm carry directives.                 |
| Pins that the explicit `Std140DataLayout` / `Std430DataLayout` / `ScalarDataLayout` arguments of `StructuredBuffer` select the matching GLSL block layout qualifier, and that `ScalarDataLayout` pulls in `GL_EXT_scalar_block_layout`. | unsupported-on-target | [#backends](../../../design/pipeline/06-emit.md#backends) | Absent targets: hlsl , metal , wgsl . Cannot express: all three reject the program outright with E36107 ("unavailable features in entry point", noting the use of `Std140DataLayout`), so no text is emitted to check. glsl and spirv-asm carry directives.                                                                                                                                                                                                                                                                                                               |
| Pins that `-track-liveness` on the GLSL target turns live-range markers into declared `spirv_instruction` functions (`livenessStart_N` / `livenessEnd_N`, ids 256 and 257) called around each local's live range.                       | unsupported-on-target | [#backends](../../../design/pipeline/06-emit.md#backends) | Absent targets: hlsl , metal , spirv-asm , wgsl . Mixed: hlsl, metal and wgsl emit no liveness marker of any kind under `-track-liveness`, so the claim has no form there; spirv-asm is believed-wrong rather than inexpressible — the same flag makes it emit a 5-word `OpLifetimeStart` where the grammar allows 3 and slangc exits 255 (`_meta/findings/spirv-track-liveness-malformed-oplifetimestart.yaml`). glsl carries the directive.                                                                                                                             |
| Pins the SPIR-V image-format name that each `[format("...")]` attribute selects, for the 27 formats that no other test in the suite declares.                                                                                           | unsupported-on-target | [#backends](../../../design/pipeline/06-emit.md#backends) | Absent targets: hlsl , metal , wgsl . Mixed: hlsl and metal drop the format from the declaration entirely, so there is nothing to assert; wgsl is believed-wrong — it has no spelling for 19 of the 27 formats and substitutes the float format `rgba32float` for all of them, integer formats included, which leaves `textureLoad` yielding `f32` into an `i32`/`u32` variable (`_meta/findings/wgsl-unsupported-int-image-format-falls-back-to-float.yaml`). spirv-asm and glsl carry directives.                                                                       |
| Pins that a `unorm`/`snorm` storage image with no `[format(...)]` infers an 8-bit SPIR-V format sized by the element's component count, with the 3-component case widening to the 4-component format.                                   | unsupported-on-target | [#backends](../../../design/pipeline/06-emit.md#backends) | Absent targets: glsl , hlsl , metal , wgsl . Mixed: glsl is believed-wrong — it collapses `unorm float4` and `snorm float4` onto the same `layout(rgba32f)` while SPIR-V keeps them apart (`_meta/findings/glsl-unorm-snorm-image-format-collapsed.yaml`). Cannot express: hlsl has no storage-image format and round-trips the modifier on the element type instead (`RWTexture2D<unorm float>`), metal drops the modifier as well as the format (`texture2d<float, access::read_write>`), and wgsl fails the declarations with E41400. spirv-asm carries the directive. |

## Unreachable gaps

Triage of what was left alone, and why. "Shipped reference" below means the
rebuilt profile described in `## Intent` (operator profile + a full in-process run
of `tests/`), not the incomplete profile handed to the sweep.

### Dead or unreachable from the slangc / slangi CLI

- **`GLSLSourceEmitter::_emitGLSLByteAddressBuffer` (`slang-emit-glsl.cpp:457`) is
  dead for the GLSL target.** `slang-emit.cpp:2072` sets
  `byteAddressBufferOptions.translateToStructuredBufferOps = true` unconditionally
  for GLSL, so an `IRByteAddressBufferTypeBase` never survives to
  `tryEmitGlobalParamImpl`. Confirmed empirically: a `ByteAddressBuffer` used by
  `Load`/`Store` **and** one used only by `GetDimensions` both emit as
  `buffer StructuredBuffer_uint_t_N { uint _data[]; }`, never as the `_S<n>` block
  this function writes. Candidate for removal rather than for a test.
- **`GLSLSourceEmitter::_requireFragmentShaderBarycentric` (`:155`) is superseded.**
  Its two call sites are the `IRInterpolationMode::PerVertex` arms at `:3856`
  and `:3904`, but `slang-ir-glsl-legalize` rewrites a `pervertex` fragment input
  into an `IRPerVertexDecoration`, which is handled at `:3975` by calling
  `_requireGLSLExtension("GL_EXT_fragment_shader_barycentric")` directly. A
  `GetAttributeAtVertex` shader emits `pervertexEXT` and the extension line while
  leaving `:156`–`:159` unexecuted. Duplicate logic; candidate for removal.
- **`CLikeSourceEmitter::emitType(IRType*, Name*)` and its two siblings
  (`slang-emit-c-like.cpp:655`, `:673`, `:684`) have no callers** anywhere under
  `source/`. Dead code.
- **`CLikeSourceEmitter::emitInterface` / `emitRTTIObject` (`:584`, `:591`) are
  no-op base stubs** whose only real implementations are the `CPPSourceEmitter`
  overrides; the base bodies run only if an `IRInterfaceType` / `IRRTTIObject`
  reached emit on a target that does not override them, which no CLI input produces.
- **`CLikeSourceEmitter::emitLivenessImpl` (`:690`) is not reachable via
  `-track-liveness`.** On the Khronos targets `applyGLSLLiveness`
  (`slang-emit.cpp:2603`) rewrites the markers into intrinsic calls before emit —
  that is what `glsl-track-liveness-spirv-intrinsics.slang` pins — and on `cpp`,
  `cuda` and `hlsl` the same flag (with and without `-O1`) emits no `SLANG_LIVE_START`
  / `SLANG_LIVE_END` at all.
- **`CLikeSourceEmitter::emitStringLiteral`'s escape switch (`:800`–`:828`) is
  effectively unreachable.** The function is only called for Torch/PyTorch function
  names (`slang-emit-torch.cpp`) and HLSL decoration names (`slang-emit-hlsl.cpp`),
  none of which can contain a tab, newline, quote or backslash. `printf` format
  strings do **not** go through it — a `printf` with `\t`, `\n`, `\"`, `\\` and `\r`
  emits correctly on `-target cpp` while leaving every one of those lines unexecuted.
- **`CLikeSourceEmitter::getSortedWitnessTableEntries` (`:371`)** is called only from
  `CPPSourceEmitter::_emitWitnessTableDefinitions` (`slang-emit-cpp.cpp:662`). A
  `ConstantBuffer<IShape>` with two `-conformance` registrations compiles to `-target cpp`
  without emitting any witness-table definition, so no plain compute `.slang` reaches it.
- **Defensive `SLANG_DIAGNOSE_UNEXPECTED` / `SLANG_UNREACHABLE` arms** in
  `emitSimpleTypeImpl` ("unhandled sampler state flavor", "structured buffer type used
  unexpectedly", "unhandled buffer type"), `_emitGLSLTypePrefix` ("unhandled GLSL type
  prefix"), `emitDeclaratorImpl` ("unknown declarator flavor") and
  `getSortedWitnessTableEntries` ("interface requirement key not found") assert
  states that valid CLI input cannot produce. Not targeted.
- **`_emitGLSLTypePrefix` / `emitSimpleTypeImpl` `IntPtrType` / `UIntPtrType` arms**
  (`:1105`, `:1137`, `:3569`, `:3582`) would need a storage image or GLSL type whose
  element type is a pointer-sized integer; no surface syntax produces one.

### Believed-wrong output — filed as findings, deliberately not pinned

- **Metal interpolation modifiers.** `_getInterpolationModifierText`
  (`slang-emit-metal.cpp:1726`) maps `linear` to `[[sample_no_perspective]]` and
  `centroid` to `[[center_perspective]]`; both change sampling behaviour rather than
  spelling. `glsl-interpolation-modifiers.slang` therefore omits `-target metal`.
  See `_meta/findings/metal-interpolation-modifier-linear-centroid-mismapped.yaml`.
- **WGSL interpolation modifiers.** `emitInterpolationModifiersImpl`
  (`slang-emit-wgsl.cpp:1842`) drops `centroid`/`sample` entirely (the emit is gated
  on an interpolation _type_ also being present) and maps `linear` onto WGSL's
  non-perspective `@interpolate(linear)`. Same test omits `-target wgsl`.
  See `_meta/findings/wgsl-interpolation-sampling-qualifier-dropped.yaml`.
- **GLSL `unorm`/`snorm` storage-image format.** The GLSL path collapses both
  `unorm float4` and `snorm float4` onto `layout(rgba32f)` while SPIR-V keeps them
  apart, so `spirv-unorm-snorm-image-format-by-width.slang` pins SPIR-V only.
  Already filed by a sibling bundle as
  `_meta/findings/glsl-unorm-snorm-image-format-collapsed.yaml`.
- **`-track-liveness` produces an invalid module on direct SPIR-V.** Any shader with a
  tracked local emits an `OpLifetimeStart` of 5 words where the SPIR-V grammar allows
  3, and `spirv-opt` rejects it (slangc exits 255, no output). `applyGLSLLiveness`
  (`slang-emit.cpp:2603`) is gated on `isKhronosTarget`, which is true for SPIR-V as
  well as GLSL, but the pass rewrites markers into the `spirv_instruction` function
  calls that only the GLSL text path consumes. No shipped test passes the flag on any
  target, which is consistent with the breakage surviving. This is why the claim above
  opts spirv-asm out instead of pinning it. See
  `_meta/findings/spirv-track-liveness-malformed-oplifetimestart.yaml`.
- **WGSL substitutes a float format for unsupported integer image formats.** 19 of the
  27 formats in `spirv-image-format-table.slang` have no WGSL spelling and all fall back
  to `rgba32float` with a warning (E31105), including the integer ones, so an
  `RWTexture2D<int>` becomes `texture_storage_2d<rgba32float, read_write>` and its
  `textureLoad` yields `f32` into a variable declared `i32`. The `default:` arm at
  `slang-emit-wgsl.cpp:477` returns the literal `"rgba32float"` without consulting the
  format's scalar class, though the same switch already spells `rgba32sint` and
  `rgba32uint`. See
  `_meta/findings/wgsl-unsupported-int-image-format-falls-back-to-float.yaml`.
- **`Optional<T*>` returned across a function boundary never finishes compiling on
  SPIR-V.** Found while probing `SPIRVEmitContext::emitCastPtrToBool`
  (`slang-emit-spirv.cpp:9554`), the lowering of the `CastPtrToBool` inst that
  `slang-ir-lower-optional-type.cpp:226` produces; that function is still uncovered
  by the whole shipped suite and cannot be covered until the hang is fixed.
  See `_meta/findings/spirv-optional-pointer-return-hangs.yaml`.
- **A `uniform` global of interface type plus `-conformance` aborts with an internal
  error** on every target, while the `ConstantBuffer<IShape>` spelling of the same
  program compiles. Observed while probing the C++ witness-table path.
  See `_meta/findings/global-interface-param-uniform-parameter-info-ice.yaml`.

### Targets a committed test does not exercise

Per-claim rows with the exact absent targets are in `## Untested claims` above;
this list records only the ones whose reason needs more than a table cell.

- **`glsl-image-format-inference-by-element-type.slang` now pins SPIR-V too.** An
  earlier note here claimed SPIR-V "does not run this inference"; measuring it while
  closing the fan-out backlog showed that is wrong. SPIR-V infers `Rgba16f`, `Rg32ui`,
  `R32i`, `Rgba16i` and `Rgba8ui` for the half/uint/int/int16/uint8 rows, and differs
  from GLSL only on the 32-bit float rows, where `float`, `float2` and `float3` all
  share one `OpTypeImage ... Unknown` instead of GLSL's `r32f`/`rg32f`/`rgba32f`. That
  divergence is now pinned on both targets rather than assumed away.
- **`spirv-image-format-table.slang` now pins GLSL too.** All 27 formats have a GLSL
  `layout(...)` spelling and emit cleanly, so the earlier "no counterpart in those
  languages" applied only to HLSL and Metal, which drop the format outright.
- **`glsl-track-liveness-spirv-intrinsics.slang` stays GLSL-only**, but not purely for
  the reason given in the dead-code list above: HLSL, Metal and WGSL emit no liveness
  marker, while direct SPIR-V does attempt one and produces an invalid module. See the
  findings list below.
- No test in this bundle needs a GPU, DXC, nvrtc or the Apple toolchain, so none
  carries `requires-tool`; every directive runs and passes locally.

### Observations recorded but not turned into tests

- **GLSL mesh shaders emit `layout(max_primitives = N) out;` twice.** A mesh entry
  point with both `out indices` and `out primitives` parameters produces the
  qualifier line twice. `glslang` accepts it (`-emit-spirv-via-glsl` succeeds), so
  it is redundant rather than invalid; it is not pinned, because pinning the
  duplicate would make removing it look like a regression.

## Doc gaps observed

NA
