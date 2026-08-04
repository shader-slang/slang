---
generated: true
model: claude-opus-4-8[1m]
generated_at: 2026-06-11T17:00:00Z
source_commit: ef1068b5485e09b3a7afadba2e25f9541e29af42
watched_paths_digest: 0a2aa024299dd826ce0f7a6153336a696d3348a0c9cb798df8e2fc61b3e42130
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for coverage/type-layout

## Intent

White-box characterization tests for `source/slang/slang-type-layout.cpp`
(~57% covered). They pin the **current observed byte offsets, strides, and
SPIR-V layout decorations** the layout rules produce for the less-trivial
uniform-block scenarios, not a spec. Strategy: pick struct/array/matrix shapes
whose layout _diverges across layout-rule sets_ — std140 (`ConstantBuffer`) vs
std430 (`RWStructuredBuffer`), the HLSL constant-buffer tail-packing rules
(`-fvk-use-dx-layout`), scalar layout (`-force-glsl-scalar-layout`), push
constant vs `ParameterBlock` — so a single shader exercises two rule arms at
once, and pin each arm's distinguishing offset/stride/decoration verbatim.

Observation point is the SPIR-V assembly emit (`-target spirv-asm`), where the
layout result is materialized as `OpMemberDecorate ... Offset N`,
`OpDecorate ... ArrayStride N`, `MatrixStride`, `RowMajor`, the storage class
of the block variable, and the descriptor-set assignment. SPIR-V is the only
text target that prints explicit member byte offsets (GLSL/HLSL emit relies on
implicit std140/packing), so it is the stable contract for these claims. The
emitted Slang type-name suffix (`S_std140` / `S_std430` / `S_default` /
`S_natural`) additionally confirms which rule set was selected. These
`SIMPLE` filecheck directives report `ignored` locally (FileCheck absent) and
validate in CI.

All offsets, strides, decorations, and type-name suffixes were copied verbatim
from the local `slangc` at `source_commit`; every shader compiles `slangc`
exit 0. The observed layouts match the std140 / std430 / D3D constant-buffer /
scalar rules these code paths implement and the inline source comments
(notably the `float3 a[2]; float b;` tail-packing example in
`HLSLConstantBufferLayoutRulesImpl`), so none carry
`characterization-unverified=true`.

## Functional coverage

| Test                                                                                               | What it pins (current behaviour)                                                                                                                                                                                                     | covers=                            |
| -------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ | ---------------------------------- |
| [`cbuffer-vs-structuredbuffer-array-stride.slang`](cbuffer-vs-structuredbuffer-array-stride.slang) | A `float[4]` array gets std140 ArrayStride 16 with the trailing field at offset 64 inside `ConstantBuffer<S>`, but std430 ArrayStride 4 with the trailing field at offset 16 (struct ArrayStride 20) inside `RWStructuredBuffer<S>`. | source/slang/slang-type-layout.cpp |
| [`matrix-stride-cbuffer-std140.slang`](matrix-stride-cbuffer-std140.slang)                         | A `float2x3` in a `ConstantBuffer` is decorated `RowMajor` with `MatrixStride 16`, so a following scalar field lands at offset 48.                                                                                                   | source/slang/slang-type-layout.cpp |
| [`nested-struct-tail-packing-std140-vs-dx.slang`](nested-struct-tail-packing-std140-vs-dx.slang)   | A field after a nested `float3[2]` lands at offset 32 under std140 (struct size rounds up to alignment) but at offset 28 under `-fvk-use-dx-layout` (tail packing into the array's trailing 4 bytes).                                | source/slang/slang-type-layout.cpp |
| [`push-constant-vs-parameter-block-layout.slang`](push-constant-vs-parameter-block-layout.slang)   | A `vk::push_constant` block uses std430 element layout in the `PushConstant` storage class, while a `ParameterBlock` uses std140 element layout in the `Uniform` storage class on its own descriptor set (set 1).                    | source/slang/slang-type-layout.cpp |
| [`vector-alignment-std140-vs-scalar.slang`](vector-alignment-std140-vs-scalar.slang)               | A `float3`/`float2`/`float` struct lays out at offsets 0/16/24 under std140 vector power-of-two alignment but 0/12/20 under scalar layout's natural packing.                                                                         | source/slang/slang-type-layout.cpp |

## Untested claims

| Claim                                                                                                                                                                                          | Reason               | Anchor                                                                                          | Why untested                                                                                                                                                                                                                                                                                                     |
| ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------- | ----------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| The CUDA / CPU / LLVM / Metal layout-rule families (`CUDALayoutRulesImpl`, `CPULayoutRulesImpl`, `LLVMLayoutRulesImpl`, `MetalLayoutRulesImpl`) compute their own struct/array/matrix layouts. | needs-cli-test       | [#global-scope-type-layout](../../../design/pipeline/04c-layout-ir.md#global-scope-type-layout) | The CUDA/C++ text targets do not emit explicit byte-offset annotations the way SPIR-V `OpMemberDecorate Offset` does; observing these layouts requires the `-reflection-json` sidecar file, which a `//TEST` directive cannot FileCheck. A wrapper CLI test diffing `-reflection-json` output would verify them. |
| `GLSLSpecializationConstantLayoutRulesImpl` lays out `[[vk::constant_id]]` specialization constants.                                                                                           | needs-cli-test       | [#global-scope-type-layout](../../../design/pipeline/04c-layout-ir.md#global-scope-type-layout) | Specialization-constant layout is exposed through reflection, not as member offsets in emitted SPIR-V text; a reflection-json wrapper test would verify it.                                                                                                                                                      |
| The `spvBindlessTextureNV` descriptor-handle layout arm (`DescriptorHandle<T>` as `uint64_t`).                                                                                                 | gpu-vulkan-extension | [#global-scope-type-layout](../../../design/pipeline/04c-layout-ir.md#global-scope-type-layout) | Requires enabling the `spvBindlessTextureNV` capability/extension, not in this bundle's default target caps.                                                                                                                                                                                                     |

## Doc gaps observed

| Anchor                                                                                          | Kind                  | Gap                                                                                                                                                                                                                                                                                                                                                                                                                     | Suggested addition                                                                                                                                                                                                                                                                                                                                                                |
| ----------------------------------------------------------------------------------------------- | --------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| [#global-scope-type-layout](../../../design/pipeline/04c-layout-ir.md#global-scope-type-layout) | undocumented-behavior | The doc describes layout-IR module construction but does not state the concrete byte-offset consequences a reader can observe: that `ConstantBuffer` uses std140, `RWStructuredBuffer` uses std430, push constants use std430, and `ParameterBlock` uses std140, nor that `-fvk-use-dx-layout` / `-force-glsl-scalar-layout` swap in the D3D / scalar rule sets with different array-stride and tail-packing behaviour. | Add a short table mapping each container kind (`ConstantBuffer`, `RWStructuredBuffer`, `[[vk::push_constant]]`, `ParameterBlock`) and each layout flag (`-fvk-use-dx-layout`, `-force-glsl-scalar-layout`) to the layout rule it selects, with one worked offset example (e.g. the `float3 a[2]; float b;` tail-packing case that differs 32 vs 28 between std140 and D3D rules). |

## Unreachable gaps

- **`default:` arms and `SLANG_UNEXPECTED` paths** in the layout-rule
  dispatch (e.g. `getSimpleLayoutImpl` / `GetObjectLayout` switches over
  `ShaderParameterKind` / `BaseType`) assert on shapes the front-end has
  already validated; they are defensive and not reachable from well-formed
  CLI input, so they are not targeted.
- **`GetPointerLayout` for HLSL** returns an empty `SimpleLayoutInfo()`
  because pointers are unsupported on HLSL; exercising it would require an
  invalid HLSL pointer input the front-end rejects earlier, so it is not
  reachable as a clean compile.
- **Obfuscated-module layout reuse / caching paths** are link-stage
  bookkeeping with no member-offset surface in single-file emit; not
  observable through a `//TEST` directive in this bundle.
