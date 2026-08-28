# Execute aggregate and read-only resource kernel parameters

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, the direct libNVVM backend executes a real raw CUDA kernel whose launch arguments
combine a selected-scalar struct, a read-only `StructuredBuffer<float>`, a read-write
`RWStructuredBuffer<float>`, and a scalar count. The existing
`tests/cuda/cuda-kernel-param-layout.slang` must produce the same `11, 12, 13, 14` result through
CUDA/NVRTC and direct libNVVM, and CUDA `ptxas` must accept the direct module.

The implementation generalizes existing representations instead of adding shader-resource-shaped
builder calls. A selected-scalar struct is a generic LLVM struct value in ordinary roles and a
generic typed pointer carrying `byval` in the physical kernel signature. A raw read-only or
read-write structured-buffer view is the established generic `{ element address-space-1 pointer,
i64 count }` value. Field extraction, pointer offset, and invariant load use existing builder
operations.

## Progress

- [x] (2026-08-28) Re-established the 342/342 NVVM unit baseline in Slice 79 and committed it as
  `faca6e990`.
- [x] (2026-08-28) Probed representative CUDA-suite files. The motivating layout shader stops at
  E52017 `entry-point parameter`; its final linked entry has `%Padding`, read-only
  `StructuredBuffer<Float>`, read-write `RWStructuredBuffer<Float>`, and `UInt` parameters.
- [x] (2026-08-28) Generalized scalar struct and raw structured-buffer view classification and
  proved isolated read-only/read-write views reach libNVVM with the established representation.
- [x] (2026-08-28) Added exact ABI revision 6's generic LLVM by-value parameter contract and
  LLVM-14-to-LLVM-7 textual normalization.
- [x] (2026-08-28) Emitted exact keyed struct extraction and read-only structured-buffer loads
  through existing
  generic builder operations.
- [x] (2026-08-28) Added negative, fake-boundary, LLVM/libNVVM, PTX, `ptxas`, and GPU comparison
  evidence.
- [x] (2026-08-28) Formatted, built, ran focused and complete validation, updated durable documents,
  self-reviewed, and prepared the completed slice for commit.

## Surprises and Discoveries

- CUDA varying legalization removes the `SV_DispatchThreadID` parameter but intentionally leaves
  raw CUDA launch arguments on the kernel. The final motivating entry is
  `Func(Void, Padding, StructuredBuffer(Float), RWStructuredBuffer(Float), UInt)`, not a
  conventional zero-argument shader entry using `SLANG_globalParams`.
- The final body already uses executable operations with generic equivalents: `get_field` for the
  two padding fields, `structuredBufferLoad` for the input, and the established
  `rwstructuredBufferGetElementPtr` plus `store` for the output. The first rejection occurs before
  provider discovery solely because the aggregate and read-only parameter types are outside the
  current role classifiers.
- `dispatch-thread-id-extraction.slang` reaches the same entry-parameter boundary for an adjacent
  read-only structured buffer. `get-buffer-ptr.slang` instead stops at a byte-address-buffer field,
  and vector fixtures stop at vector construction. Those are separate later families.
- Passing `%Padding` directly as an LLVM struct argument verifies in LLVM 14 but causes the
  CUDA 12.9 libNVVM process to terminate with access violation `0xC0000005`. Read-only,
  read-write, and scalar control parameters compile independently. The LLVM 7 NVPTX regression
  `lower-kernel-ptr-arg.ll` models aggregate kernel arguments as generic pointers with `byval`, and
  accesses their fields through GEP/load. The downstream crash therefore exposed a missing LLVM
  parameter ABI contract, not a reason to flatten the Slang struct.
- A raw `[CUDAKernel]` with a retained struct parameter legitimately has no conventional global
  parameter block. The existing conventional-storage helper assumed that optional block was
  present and dereferenced a null element type while scanning retained globals. Returning false
  when the block is absent fixes the ownership predicate; admitting only the exact canonical
  entry-parameter struct then preserves the closed-world global audit.
- A stale Release MSBuild link-tracking record listed only `slang-compiler.pdb`, so MSBuild skipped
  linking while `slang-compiler.dll` was absent. Moving that generated PDB aside forced one clean
  relink; subsequent serialized target builds and tests were stable.

## Decision Log

- Decision: admit exact nonempty structs containing only selected integer and float32 scalar fields
  as entry-parameter and value types, and reuse that classifier for scalar parameter-group element
  structs.
  Rationale: the canonical struct definition and keyed fields are already the source of truth.
  One classifier prevents raw launch structs and parameter-group structs from drifting while
  retaining an explicit bounded shape. Revisit when nested aggregate execution is deliberately
  added.
  Date/author: 2026-08-28, Codex.
- Decision: generalize the raw `RWStructuredBuffer<T>` recognizer and lowering to one exact raw
  structured-buffer-view family that records whether the source type is read-only or read-write.
  Rationale: both source types have the same CUDA value representation; access is a semantic
  constraint on allowed operations, not a reason for duplicate LLVM types or builder callbacks.
  Revisit if CUDA layout evidence shows a representation difference for another buffer kind.
  Date/author: 2026-08-28, Codex.
- Decision: lower `IRStructuredBufferLoad` into existing struct-field-value, pointer-offset, and
  invariant-load operations.
  Rationale: the linked IR operation canonically denotes an immutable resource read, and the
  provider already exposes every generic IR construction needed. A resource-specific callback
  would duplicate IR semantics in the ABI.
  Date/author: 2026-08-28, Codex.
- Decision: preserve raw CUDA kernel parameters rather than collecting them into the conventional
  global block.
  Rationale: CUDA varying legalization intentionally retains these launch parameters and the
  render-test runtime already owns their launch layout. Rewriting them would change the producer's
  established ABI and conceal rather than support the actual input shape.
  Date/author: 2026-08-28, Codex.
- Decision: represent a scalar-struct entry parameter physically as a generic pointer carrying
  explicit by-value type and alignment attributes, through one extensible generic parameter-
  attribute builder operation and exact ABI revision 6.
  Rationale: this is the LLVM 7 NVPTX aggregate-kernel contract and preserves the source argument
  as one launch parameter. Keeping parameter attributes independent from declaration avoids
  duplicating every ordinary declaration call, while flags can cover later generic parameter
  properties. Revisit if another required attribute must affect the physical parameter type rather
  than decorate it.
  Date/author: 2026-08-28, Codex.
- Decision: give entry-parameter representations their own type cache entry.
  Rationale: the same canonical Slang struct is a first-class LLVM struct as an ordinary value but
  a generic by-value pointer in a kernel signature. Looking up the ordinary canonical-type cache
  would conflate those roles. The role-specific cache records the deliberate ABI representation
  instead of creating a second Slang IR type.
  Date/author: 2026-08-28, Codex.

## Outcomes and Retrospective

The representative fake trace contains one scalar-struct pointer parameter with `BY_VALUE`, exact
aggregate pointee, alignment 8, field indices 0 and 1, separate read-only/read-write float resource
views, two data-field extractions and offsets, and three invariant loads. The real provider prints
LLVM 14 `byval({ i64, i16 }) align 8`, normalizes the compatible text to `byval align 8`, and
libNVVM emits an aligned 16-byte PTX aggregate parameter.

`tests/cuda/cuda-kernel-param-layout.slang` passes all four registered lanes. CUDA/NVRTC and direct
libNVVM both produce `11, 12, 13, 14`; direct PTX contains the aligned 16-byte aggregate, two
aligned 16-byte resource views, `ld.global.nc.f32`, and `st.global.f32`. CUDA 12.9 `ptxas` accepts
the module for `sm_70`. The standalone provider and Release host/test targets build successfully,
focused builder/emitter/negative tests pass, and the complete NVVM unit prefix passes 344/344.

Self-review inventory: the new scalar-struct and access-aware resource classifiers survive as the
single bounded type predicates; the entry-parameter representation cache survives because one
canonical struct has intentionally different value and physical ABI roles; the conventional-global
absence check survives because raw kernels are valid without that optional producer; exact retained
entry-struct identity survives in the global audit; no syntax is reconstructed, no structural
equivalence is introduced, and no resource-specific builder callback or fallback remains.

## Context and Current Pipeline

Consider the existing shader:

```slang
struct Padding
{
    uint64_t big;
    uint16_t little;
}

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(
    uint tid : SV_DispatchThreadID,
    uniform Padding padding,
    uniform StructuredBuffer<float> input,
    uniform RWStructuredBuffer<float> output,
    uniform uint count)
{
    if (tid >= count)
        return;
    output[tid] = input[tid] + padding.big + padding.little;
}
```

CUDA varying legalization replaces `tid` with the established `blockIdx * blockDim + threadIdx`
graph and leaves four raw launch parameters. `slang-emit-nvvm.cpp::_validateNVVMFunction` currently
rejects `%Padding` before inspecting the body because `isNVVMSupportedParameterType` admits only
selected scalars, selected pointers/arrays, and raw read-write structured buffers.

The body then uses `IRFieldExtract` keyed by `%big` or `%little`, and
`IRStructuredBufferLoad(input, tid)`. `NVVMTypeLoweringContext` already lowers generic structs,
address-space-1 pointers, and `{ pointer, i64 }` resource views for storage and read-write resource
values. The builder already supplies `emitStructFieldValue`, `emitPointerOffset`, and `emitLoad`.
This slice joins those existing pieces under exact type/operation validation. The measured
libNVVM crash adds one required physical boundary: an aggregate kernel parameter is a generic LLVM
pointer with `byval(aggregate-type)` and natural CUDA alignment, and its fields are loaded through
that pointer.

## Scope and Non-Goals

In scope are nonempty flat structs of selected integer/float32 fields in raw entry parameters and
ordinary values; exact keyed scalar field extraction; exact default-layout raw
`StructuredBuffer<T>` and `RWStructuredBuffer<T>` views for selected scalar `T`; read-only
structured-buffer load; conventional-global read-only views when they have the same canonical
shape; invariant metadata; and the existing layout shader's end-to-end execution.

Out of scope are nested structs, arrays or matrices in runtime aggregate values; aggregate helper
parameters/results/phis; vector-valued fields; aggregate construction or mutation; byte-address,
append/consume, rasterizer-ordered, or descriptor-heap resources; resource length queries; writable
operations through a read-only view; general vector construction; and changes to host launch ABI.
Those remain deterministic pre-provider boundaries.

## Architecture and Invariants

The final linked IR is authoritative. A scalar struct is admitted only when its canonical
`IRStructType` is nonempty and every keyed field has an established selected scalar type. Field
extraction resolves the exact key to its actual declaration index and verifies the result type.
It never assumes source order from a separate layout description or reconstructs syntax.

One raw structured-buffer classifier accepts only canonical default-layout read-only or read-write
HLSL structured-buffer types with a selected scalar element. Both lower to `{ T addrspace(1)*,
i64 }`, cached by canonical Slang type so their semantic identities remain distinct even if LLVM
structurally reuses a type. The classifier reports access; `IRRWStructuredBufferGetElementPtr`
requires a read-write view, while `IRStructuredBufferLoad` requires a read-only view and an index
accepted by the existing integer32 value contract.

The structured-buffer load extracts field zero, offsets the pointer by the exact index, and emits a
naturally aligned invariant load. The semantic immutability comes directly from the read-only
resource type/operation pair. Unknown resource types, element types, field keys, result relations,
and nested aggregates fail before provider discovery.

## Interfaces and Dependencies

Update `source/slang/slang-emit-nvvm-type-lowering.{h,cpp}` with one scalar-struct classifier and a
generic raw structured-buffer-view classifier. Rename the existing read-write-specific lowering
helper to reflect the generic representation. Extend the existing `NVVMTypeUse` role matrix without
changing the source-level type identity. Add a role-specific cache for the generic pointer used by
an aggregate entry parameter.

Revise `source/compiler-core/slang-nvvm-ir-builder-api.h` to exact ABI revision 6 with one generic
parameter-attribute setter. Its extensible flags currently contain `BY_VALUE`; the operation takes
the exact pointee type and alignment and validates every field before mutation. Update the facade,
real provider, fake provider, and ABI completeness tests without retaining a compatibility path.
LLVM 14's typed `byval(T)` spelling must be validated and normalized to LLVM 7's untyped `byval`
spelling in the existing legacy text writer.

Update `source/slang/slang-emit-nvvm.cpp` to validate and emit `IRFieldExtract` and
`IRStructuredBufferLoad`, and to use access-aware resource classification for existing read-write
addressing. Reuse `emitStructFieldValue`, `emitPointerOffset`, and `emitLoad`; no construction API
operation is added.

Extend `tools/slang-unit-test/unit-test-nvvm-support.h` and
`tools/slang-unit-test/unit-test-nvvm-emitter.cpp` with representative direct-source and fake
provider evidence. Add a direct GPU comparison lane to
`tests/cuda/cuda-kernel-param-layout.slang` and focused adjacent-shape rejection where needed.

## Milestones

1. Add and test the exact scalar-struct and access-aware raw resource-view classifiers, then reuse
   them in type role validation and lowering.
2. Add first-pass and SSA validation for keyed scalar `IRFieldExtract` and read-only
   `IRStructuredBufferLoad`, including exact base/result/access relations and negative cases.
3. Emit both operations by composing existing generic builder operations and record the fake
   provider trace, including struct indices, resource data-field extraction, pointer offset,
   alignment, and invariant load flag.
4. Compile the representative module through the real provider's LLVM 14 and LLVM 7-compatible
   text paths, libNVVM, and PTX; inspect raw kernel parameters and run `ptxas`.
5. Register the existing layout shader's direct GPU comparison, run focused and complete tests,
   update durable documents, self-review, and commit.

## Validation and Acceptance

Run every CMake build and test outside the sandbox. Acceptance requires:

- fake-provider preflight accepts exact scalar struct/read-only view parameters and sees no
  resource-specific builder operation;
- field keys map to actual struct indices and mismatched/unknown keys fail before provider mutation;
- read-only structured-buffer load composes data-field extraction, pointer offset, and naturally
  aligned invariant load, while read-write element addressing remains access-checked;
- nested aggregate parameters and unsupported resource/element types retain E52017 before provider
  discovery;
- LLVM 7-compatible text verifies and libNVVM produces PTX with the expected raw parameter shapes;
- `tests/cuda/cuda-kernel-param-layout.slang` returns `11, 12, 13, 14` through both CUDA/NVRTC and
  direct libNVVM;
- CUDA 12.9 `ptxas -arch=sm_70` accepts the direct PTX;
- Release host and standalone-provider builds, focused tests, and the complete NVVM prefix pass;
- pinned clang-format 17, `git diff --check`, and repository status checks pass; and
- `external/slang-binaries/` and generated `build/` artifacts remain unstaged.

## Failure and Recovery

If libNVVM or `ptxas` rejects an LLVM aggregate kernel parameter despite valid generic IR, isolate
the smallest by-value struct and compare generated CUDA/PTX ABI before changing the representation.
Do not flatten fields speculatively. If the GPU launch result disagrees, compare reflection offsets,
render-test argument packing, and PTX `.param` declarations to identify the producer/consumer whose
layout contract differs.

If read-only resource lowering fails, inspect the canonical final op and type. Do not accept an
adjacent resource by name or make loads mutable to obtain PTX. All type, emitter, test, and document
changes form one forward-only slice and can be reverted together.

## Artifacts and Hand-Off

Keep final linked IR, normal LLVM assembly, LLVM 7-compatible text, direct PTX, `ptxas` cubin, and
GPU output evidence under ignored `build/` paths. Distill the scalar aggregate contract,
access-aware resource-view representation, composed immutable-load lowering, registered file test,
remaining boundaries, and exact validation totals into `docs/design/nvvm-backend.md` and
`docs/design/nvvm-backend-capability-ledger.md`. Complete the living sections and self-review
inventory before committing this plan with Slice 80.
