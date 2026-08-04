---
generated: true
model: claude-opus-4-8[1m]
generated_at: 2026-06-11T15:00:00Z
source_commit: ef1068b5485e09b3a7afadba2e25f9541e29af42
watched_paths_digest: 7c8fbec89ad4217847f03ab88cd9895b0e9ec0d88dc602baa08bac9cb716a0e1
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for coverage/emit

## Intent

White-box characterization tests for the target back-end emitters
`source/slang/slang-emit-c-like.cpp` (~62% covered), `slang-emit-glsl.cpp`
(~50%), and `slang-emit-spirv.cpp` (~60%). They pin the **current observed
emitted text** of CLI-reachable, less-common emit arms, not a spec.
Strategy: pick constructs whose lowering _diverges per target_ (so a
single shader exercises arms in all three files at once) and pin each
target's distinguishing token verbatim. Targeted arms: the
`MakeMatrixFromScalar` element fan-out (`slang-emit-c-like.cpp:2436`),
the bitfield-intrinsic split (native SPIR-V `OpBitField*` vs GLSL
`bitfield*` vs HLSL shift/mask arithmetic), matrix-times-vector `mul`
operand reordering for column-major GLSL, region-based `switch` emission
with shared case labels and fall-through (`emitSwitchCaseSelectorsImpl`,
`:3555`/`:3718`), matrix-narrowing reshape (row `.xyz` truncation), and
struct-by-value return / `FieldExtract` (`:2486`).

Observation points follow each construct's stable contract. The codegen
_shape_ claims (matrix constructors, bitfield ops, `mul` order, switch
labels, reshape) are pinned with `SIMPLE` emit and FileCheck across every
feasible text target the divergence is visible on — these report
`ignored` locally (FileCheck absent) and validate in CI. The value-level
companion (struct round-trip) is pinned with `INTERPRET`, which validates
locally on slangi.

All emitted tokens were copied verbatim from the local `slangc` /
`slangi` at `source_commit`. The behaviours are well-formed and
self-consistent across targets, so none carry
`characterization-unverified=true`.

## Functional coverage

| Test                                                                             | What it pins (current behaviour)                                                                                                                                                                  | covers=                            |
| -------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------- |
| [`bitfield-extract-insert.slang`](bitfield-extract-insert.slang)                 | `bitfieldExtract`/`bitfieldInsert` emit native `OpBitFieldUExtract`/`OpBitFieldInsert` on SPIR-V, GLSL `bitfieldExtract`/`bitfieldInsert`, and expanded shift/mask arithmetic on HLSL/Metal/WGSL. | source/slang/slang-emit-spirv.cpp  |
| [`make-matrix-from-scalar-fanout.slang`](make-matrix-from-scalar-fanout.slang)   | `float3x3(scalar)` fans the scalar out to 9 constructor args on HLSL/GLSL/Metal/WGSL but emits a single-arg `makeMatrix<float,3,3>` on CUDA.                                                      | source/slang/slang-emit-c-like.cpp |
| [`matrix-reshape-row-truncation.slang`](matrix-reshape-row-truncation.slang)     | `(float3x3)float4x4` emits a constructor of the first three rows each `.xyz`-truncated, on HLSL/GLSL/Metal/WGSL.                                                                                  | source/slang/slang-emit-c-like.cpp |
| [`matrix-vector-mul-operand-order.slang`](matrix-vector-mul-operand-order.slang) | `mul(M, v)` stays `mul(M, v)` on HLSL, reverses to `v * M` on GLSL/Metal/WGSL (column-major), and emits `OpVectorTimesMatrix` on SPIR-V.                                                          | source/slang/slang-emit-glsl.cpp   |
| [`struct-return-value-roundtrip.slang`](struct-return-value-roundtrip.slang)     | A by-value struct returned from a helper and read field-by-field round-trips its fields (`makePair(7)` → 7+14=21) under the interpreter.                                                          | source/slang/slang-emit-c-like.cpp |
| [`switch-fallthrough-shared-case.slang`](switch-fallthrough-shared-case.slang)   | Adjacent empty cases share one label; a non-`break` case falls through; HLSL duplicates the default body inline while SPIR-V keeps one `OpSwitch` with 0 and 1 mapped to the same case label.     | source/slang/slang-emit-c-like.cpp |

## Unreachable gaps

- **`MakeMatrixFromScalar` / `MatrixReshape` SPIR-V arm** is not pinned:
  under SPIR-V the matrix is built/extracted by `OpCompositeExtract` /
  `OpCompositeConstruct` rather than passing through the c-like
  constructor fan-out, so the SPIR-V emit does not exercise the same arm
  the HLSL/GLSL/Metal/WGSL/CUDA constructor shapes do; only the
  constructor-style targets are pinned for those two tests. (This is the
  source of the lint's "missing spirv-asm" warning on those two claims —
  the construct genuinely takes a different SPIR-V path.)
- **`switch` fall-through shape on GLSL / Metal / WGSL** differs from
  both pinned targets: those emitters produce a natural C fall-through
  (case 2 has no `break` and drops into `default` with the body emitted
  once), whereas HLSL duplicates the default body inline and SPIR-V uses
  a structured `OpSwitch`. The switch test pins only HLSL (body-dup) and
  SPIR-V (`OpSwitch` shared label) because those two tokens are the most
  stable; the GLSL/Metal/WGSL fall-through is recorded here rather than
  pinned (source of the lint's "missing glsl/metal/wgsl" warning).
- **`AllocObj` / `MakeUInt64` arms** (`slang-emit-c-like.cpp:2455`,
  `:2460`): reachable only via CPU/CUDA host-side `class` construction or
  the internal 64-bit-pack helper, neither of which a plain compute
  `.slang` drives without target-gated plumbing; not targeted here.
- **`SLANG_RELEASE_ASSERT(matrixType)` / `columnCount` / `rowCount`** in
  the `MakeMatrixFromScalar` arm are defensive: the inst's data type is
  always a matrix by construction when this arm runs, so the asserts are
  not reachable from valid CLI input.
- **GLSL `switch` fall-through shape** differs from HLSL (GLSL emits a
  natural C fall-through into `default`, no duplicated body) but is not
  separately pinned in the switch test to keep that test's CHECKs to the
  two targets whose tokens are most stable (HLSL labels, SPIR-V
  `OpSwitch`); recorded here as an observed divergence.
- **Downstream-tool-gated targets** (`-target dxil`→dxc, `-target
cuda`→nvrtc runtime, metal/wgsl toolchains) emit source text that
  FileCheck matches locally; only genuine tool steps are CI-only. No
  test here needs a runtime, so none carry `requires-tool`.

## Doc gaps observed

NA
