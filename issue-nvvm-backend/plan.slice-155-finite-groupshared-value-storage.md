# Generalize group-shared storage to finite helper values

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires the completed slice plan to be committed with this experimental backend.

## Purpose and Observable Result

After this slice, direct NVVM stores the existing finite helper-value algebra in CUDA group-shared
memory rather than restricting shared globals, arrays, element pointers, and helper parameters to
numeric leaves. Structs, fixed arrays, and selected pointer-bearing helper values use one recursively
proved provider representation in address space 3.

The bounded target is five healthy workloads sharing this storage invariant: frozen
`groupshared-struct-with-interface` and discovery `array-size-groupshared`,
`generic-groupshared`, `ptr-to-groupshared`, and `groupshared-ptr-of-device`. Each later failure will
be recorded by exact canonical producer and shape rather than widened speculatively.

## Progress

- [x] (2026-08-31) Reconciled Slice 154 Pareto rows and separated group-shared storage from
  thread-local wave context, entry-point ABI, and unrelated generic-pointer failures.
- [x] (2026-08-31) Inspected the five source contracts and final linked IR for
  `ptr-to-groupshared`: a `RateQualified(GroupShared, Ptr<Data>)` global flows directly to a helper
  parameter `Ptr<Data, GroupShared, DefaultLayout>` and typed field addresses.
- [x] (2026-08-31) Defined one exact finite group-shared storage descriptor and replaced the
  numeric-only global, array, element-pointer, and helper-pointer restrictions that consumed the
  same representation.
- [x] (2026-08-31) Built and ran all five motivating workloads at O0/O3, promoted the four stable
  correct representatives, and classified the later `UserPointer` provider failure separately.
- [x] (2026-08-31) Ran the selected unit prefix, both complete corpora, representative
  measurements, integrity checks, documentation, and self-review for Slice 155.

## Surprises and Discoveries

- The two static-group-shared-array tests intentionally exercise core-only `Ref<T>` patterns and
  already contain comments discouraging ordinary user reliance. Their final numeric arrays are
  nevertheless canonical shared globals; this slice must not add special handling for `Ref<T>`.
- `ptr-to-groupshared` has no representation ambiguity. The final global's pointee is the same
  finite `Data { int, int }` helper struct named by the helper parameter, and field-address results
  preserve address space 3 with scalar layout.
- `groupshared-ptr-of-device` stores `UserPointer<int>` values inside a fixed shared array. The
  existing helper-value algebra already represents each pointer generically; the missing piece is
  allowing that proven helper leaf at a shared storage boundary.
- Frozen `groupshared-struct-with-interface` contains the fixed tuple produced by existential
  lowering plus a float. It is a stronger aggregate-transport gate than the two numeric arrays.
- Canonical group-shared global pointers have two spellings after legalization: a one-operand
  `Ptr<T>` and an exact three-operand `Ptr<T, ReadWrite, Generic>`. In both cases the
  `IRGroupSharedRate` on the global, rather than the pointer spelling, is the source of physical
  address-space-3 provenance.
- `IRGetElementPtr` can see the same one-operand pointer spelling for local and shared arrays, so
  the canonical global producer must be classified before type-only local-array alternatives.
- `groupshared-ptr-of-device` now passes compiler preflight and reaches the independent provider
  operation that converts an address-space-1 pointer value to the generic `UserPointer`
  representation. The provider returns `-2147024809`; that conversion is not widened here.
- The repository formatting check runs but this machine does not provide `gersemi`,
  `clang-format`, `prettier`, or `shfmt`. The touched C++ and directives were manually reviewed,
  and `git diff --check` is clean.

## Decision Log

- Decision: treat group-shared storage as the existing finite helper-value algebra, not a new
  parallel type system.
  Rationale: helper values already recursively classify copyable aggregates and selected pointer
  leaves, calculate their alignment, and lower them to executable LLVM values. CUDA address space
  changes pointer provenance, not the pointee representation.
  Date/author: 2026-08-31, Codex.
- Decision: keep thread-local `Ptr<KernelContext, addressSpace=1>`, entry-point parameters, and
  double-indirect generic pointers out of this slice.
  Rationale: they have different address spaces or ABI producers and are not children of a
  group-shared global/element producer.
  Date/author: 2026-08-31, Codex.
- Decision: keep provider ABI revision 30.
  Rationale: global declarations, typed pointers, aggregate load/store, field GEP, and sequential
  element GEP already exist in the generic provider interface.
  Date/author: 2026-08-31, Codex.

## Outcomes and Retrospective

One `NVVMSharedGlobal` descriptor now owns recognition of uninitialized group-shared storage,
canonical pointer spelling, finite storage type, and executable alignment type. The same finite
helper-value algebra is reused by explicit helper pointers and scalar-layout element pointers.
Sequential array access and struct-field access retain exact source types while deriving physical
address space 3 from the canonical group-shared producer. Direct helper calls admit only the
global itself or an explicitly typed group-shared pointer; no root-address search or syntax
reconstruction remains.

The frozen corpus remains exactly 452 workloads/427 healthy references and reaches 381 O0, 385 O3,
and 381 correct in both modes, up from 380/384/380 with zero old-correct regressions. Discovery
remains exactly 82/72 and reaches 58/58/58, up from 55/55/55 with zero old-correct regressions.
The frozen interface aggregate and three discovery workloads are newly correct in both modes and
receive eight permanent direct lanes. `groupshared-ptr-of-device` advances to the separately owned
provider address-space-conversion failure. The selected prefix passes 427/427.

All established representative direct-O3 gates assemble with CUDA 12.9 for SM70, SM80, and SM90.
For the existential gate, the exploratory SM70 measurements are 287.2 ms and 1007-byte PTX for
direct O3 versus 385.3 ms and 8946-byte PTX for NVRTC O3; direct O0 is 268.6 ms and 60001-byte PTX.
Provider ABI revision 30 remains unchanged.

## Context and Current Pipeline

Consider this source shape:

```slang
struct Data { int x; int y; }
groupshared Data shared;
void read(Ptr<Data, Access::ReadWrite, AddressSpace::GroupShared> p)
{
    output[0] = p.x;
}
```

Address-space specialization produces a module-scope
`global_var : RateQualified(GroupShared, Ptr<Data>)`. The call passes that exact global to a helper
parameter `Ptr<Data, ReadWrite, GroupShared, DefaultLayout>`. Field selection produces
`Ptr<int, ReadWrite, GroupShared, ScalarLayout>`.

Direct NVVM already lowers `Data` as a first-class helper struct and has typed provider operations
for shared global declaration, aggregate store/load, helper calls, and struct-field pointers. Its
classifiers currently admit only selected numeric shared scalars, numeric fixed arrays, numeric
shared element pointers, and numeric shared helper pointers. Those independent restrictions reject
the canonical aggregate before provider mutation.

The owning boundary is shared storage classification. It should prove a finite helper value once,
retain the canonical source type on every pointer relation, and select LLVM address space 3 for the
pointer. It must not infer shared provenance from arbitrary pointers or reconstruct a type from
syntax.

## Scope and Non-Goals

In scope are finite helper-value group-shared globals, fixed arrays, exact shared pointers used as
helper parameters/body values, sequential array elements, struct fields, aggregate loads/stores,
and the five motivating workloads.

Out of scope are thread-local globals, arbitrary generic or double-indirect pointers, entry-point
ABI, dynamically sized shared memory, recursive/infinite values, unsupported resource leaves,
non-default shared pointer layouts, `Ref<T>` syntax reconstruction, provider callbacks, ABI
revision, frozen-corpus identity changes, and corpus v2.

## Architecture and Invariants

- Every admitted shared pointee is an existing finite `isNVVMSupportedHelperValueType`.
- A group-shared global owns the address-space-3 provenance used by its exact derived pointers.
- Fixed arrays have a positive canonical count and an executable element representation.
- Shared helper and element pointers preserve exact pointee type, read-write access, address space
  3, and their producer-owned default/scalar data layout.
- Recursive aggregate layout uses existing helper alignment and CUDA/LLVM compatibility checks.
- No accepted pointer changes address space or gains mutability through downstream inference.
- Frozen corpus v1 and discovery retain separate exact identities and denominators.

## Interfaces and Dependencies

`source/slang/slang-emit-nvvm-type-lowering.h/.cpp` owns shared global/pointer classification and
role-sensitive lowering. `source/slang/slang-emit-nvvm.cpp` owns exact producer relations,
preflight, and emission. Existing revision-30 builder operations remain sufficient; no public API,
provider callback, or external dependency change is planned.

## Milestones

1. Replace the scalar/array numeric shared-global split with one exact descriptor for finite helper
   storage while preserving array count and canonical storage type.
2. Generalize shared helper and scalar-layout element pointers to exact helper-value pointees, then
   route them through existing address-space-3 pointer lowering.
3. Reuse the generalized classification in global collection, value validation, sequential/field
   pointer resolution, loads/stores, and emission without adding fixture or syntax checks.
4. Run the five targets against native NVRTC at direct O0/O3. Promote only stable correct semantic
   combinations and capture later first blockers.
5. Run the 427-test selected prefix, complete frozen v1/discovery corpora, and SM70/80/90 gates.
   Update separate artifacts, report, design, ledger, and this plan; self-review and commit.

## Validation and Acceptance

All CMake builds and tests run outside the sandbox with Windows-native tools as required by
`AGENTS.md`. Acceptance requires:

- final linked IR retains the canonical group-shared global and exact typed pointer chain;
- every admitted pointee satisfies the existing finite helper-value classifier;
- the provider sees address-space-3 pointers and typed aggregate operations, not reconstructed or
  bitcast storage;
- every promoted workload is differentially correct at direct O0 and O3;
- frozen v1 remains exactly 452/427 and discovery exactly 82/72, with separate O0/O3/both,
  classifications, Pareto, and zero old-correct regression;
- the selected prefix passes and representative O3 PTX assembles for SM70/80/90;
- provider ABI revision remains 30; and
- artifact integrity and `git diff --check` pass without staging `external/slang-binaries/`.

## Failure and Recovery

If a target reaches a later failure, record its exact producer and keep only independently proven
shared-storage behavior. If helper-value and shared-storage layouts differ, reject that aggregate
at the shared declaration boundary rather than padding or converting downstream. Generated probes
and corpus output under `build/` are reproducible and remain untracked.

## Artifacts and Hand-Off

Commit this completed plan with the implementation because the user explicitly requires them
together. Retain final-IR probes and measurement outputs under `build/`; commit stable direct lanes,
Slice 155 corpus snapshots, the five-part report, and durable design/ledger updates. The report must
trace every retained pointer shape from its canonical global/element producer through helper or
memory consumers.
