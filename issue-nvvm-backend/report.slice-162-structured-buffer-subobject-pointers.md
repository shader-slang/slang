# Slice 162: Structured-buffer subobject pointers

## Motivation

Direct NVVM already supported ordinary numeric structured-buffer elements, but two real workloads
showed that the element pointer contract stopped at the simplest storage shape. Consider the
read-only matrix in `gh-5776.slang`:

```slang
struct S
{
    row_major float2x2 nonDefaultMajorMatrix;
}

StructuredBuffer<S> gInput;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    S value = gInput[dispatchThreadID.x];
    // The test writes the matrix values to its output buffers.
}
```

Final layout lowering represents the physical matrix storage as a struct containing a fixed array
of vector rows. `IRRWStructuredBufferGetElementPtr` produces a pointer to that physical element,
and address-space specialization retains Slang `AddressSpace::StorageBuffer`. Slice 161 rejected
that exact instruction at preflight with `raw RWStructuredBuffer numeric element pointer` because
the old type predicate admitted only the generic pointer spelling and the old instruction check
required a writable resource.

The discovery workload `parameter-block-load.slang` exposed the compositional form:

```slang
ParameterBlock<RWStructuredBuffer<uint3>> gBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    gBuffer[0][0] = 1;
}
```

Here `IRRWStructuredBufferGetElementPtr` returns the canonical generic pointer to `uint3`, then
`IRBuilder::emitElementAddress` produces a typed `IRGetElementPtr` for the selected `uint` lane.
Preflight reported `sequential element pointer: Ptr<uint, ... layout=ScalarLayout>` because the
sequential resolver recognized local vectors but not a vector rooted in resource storage.

## Proposed solution

Make the exact structured-buffer element instruction the source of truth. One resolver proves its
selected structured-buffer operand, read-only or read/write access, i32 index, result pointer
contract, and exact resource-element/pointee relation. The result type may use only the generic or
storage-buffer spelling produced by final address-space specialization. Both map to the existing
LLVM global resource pointer, but only after the producer proof succeeds.

Allow an exact fixed-array or vector `IRGetElementPtr` to compose from that root through the
existing sequential-element resolver. Preserve the root's access and address space, propagate
read-only provenance through nested indices, and limit an immutable chain to child addressing and
loads. Reuse the provider's aggregate extraction, pointer offset, and sequential element pointer
operations. No new callback or ABI revision is needed.

## Change summary

- `slang-emit-nvvm-type-lowering.cpp` recognizes the two final producer-owned address-space
  spellings for structured-buffer element pointer types.
- `slang-emit-nvvm.cpp` adds one exact structured-buffer element resolver, reuses it in instruction
  and pointer validation, admits exact array/vector child GEPs, and propagates immutable resource
  provenance.
- Focused fake-emitter coverage proves read-only physical matrix storage and resource-rooted vector
  lane addressing while preserving the earlier read/write matrix assertions.
- `gh-5776.slang` and `parameter-block-load.slang` gain permanent O0 and O3 direct-NVVM lanes.
- Frozen and discovery census/Pareto artifacts stay separate, and the representative measurement
  manifest grows from 19 to 21 gates.
- The design document, capability ledger, completed plan, and this process report record the
  representation and its validation evidence.

## Concepts and vocabulary

**Structured-buffer element pointer** means the result of the exact final-IR
`IRRWStructuredBufferGetElementPtr` instruction. Despite its historical operation name, the
producer occurs for read-only physical structured-buffer storage as well as writable elements.

**Physical storage type** means the recursively lowered buffer representation used in memory. A
matrix can become a struct and fixed row array even though its ordinary semantic value is a matrix.
The existing structured-buffer load conversion owns the transition between those representations.

**Sequential subobject pointer** means an exact `IRGetElementPtr` selecting one element of a fixed
array or numeric vector. It is distinct from byte-offset or raw-buffer reinterpretation.

**Producer-owned address-space spelling** means that generic and storage-buffer Slang pointer
types are equivalent for this backend only when the exact structured-buffer element instruction
establishes CUDA global-resource provenance.

## Process report

The input-shape audit started from final linked IR, not the source fixture names. In `gh-5776`,
buffer layout lowering makes the matrix's physical element a supported recursive storage struct,
and `specializeAddressSpace` retains `AddressSpace::StorageBuffer` on the exact
`IRRWStructuredBufferGetElementPtr` result. That is a canonical target-owned shape: the instruction
selects one element from a selected structured buffer, and the pointer is consumed by the existing
physical structured-buffer load path. The type classifier therefore accepts generic or
storage-buffer address space, but it still proves the pointer opcode, four-operand scalar layout,
read/write pointer qualifier, and recursively supported pointee.

Widening that type predicate alone would be unsafe because the same pointer type could appear on an
unrelated producer. `_getNVVMStructuredBufferElementPointer` supplies the missing semantic proof.
It requires exactly `IRRWStructuredBufferGetElementPtr(buffer, index)`, an i32 index, a selected raw
structured-buffer type, an admitted result pointer, and exact equality between the selected buffer
element and pointer pointee. The resolver retains the buffer access mode. Both
`_validateNVVMFunction` and `_validatePointerValue` now consume this single result instead of
repeating weaker operation/type checks.

The first probe after the address-space change exposed the old read/write-only instruction
relation. The operation name suggested mutability, but the audited final IR proved that read-only
physical storage intentionally uses it too. The resolver therefore admits both selected access
modes and makes access a property of the buffer operand. Its consumer rule allows a read-only root
only to feed an exact sequential child or a load. A store, swizzled store, atomic, helper call, or
other escape retains a deterministic preflight rejection. This preserves the semantic source of
truth rather than trusting the pointer's historical read/write qualifier.

In `parameter-block-load`, `IRBuilder::emitElementAddress` creates an exact child
`IRGetElementPtr(elementPointer, laneIndex)`. `_getNVVMSequentialElementPointer` now asks the shared
element resolver whether its base is the canonical resource producer. It then accepts only a
supported fixed array or numeric vector, requires the result pointee to equal the selected element,
requires the exact inherited address space and access qualifier, and requires an i32 index. Nested
sequential pointers already recurse through this resolver, so immutable provenance naturally
flows through the chain. Emission remains the established `emitSequentialElementPointer` call.

The implementation cascades were useful boundary checks. The physical matrix first exposed the
address-space spelling, then the read-only producer relation. The vector workload next exposed the
resource-pointer consumer gate. Consolidating those three checks in the exact resolver fixed both
workloads; no syntax reconstruction, operand-graph search, fixture-name check, compatibility
fallback, downstream malformed-IR patch, or provider callback was added.

The self-review inventory contains four changes that might otherwise look like special cases. The
broader pointer type classifier survives only because every new semantic use is gated by the exact
producer resolver. The resolver survives as the single source of truth for the canonical
instruction and buffer/result relation. The two sequential array/vector branches survive because
they reuse that resolver and the selected workloads prove both physical aggregate and numeric
vector composition. The read-only consumer condition survives because the buffer operand, not the
historical pointer operation name or qualifier, owns mutability. Removing any of these pieces
reproduces one of the audited first failures. The provider interface remains revision 30 because
its existing typed aggregate extraction and pointer operations express the complete invariant.

Validation used a Release build and all tests ran outside the sandbox. The three focused fake
units pass, both promoted shaders pass their O0/O3 differential lanes, and the selected direct-NVVM
prefix passes 428/428. Frozen corpus v1 remains exactly 452 workloads/427 healthy references and
improves from 390/394/390 to 391/395/391 O0/O3/both-mode correct. Across all frozen rows, native
CUDA is 449 correct and three infrastructure; direct O0 is 404 correct, 35 preflight, eight runtime
mismatch, and five provider; direct O3 is 409 correct, 35 preflight, and eight runtime mismatch.
There are no old-correct losses.

Discovery remains exactly 82 workloads/72 healthy references and improves from 63/63/63 to
64/64/64. Each direct mode reports 64 correct, eight preflight, two provider, seven infrastructure,
and one runtime mismatch. The only newly unlocked rows across the two separate corpora are
`bugs/gh-5776.slang#cuda-1` and `bugs/parameter-block-load.slang#discovery-1`.

All 21 representative direct-O3 gates assemble with CUDA 12.9 for SM70, SM80, and SM90. The
physical matrix gate measures 245.7 ms and 1004-byte PTX at direct O3 SM70 versus 356.9 ms and
9438 bytes through NVRTC O3; direct O0 measures 240.7 ms and emits 8638-byte PTX. The vector
subobject gate measures 231.3 ms and 851-byte PTX versus 342.4 ms and 8858 bytes; direct O0 measures
228.5 ms and emits 2207-byte PTX. These timings remain exploratory rather than a controlled
benchmark. Provider ABI revision 30 and both corpus identities are unchanged.
The repository formatting driver was also attempted; this machine lacks gersemi, clang-format,
prettier, and shfmt, so no automated formatter ran. Manual diff review and `git diff --check` are
clean.
