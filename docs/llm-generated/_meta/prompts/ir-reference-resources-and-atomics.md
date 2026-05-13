# Prompt: ir-reference/resources-and-atomics.md

See [_common.md](_common.md) for the universal rules and the
**IR-reference family contract**.

## Target

Produce `docs/llm-generated/ir-reference/resources-and-atomics.md`
— the per-opcode reference for GPU-resource operations
(textures / images, samplers, structured / byte-address / append /
consume buffers), shader-IO opcodes (entry-point and global
parameters), atomic operations, memory barriers, and descriptor-heap
queries.

The relevant Lua ranges include the `imageLoad` / `imageStore`
cluster (around line 1153), the `rwstructuredBufferLoad*` /
`rwstructuredBufferStore` cluster (around lines 1185-1204), the
`AtomicOperation` parent (around line 1071), and shader-IO entries
that live elsewhere in the file.

## Family-specific guidance

- Split `## Opcodes` into:
  - **Texture and image** (`imageLoad`, `imageStore`,
    `imageSubscript`, `SampleImplicit`, `SampleExplicit`,
    `SampleCmp*`, `SampleGrad*`, `Texture*Query*`).
  - **Buffer load / store** (`structuredBufferLoad`,
    `rwstructuredBufferLoad`, `rwstructuredBufferLoadStatus`,
    `rwstructuredBufferStore`, `rwstructuredBufferGetElementPtr`,
    `byteAddressBufferLoad`, `byteAddressBufferStore`).
  - **Append / consume** (`AppendStructuredBuffer`,
    `ConsumeStructuredBuffer`).
  - **Sampler operations** (sampler-state queries, combined
    texture/sampler helpers).
  - **Shader IO** (`EntryPointParam`, `GlobalParam`, geometry-shader
    output emitters, mesh-shader output writes, raytracing payload
    ops). Cite the Lua entries by name.
  - **Atomics** (`AtomicOperation` group: `AtomicLoad`, `AtomicStore`,
    `AtomicAdd`, `AtomicSub`, `AtomicMin`, `AtomicMax`, `AtomicAnd`,
    `AtomicOr`, `AtomicXor`, `AtomicExchange`, `AtomicCompareExchange`).
  - **Barriers and sync** (memory / group / control barrier opcodes;
    cite `MemoryBarrier`, `GroupMemoryBarrier`, `ControlBarrier`,
    ...).
  - **Descriptor heaps** (`LoadResourceDescriptorFromHeap`,
    `LoadSamplerDescriptorFromHeap`,
    `SPIRVLoadDescriptorFromHeap`,
    `SPIRVLoadTexelPointerFromHeap`).
- The atomics row should cite the `IRMemoryOrder` enum from
  [slang-ir.h](../../../source/slang/slang-ir.h) and note that the
  ordering is an extra operand on the atomic opcode.
- `AST origin` typically points to method-call expressions on
  resource types or to entry-point parameter declarations.

## Notable opcodes

Cover at least:

- `imageLoad` / `imageStore` — operand layout, coordinate encoding.
- `SampleImplicit` vs `SampleExplicit` vs `SampleGrad*` — what each
  encodes about LOD selection.
- `rwstructuredBufferGetElementPtr` — its role as the lvalue
  counterpart to `rwstructuredBufferLoad`.
- `AtomicCompareExchange` — operand layout and what the expected /
  desired pair encodes.
- `EntryPointParam` and `GlobalParam` — how they differ and how
  layout decorations attach to them.
- `LoadResourceDescriptorFromHeap` — how descriptor-heap indexing
  surfaces in the IR.

## Forbidden content

- Layout decorations themselves (the `LayoutDecoration` / `Layout`
  family) — see [decorations.md](decorations.md) and
  [metadata.md](metadata.md).
- Target-specific lowering of these opcodes — see
  [../pipeline/06-emit.md](../pipeline/06-emit.md) and the
  `slang-emit-*.cpp` files.

## Quality checklist (in addition to the universal one and the family contract)

- [ ] Every `AtomicOperation` child is listed.
- [ ] Hierarchy diagram covers the seven sub-groups above.
- [ ] At least one shader-IO row and one buffer row cite a
      `slang-lower-to-ir.cpp` visitor.
