# Slice 155: Finite group-shared value storage

## 1. Motivation

Consider this source:

```slang
struct Data
{
    int x;
    int y;
}

groupshared Data shared;

void read(Ptr<Data, Access::ReadWrite, AddressSpace::GroupShared> value)
{
    output[0] = value.x;
}
```

After address-space specialization and linking, `shared` is an uninitialized
`IRGlobalVar : RateQualified(GroupShared, Ptr<Data>)`. The helper parameter is the explicit
`Ptr<Data, ReadWrite, GroupShared, DefaultLayout>`, and selecting `x` produces an explicit
group-shared scalar-layout field pointer. Direct NVVM already represented `Data` as a first-class
helper value, yet independent numeric-only checks rejected it at the shared global, derived
pointer, or helper-call boundary.

The same representation gap blocked fixed shared arrays and the existential aggregate in
`groupshared-struct-with-interface`. The problem was not four missing operations; it was the lack
of one storage invariant connecting a canonical shared producer to the existing finite helper
value algebra.

## 2. Proposed solution

Use one exact `NVVMSharedGlobal` descriptor for canonical uninitialized group-shared globals. It
records the global, finite semantic storage type, and executable alignment type. Reuse the existing
recursive helper-value classifier for the storage and for exact group-shared helper and element
pointers.

Derive physical address space 3 from the `IRGroupSharedRate` producer. Preserve exact result
pointee, access, and layout checks on every derived pointer. At helper calls, admit only the direct
shared global or a pointer whose type explicitly retains group-shared provenance. Validate the
recursive storage layout before any provider mutation, then use the existing generic typed global,
load/store, GEP, and call operations.

## 3. Change summary

- `slang-emit-nvvm-type-lowering` replaces separate numeric scalar/array global classifiers with
  `NVVMSharedGlobal` and widens exact group-shared pointers to finite helper values.
- `slang-emit-nvvm` uses that descriptor for type reachability, layout preflight, global emission,
  array and field addressing, pointer validation, atomics, naming, and exact helper arguments.
- Four stable workloads gain permanent direct-NVVM O0/O3 differential lanes.
- Frozen-v1 and discovery census snapshots, the measurement manifest, design documentation, and
  this completed plan/report record the resulting coverage separately.

## 4. Concepts and vocabulary

- **Finite helper value**: a non-recursive provider value selected by the existing helper ABI
  classifier, including supported scalars, fixed arrays, aggregates, and selected pointer leaves.
- **Group-shared rate**: the canonical global producer marker that assigns CUDA shared-memory
  provenance even when the global pointer type itself uses a generic spelling.
- **Pointer relation**: the checked producer/base/result relationship that preserves exact pointee,
  access, address space, and data layout through an address-producing IR instruction.
- **Healthy denominator**: workloads with a stable native CUDA/NVRTC reference; frozen v1 and
  discovery denominators are reported independently.

## 5. Process report

The first input-shape audit found two canonical pointer spellings on group-shared globals. The
array sources produce a one-operand `Ptr<Array<T, N>>`, while the existential workload produces
the exact three-operand `Ptr<SharedState, ReadWrite, Generic>` with no layout operand. Both are
created on an `IRGlobalVar` carrying `IRGroupSharedRate`. They are intentional final-IR shapes: the
rate, not an inferred address-space operand, is the source of storage class. The descriptor accepts
only those two spellings, rejects initialized globals, and requires the existing finite helper
value or selected atomic representation.

That producer distinction matters for array access. `IRBuilder::emitElementAddress` can emit an
`IRGetElementPtr` whose base type is shape-identical to a local array pointer. The old type-only
local classifier could therefore claim a shared base first and expect generic address space. The
resolver now checks the direct canonical shared global first, proves its fixed helper array, and
requires the derived `Ptr<Element, ReadWrite, GroupShared, ScalarLayout>`. This is producer-side
classification at the relation boundary, not a fixture check or a downstream cast.

For struct access, `IRBuilder::emitFieldAddress` produces either a field directly rooted in the
shared global or one rooted in an explicit helper pointer. The existing field-key lookup remains
the source of truth for the declared field type. The new branches only select the already-proved
aggregate base and mutability; they do not rebuild a struct or search operand graphs. A direct
global can have a generic pointer spelling because its rate owns the physical address space, while
the helper-pointer spelling must explicitly be group-shared.

The call from `ptr-to-groupshared` exposed the one valid source/parameter spelling difference. The
argument is the direct `RateQualified(GroupShared, Ptr<Data>)` global and the parameter is
`Ptr<Data, ReadWrite, GroupShared, DefaultLayout>`. `_isSupportedNVVMHelperArgument` is now
value-aware so it can verify that exact producer. Its helper rejects arbitrary generic pointers:
only the shared global itself or an already classified explicit group-shared helper/element pointer
survives. An earlier exploratory root-address walk was removed during self-review because it would
have admitted provenance not required by any canonical target.

Adding each shared storage type to the retained reachable-type roots ensures that its struct keys
and child types survive the module-scope audit. `_hasNVVMCompatibleCopyableValueLayout` then checks
the descriptor's alignment representation before declaration. Emission lowers the exact semantic
storage type and calls the existing revision-30 typed global operation with address space 3. No
provider callback, ABI revision, bitcast storage, compatibility fallback, or malformed-IR patch is
needed.

Four workloads prove this layer owns the invariant. Frozen
`groupshared-struct-with-interface` transports an existential tuple plus a float. Discovery
`array-size-groupshared` and `generic-groupshared` cover canonical fixed numeric arrays through the
unified path. Discovery `ptr-to-groupshared` covers an aggregate global, exact helper argument, and
field access. All four are correct against native CUDA at O0 and O3 and receive eight permanent
direct lanes.

`groupshared-ptr-of-device` proves the boundary rather than another widening. Its shared array of
`UserPointer<int>` now passes Slang preflight, but `_convertGlobalNVVMPointerToUserPointer` asks the
provider for an address-space-1 to generic pointer conversion and receives `-2147024809`. That is
an independent `UserPointer` value-representation/provider issue and remains unsupported. The
capability-gated `coherent-load-store-groupshared` row remains infrastructure-owned. Two WebGPU
failures encountered by whole-source test runs are likewise unrelated existing infrastructure
results.

Frozen corpus v1 remains exactly 452 workloads/427 healthy references and moves from 380/384/380
to 381/385/381 O0/O3/both correctness. Discovery remains exactly 82/72 and moves from 55/55/55 to
58/58/58. Both comparisons have zero old-correct loss, and a post-self-review direct-only rerun
matched every prior row classification, exact shape, and diagnostic. The selected prefix passes
427/427. All established measurement gates assemble at direct O3 with CUDA 12.9 for SM70, SM80,
and SM90. The existential gate measures 287.2 ms and 1007-byte PTX at direct O3 SM70 versus
385.3 ms and 8946 bytes for NVRTC O3; direct O0 emits 60001-byte PTX. These timing and size numbers
remain exploratory rather than controlled benchmark claims.
