# Physical parameter-group helper references

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed experimental-backend slice plan to be committed with its implementation;
the completed plan therefore follows that established exception to the active working-log policy.

## Purpose and Observable Result

After this slice, methods on a layout-lowered constant-buffer value can receive the exact
immutable reference produced for its physical element type. The direct NVVM path must preserve one
parameter-group storage representation from the synthesized conventional global, through a
`ConstantBuffer<T>` load and direct helper call, into nested physical field addresses.

The bounded primary probe is discovery `optimization/arrray-storage-lowering`. It must execute
correctly through native CUDA and direct NVVM O0/O3 before promotion. Existing parameter-group,
immutable-reference, matrix-storage, local-copy, and helper-call workloads are regression gates.
Discovery `bugs/type-legalize-bug-1` remains a separate parameter-block resource-field cluster.

## Progress

- [x] (2026-09-01) Completed and committed Slice 173 as `005205203`; frozen v1 is
  413/413/413 over 427 and discovery is 70/70/70 over 72.
- [x] (2026-09-01) Re-ranked the two remaining healthy discovery failures and proved they have
  distinct canonical roots.
- [x] (2026-09-01) Captured the final linked IR for `arrray-storage-lowering` and traced the
  physical parameter-group element from global storage through its helper signatures and calls.
- [x] (2026-09-01) Defined the exact physical-storage reference, local, and device pointer roles
  and their producer relations without changing provider ABI revision 32.
- [x] (2026-09-01) Carried the bounded probe through every principled preflight/emission cascade
  without admitting ordinary aggregate references or weakening diagnostics.
- [x] (2026-09-01) Promoted stable O0/O3 coverage, regenerated both exact corpora, measured,
  documented, validated, self-reviewed, and prepared the exact Slice 174 commit.

## Surprises and Discoveries

- The discovery census stops first at `BorrowInParam<Params_natural, Read, Generic,
  DefaultLayout>`, while a direct `slangc -dump-ir` observation continues to the conventional
  global field-address diagnostic. Function-closure validation intentionally precedes global and
  instruction validation, so both are expected stages of the same vertical representation.
- CUDA buffer-element lowering decorates `Params_natural`, `Nested_natural`, and
  `DoubleNested_natural` as `PhysicalType` and gives them explicit sizes/offsets. Their helper
  methods receive immutable references rather than first-class logical aggregates.
- The entry point loads `ConstantBuffer<Params_natural>` from the synthesized `GlobalParams`
  block and passes that pointer directly to `Params.getVal`. Nested calls may instead pass a
  `TempCallArgImmutableVar` after loading a physical child. Both producers must retain the same
  pointee type but have distinct global and local provenance.
- The old aggregate-storage classifier treated every `PhysicalType` struct as a one-field array
  wrapper. The target's physical structs are ordinary nonempty multi-field storage records; only
  the separate physical-array wrapper classifier owns the one-array-field invariant.
- The exact physical matrix field producer retains an immutable semantic root but spells its
  `GetElementPtr` result as a read-write generic pointer. Preserving those as separate facts lets
  load emission retain invariant semantics while matching the producer's exact pointer type.
- Initial field resolution reused one output variable for two mutually exclusive classifiers.
  The second failed classifier cleared the first classifier's result, which surfaced as a null
  struct dereference. Giving each classifier its own output makes classifier state local and
  preserves the already-proved physical-reference type.

## Decision Log

- Decision: treat physical parameter-group helper references as their own role instead of
  broadening the ordinary helper-reference classifier to every aggregate-storage type.
  Rationale: the `PhysicalType` decoration and exact immutable four-operand `BorrowInParam`
  spelling identify a producer-owned external storage ABI. Ordinary helper values continue using
  their existing value representation and must not accidentally acquire storage layout.
  Date/author: 2026-09-01, Codex.
- Decision: keep provider ABI revision 32 unless a traced canonical operation cannot be expressed
  with existing typed pointer, address-space cast, load, aggregate, and field-pointer operations.
  Rationale: the observed gap is role classification and pointee representation, not a missing
  LLVM construction primitive.
  Date/author: 2026-09-01, Codex.
- Decision: carry an explicit physical-storage root bit through nested field resolution.
  Rationale: only the canonical physical-storage field producer emits the observed immutable-root
  plus read-write-result `GetElementPtr` combination. An explicit root role keeps that spelling
  from widening ordinary immutable aggregate fields.
  Date/author: 2026-09-01, Codex.

## Outcomes and Retrospective

Slice 174 unlocks discovery `optimization/arrray-storage-lowering` in both modes and promotes two
permanent direct lanes. Frozen v1 remains exactly 452 workloads/427 healthy references and
413/413/413, with zero changed row and zero old-correct regression. Discovery remains exactly
82/72 and advances from 70/70/70 to 71/71/71, with exactly one gain and no loss.

The selected prefix passes 433/433, the permanent `nvvm` category passes 78/78, and the focused
shader passes 4/4. The representative gate compiles and assembles through CUDA 12.9 for native
NVRTC, direct O0 SM70, and direct O3 SM70/SM80/SM90. Provider ABI revision 32 remains unchanged.

## Context and Current Pipeline

CUDA buffer-element lowering creates physical storage structs when logical matrix, Boolean, or
layout-sensitive fields cannot use the ordinary value spelling. Parameter-group lowering places a
global pointer to that physical element behind `ConstantBuffer<T>`. Transform-parameters-to-
constref turns aggregate method receivers into immutable `BorrowInParam<T>` helper parameters and
may create decorated local temporaries for nested calls. Direct NVVM preflight validates the
complete helper closure before conventional globals and function bodies, then type lowering maps
each accepted source role to a typed LLVM representation.

## Scope and Non-Goals

In scope are exact immutable generic `BorrowInParam` references to finite `PhysicalType` aggregate
storage, global parameter-group and decorated local argument producers, nested physical field
addresses and loads, the bounded discovery workload, adjacent negatives, exact corpus
regeneration, and representative measurements.

Out of scope are mutable physical-storage references, arbitrary `BorrowInParam` aggregates,
logical-to-storage syntax reconstruction, fixture-name checks, malformed upstream IR, fallback
paths, parameter-block resource fields, provider callbacks without a concrete operation gap, and
external workloads.

## Architecture and Invariants

- A physical storage reference is admitted only for the exact four-operand immutable
  `BorrowInParam<T, Read, Generic, DefaultLayout>` spelling whose pointee is a finite physical
  aggregate already accepted by the parameter-group storage algebra.
- Its provider pointee is lowered with the existing parameter-group-storage role, never inferred
  from a source type name or reconstructed logical type.
- A call argument may satisfy that parameter only when it is either the exact parameter-group
  pointer value to the same pointee or an exact generic local pointer produced for the same
  physical storage type. Provenance remains explicit.
- Field resolution preserves semantic immutability and uses the physical struct's canonical
  fields, offsets, array strides, and storage types. The exact physical array-field
  `GetElementPtr` result retains its producer-owned read-write access spelling separately.
- Unsupported shapes stop before provider mutation and retain producer/type/operation
  diagnostics.

## Interfaces and Dependencies

Exact classifiers and storage-role lowering live in
`source/slang/slang-emit-nvvm-type-lowering.{h,cpp}`. Global/field/call/pointer validation and
emission live in `source/slang/slang-emit-nvvm.cpp`. Permanent source coverage belongs on the
existing repository-local shader only after differential correctness. Existing revision-32
provider operations are preferred.

## Milestones

1. Preserve the final linked signature, global, call, local-temporary, and nested-field shapes.
2. Add one exact physical-storage-reference classifier and use it only at proven signature,
   lowering, call-relation, pointer-validation, and field-address boundaries.
3. Resolve each newly exposed cascade at its canonical producer/consumer boundary; stop if any
   operation requires a provider ABI change or an unprincipled downstream patch.
4. Promote the stable workload and run build, focused O0/O3 differential tests, selected prefix,
   permanent category, both exact corpora, and SM70/SM80/SM90 measurement.
5. Update design, ledger, five-part report, and this plan; format, audit, stage exactly the slice
   files excluding `external/slang-binaries/`, and commit.

## Validation and Acceptance

All builds/tests run outside the sandbox with Windows-native tools and the isolated Release
provider. Acceptance requires exact corpus identities 452/427 and 82/72; O0/O3 differential
results; zero old-correct regression; selected-prefix and permanent-category success; retained
negative diagnostic ownership; PTX assembly for the promoted gate; formatting; artifact integrity;
and an exact staged-file audit.

## Failure and Recovery

If the workload requires a representation not identified by `PhysicalType` plus the exact
immutable-reference spelling, stop and split the new root cause. If global and local producers
cannot share one typed pointer representation safely, retain only independently proven paths. Do
not declare physical storage copyable, rebuild logical syntax, or patch emitted LLVM text.

## Artifacts and Hand-Off

Keep dumps, PTX, and logs under ignored `build/nvvm-census` paths. Retain the completed plan only
with a committed result under the user's workflow exception. Distill durable representation rules
into `docs/design/nvvm-backend.md`, exact status into the capability ledger and separate corpus
artifacts, and every producer/input-shape decision into the Slice 174 five-part report.
