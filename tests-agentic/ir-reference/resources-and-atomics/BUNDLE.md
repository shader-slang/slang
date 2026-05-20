---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-20T17:00:00+00:00
source_commit: ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
watched_paths_digest: 4cd2b0ab91da080eb6a16ece95070e661cf2096b991cd6d164bfccb383236671
source_doc: docs/llm-generated/ir-reference/resources-and-atomics.md
source_doc_digest: 3ac0724b29539a4ec7edd0c37f2e44add27803e6c97e8427e2d67a80b87bb345
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for ir-reference/resources-and-atomics

## Intent

Tests verify the per-opcode catalog of the IR resource/atomic family
described in
[`docs/llm-generated/ir-reference/resources-and-atomics.md`](../../../docs/llm-generated/ir-reference/resources-and-atomics.md):
that documented IR opcodes for resource access
(`rwstructuredBufferGetElementPtr`, `rwstructuredBufferLoad`,
`structuredBufferLoad`, `byteAddressBufferLoad`,
`byteAddressBufferStore`, `StructuredBufferAppend`,
`StructuredBufferConsume`, `nonUniformResourceIndex`) and module-
scope shader parameter declarations (`global_param` with various
resource types) surface in IR dumps from natural Slang surfaces, and
that the `Atomic<T>` core-module type lowers each of its methods
(`add`, `min`, `max`, `and`, `or`, `xor`, `exchange`,
`compareExchange`, `load`, `store`) to the corresponding atomic
opcode with a trailing memory-order operand.

Each test compiles to a text-emit target with `-dump-ir
-o /dev/null` so the IR dump goes to stdout uncontaminated by
target text, and uses FileCheck patterns anchored at `func %main`
(for opcodes that surface in user code) or as bare opcode
matchers (for opcodes that live inside the pulled-in library
function bodies of byte-address-buffer accessors).

## Claims enumerated

| Claim ID | Anchor                                                | Claim (one line)                                                                                                                              | Tests                                                                                                                                                                                                                                                                  |
| -------- | ----------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| C-01     | #rwstructuredbuffergetelementptr                      | `rwBuf[i] = ...` / `rwBuf[i].f = ...` (lvalue path) surfaces `rwstructuredBufferGetElementPtr(%base, %index)`.                                | `rwstructured-buffer-getelementptr-nested.slang`, `rwstructured-buffer-vector-element.slang`                                                                                                                                                                           |
| C-02     | #buffer-load-and-store                                | `RWStructuredBuffer::Load(idx)` produces `rwstructuredBufferLoad(%buf, %idx)`.                                                                | `rwstructured-buffer-load.slang`, `rwstructured-buffer-load-vector.slang`                                                                                                                                                                                              |
| C-03     | #buffer-load-and-store                                | `StructuredBuffer::Load(idx)` produces `structuredBufferLoad(%buf, %idx)` (read-only buffer counterpart).                                     | `structured-buffer-load.slang`, `structured-buffer-load-struct.slang`                                                                                                                                                                                                  |
| C-04     | #buffer-load-and-store                                | `ByteAddressBuffer::Load(off)` lowers to `byteAddressBufferLoad(%buf, %off, %align)` inside the pulled-in library helper.                     | `byte-address-buffer-load.slang`                                                                                                                                                                                                                                       |
| C-05     | #buffer-load-and-store                                | `RWByteAddressBuffer::Store(off, val)` lowers to `byteAddressBufferStore(%buf, %off, %align, %val)` inside the pulled-in library helper.      | `byte-address-buffer-store.slang`                                                                                                                                                                                                                                      |
| C-06     | #append-and-consume-buffers                           | `AppendStructuredBuffer::Append(v)` lowers to `StructuredBufferAppend(%buf, %v)`; `ConsumeStructuredBuffer::Consume()` to `StructuredBufferConsume(%buf)`. | `append-consume-buffer.slang`, `append-buffer-struct.slang`                                                                                                                                                                                                            |
| C-07     | #nonuniformresourceindex                              | `NonUniformResourceIndex(i)` surfaces as `nonUniformResourceIndex(%i)`; result type matches operand.                                          | `non-uniform-resource-index.slang`, `non-uniform-resource-index-buffer-index.slang`                                                                                                                                                                                    |
| C-08     | #shader-io                                            | Module-scope `uniform` shader-parameter declarations surface a `global_param` line with the resource type carried in the result type.        | `structured-buffer-types-globalparam.slang`, `byte-address-buffer-type-globalparam.slang`, `parameter-block-globalparam.slang`                                                                                                                                         |
| C-09     | #atomic-operations                                    | `Atomic<T>::add(v)` lowers to `atomicAdd(%ptr, %val, %order)`.                                                                                | `atomic-add.slang`, `atomic-uint-add.slang`, `atomic-groupshared-add.slang`                                                                                                                                                                                            |
| C-10     | #atomic-operations                                    | `Atomic<T>::min(v)` and `::max(v)` lower to `atomicMin` / `atomicMax`.                                                                        | `atomic-min-max.slang`                                                                                                                                                                                                                                                 |
| C-11     | #atomic-operations                                    | `Atomic<T>::and(v)`, `or(v)`, `xor(v)` lower to `atomicAnd`, `atomicOr`, `atomicXor`.                                                         | `atomic-bitwise.slang`                                                                                                                                                                                                                                                 |
| C-12     | #atomic-operations                                    | `Atomic<T>::exchange(v)` lowers to `atomicExchange(%ptr, %val, %order)`.                                                                      | `atomic-exchange.slang`                                                                                                                                                                                                                                                |
| C-13     | #atomiccompareexchange                                | `Atomic<T>::compareExchange(e, d)` lowers to `atomicCompareExchange(%ptr, %expected, %desired, ...)`.                                         | `atomic-compare-exchange.slang`                                                                                                                                                                                                                                        |
| C-14     | #atomic-operations                                    | `Atomic<T>::load()` and `::store(v)` lower to dedicated `atomicLoad` / `atomicStore` opcodes (not RMW).                                       | `atomic-load-store.slang`                                                                                                                                                                                                                                              |

## Tests in this bundle

| File                                                  | Intent     | Doc anchor                          |
| ----------------------------------------------------- | ---------- | ----------------------------------- |
| `append-buffer-struct.slang`                          | functional | `#append-and-consume-buffers`       |
| `append-consume-buffer.slang`                         | functional | `#append-and-consume-buffers`       |
| `atomic-add.slang`                                    | functional | `#atomic-operations`                |
| `atomic-bitwise.slang`                                | functional | `#atomic-operations`                |
| `atomic-compare-exchange.slang`                       | functional | `#atomiccompareexchange`            |
| `atomic-exchange.slang`                               | functional | `#atomic-operations`                |
| `atomic-groupshared-add.slang`                        | functional | `#atomic-operations`                |
| `atomic-load-store.slang`                             | functional | `#atomic-operations`                |
| `atomic-min-max.slang`                                | functional | `#atomic-operations`                |
| `atomic-uint-add.slang`                               | functional | `#atomic-operations`                |
| `byte-address-buffer-load.slang`                      | functional | `#buffer-load-and-store`            |
| `byte-address-buffer-store.slang`                     | functional | `#buffer-load-and-store`            |
| `byte-address-buffer-type-globalparam.slang`          | functional | `#shader-io`                        |
| `non-uniform-resource-index-buffer-index.slang`       | functional | `#nonuniformresourceindex`          |
| `non-uniform-resource-index.slang`                    | functional | `#nonuniformresourceindex`          |
| `parameter-block-globalparam.slang`                   | functional | `#shader-io`                        |
| `rwstructured-buffer-getelementptr-nested.slang`      | functional | `#rwstructuredbuffergetelementptr`  |
| `rwstructured-buffer-load-vector.slang`               | functional | `#buffer-load-and-store`            |
| `rwstructured-buffer-load.slang`                      | functional | `#buffer-load-and-store`            |
| `rwstructured-buffer-vector-element.slang`            | functional | `#rwstructuredbuffergetelementptr`  |
| `structured-buffer-load-struct.slang`                 | functional | `#buffer-load-and-store`            |
| `structured-buffer-load.slang`                        | functional | `#buffer-load-and-store`            |
| `structured-buffer-types-globalparam.slang`           | functional | `#shader-io`                        |

## Doc gaps observed

- The `atomic-operations` table lists `atomicSub` as a distinct
  opcode, but `Atomic<T>` does not expose a `sub(v)` method on its
  natural surface — `a.add(-v)` lowers to `atomicAdd(%p, -%v)`,
  not `atomicSub`. The doc should either name the AST surface that
  produces `atomicSub` (perhaps an IR pass that folds `add(-v)`
  into `sub`) or note that `atomicSub` is synthesized.
- The doc's HLSL-intrinsic-mapped atomic rows (`InterlockedAdd` →
  `atomicAdd`, `InterlockedCompareExchange` → `atomicCompareExchange`,
  etc.) imply the opcodes are produced at LOWER-TO-IR, but in the
  observed dump the `Interlocked*` intrinsics remain as
  `call %InterlockedAdd(...)` etc.; only an IR pass later rewrites
  them to atomic opcodes. The portable surface that does produce
  the opcodes directly is the `Atomic<T>` core-module type — the
  doc should mention this surface alongside the HLSL intrinsics.
- The `buffer-load-and-store` table lists `rwstructuredBufferStore`
  with AST origin "`rwBuf[i] = val` lowering in
  `slang-lower-to-ir.cpp`", but the observed lowering of
  `rwBuf[i] = val` is `rwstructuredBufferGetElementPtr` followed by
  `store`, not `rwstructuredBufferStore`. The doc should either
  name a different surface that produces `rwstructuredBufferStore`
  or mark it "(synthesized by an IR pass)".
- The `buffer-load-and-store` table lists
  `structuredBufferLoadStatus` / `rwstructuredBufferLoadStatus`
  produced by the `Load(idx, out status)` overload, but in the
  `compute` stage on `spirv` the overload is gated behind a
  capability flag and emits `E36107`. The doc should either name
  the required capability or note the overload's gating.
- The `barriers-and-synchronization` row says
  `GroupMemoryBarrierWithGroupSync` is the opcode produced by the
  same-named intrinsic, but at LOWER-TO-IR the intrinsic surfaces
  as `call %GroupMemoryBarrierWithGroupSync()` — the opcode form
  must appear later in the pipeline. Same observation applies to
  `ControlBarrier` and `BeginFragmentShaderInterlock` /
  `EndFragmentShaderInterlock`.
- The `wave-intrinsics` table similarly lists
  `waveGetActiveMask` / `waveMaskBallot` / `waveMaskMatch` as the
  opcodes produced by their same-named intrinsics, but at
  LOWER-TO-IR they surface as `call %WaveGetActiveMask(...)`. The
  doc should clarify the stage at which the call form is rewritten
  to the opcode form.
- The `texture-and-image` and `sampling-and-combined-samplers`
  tables list `imageLoad` / `imageStore` / `imageSubscript` /
  `sample` / `sampleGrad` as the opcodes produced by
  `Texture*::Load`, `Texture*::Sample`, `rwtex[uv]`, and
  `rwtex[uv] = v` respectively. At LOWER-TO-IR the user `main` body
  hosts a `call specialize(%Load, ...)` / `%Sample` /
  `%operatorx5Bx5Dx5Fget` / `%operatorx5Bx5Dx5Fset`; the named
  opcodes live inside those library functions' `GenericAsm` bodies
  and are not observable from `main`. The doc should either name
  the IR pass that inlines and replaces the call with the opcode,
  or describe the observation method (look inside the library
  helper body, not main).
- The `resource-queries-and-modifiers` table lists
  `StructuredBufferGetDimensions` with AST origin
  `StructuredBuffer::GetDimensions` method, but the natural
  surface lowers to a `call %StructuredBufferx5FGetDimensions(...)`
  on the helper function; the opcode itself appears only inside
  that helper's body. Same caveat as for the image/sample opcodes
  above.

## Out of scope (no-GPU runner)

These items are documented in the source doc but no portable
LOWER-TO-IR shader surface produces them in a form that this
no-GPU-runner bundle can FileCheck cleanly. See the doc gaps above
for the reasoning per family.

- Texture and image opcodes (`imageSubscript`, `imageLoad`,
  `imageStore`, `SubpassLoad`, `MetalCastToDepthTexture`,
  `IsTextureAccess`, `IsTextureScalarAccess`, `IsTextureArrayAccess`,
  `ExtractTextureFromTextureAccess`, `ExtractCoordFromTextureAccess`,
  `ExtractArrayCoordFromTextureAccess`).
- Sampling and combined-sampler opcodes (`sample`, `sampleGrad`,
  `makeCombinedTextureSampler`, `MakeCombinedTextureSamplerFromHandle`,
  `CombinedTextureSamplerGetTexture`, `CombinedTextureSamplerGetSampler`).
- Status-overload buffer opcodes (`structuredBufferLoadStatus`,
  `rwstructuredBufferLoadStatus`) — require a capability flag not
  available in the `compute` stage on `spirv`.
- The doc's `rwstructuredBufferStore` opcode — actually produced via
  `rwstructuredBufferGetElementPtr` + `store` lowering.
- `StructuredBufferGetDimensions` — appears only inside the helper
  function body, not on `main`.
- Resource queries and modifier helpers (`getNaturalStride`,
  `castDynamicResource`, `getEquivalentStructuredBuffer`,
  `getStructuredBufferPtr`, `getUntypedBufferPtr`,
  `getRegisterIndex`, `getRegisterSpace`).
- Mesh-shader outputs (`meshOutputRef`, `meshOutputSet`,
  `metalSetVertex`, `metalSetPrimitive`, `metalSetIndices`) —
  require mesh stage.
- Workgroup / stage introspection (`GetWorkGroupSize`,
  `GetCurrentStage`) — materialized in the layout / emit pipeline.
- Barriers (`GroupMemoryBarrierWithGroupSync`, `ControlBarrier`,
  `BeginFragmentShaderInterlock`, `EndFragmentShaderInterlock`)
  — surface as `call` at LOWER-TO-IR.
- Cooperative matrix / vector (`CoopMatMapElementIFunc`,
  `CoopMatMulAdd`, `CoopVecMatMulAdd`, `CoopVecOuterProductAccumulate`,
  `CoopVecReduceSumAccumulate`) — require capability flags and
  core-module `CoopMat` / `CoopVec` types.
- Wave intrinsics (`waveGetActiveMask`, `waveMaskBallot`,
  `waveMaskMatch`) — surface as `call` at LOWER-TO-IR.
- Raytracing payload (`getOptiXRayPayloadPtr`,
  `getOptiXHitAttribute`, `getOptiXSbtDataPointer`,
  `getOptiXPayloadRegister`, `setOptiXPayloadRegister`,
  `GetVulkanRayTracingPayloadLocation`) — require raytracing stage
  / OptiX backend.
- Descriptor heaps (`LoadResourceDescriptorFromHeap`,
  `LoadSamplerDescriptorFromHeap`, `SPIRVLoadDescriptorFromHeap`,
  `SPIRVLoadTexelPointerFromHeap`, `SPIRVResourceHeap`,
  `SPIRVSamplerHeap`) — bindless-only synthesized opcodes.
- `MetalAtomicCast`, `IncrementCoverageCounter` — synthesized.
- `atomicSub` — `Atomic<T>` does not expose a `.sub(v)` surface; see
  doc gaps.
