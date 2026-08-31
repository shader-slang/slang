# Preserve canonical loaded parameter-group value pointers

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each experimental NVVM slice plan to be committed with its implementation, overriding the
repository's normal working-log exclusion for this branch.

## Purpose and Observable Result

After this slice, direct NVVM accepts a whole parameter-block or constant-buffer element load when
the pointer is the canonical result of loading that exact parameter-group wrapper from an already
admitted struct field. The wrapper is an immutable pointer to its specialized element in the CUDA
and provider representations. Preflight proves the complete producer chain and exact element type;
emission uses the existing typed load and marks the element read invariant.

The observable goal is O0/O3 differential runtime correctness for the four healthy discovery
workloads currently stopped at `device scalar pointer: producer=load, consumer=load`, without
admitting arbitrary loaded pointer values or changing provider ABI revision 30.

## Progress

- [x] (2026-08-31) Reconciled the Slice 148 Pareto and selected the four-row loaded
  parameter-group pointer cluster as the largest exact discovery blocker.
- [x] (2026-08-31) Captured final linked IR for scalar, nested resource, and tuple parameter groups;
  each uses `IRFieldAddress -> IRLoad<ParameterGroup<T>> -> IRLoad<T>`.
- [x] (2026-08-31) Added one exact producer classifier and recursive representation-compatibility
  proof shared by preflight and emission, plus focused positive and compact-vector negative
  coverage.
- [x] (2026-08-31) Promoted three distinct stable representatives after real-provider O0/O3
  differential runtime validation; the fourth measured row advanced to its next exact blocker.
- [x] (2026-08-31) Ran the complete frozen-v1 and discovery censuses, selected regressions, and
  exploratory PTX/assembly measurements; completed docs and the principled-change self-review.
- [x] (2026-08-31) Completed the Slice 149 validation, documentation, and staged-scope audit; the
  slice commit is the final hand-off transaction before selecting Slice 150.

## Surprises and Discoveries

- `ParameterBlock<T>` and `ConstantBuffer<T>` are semantic wrappers in linked Slang IR, but
  `NVVMTypeLoweringContext::_lowerParameterGroupType` already represents either wrapper as an LLVM
  global pointer to `T` lowered for `ParameterGroupStorage`. The second `IRLoad` is therefore an
  ordinary provider load once preflight proves that semantic pointer role.
- The four rows are not merely type-alike. `parameter-block-unify.slang` loads a parameter group
  from the synthesized conventional global struct; `generic-shader-object-cbuffer.slang` first
  loads a constant buffer and then loads a nested parameter block from one of its fields;
  `tuple-parameter.slang` loads a texture/sampler tuple through a constant buffer. In every case,
  `_getNVVMStructFieldAddress` already proves the root and exact declared field type.
- Parameter-group storage and ordinary value representations are not universally identical.
  Compact 32-bit vector3 storage uses an array while its value uses an LLVM vector, and stored
  device `UserPointer` leaves use global pointers while ordinary helper values use generic
  pointers. Whole-element loads must reject such elements until a canonical conversion exists.

## Decision Log

- Decision: classify an element pointer only as an `IRLoad` of a supported parameter-group type
  whose operand is an `IRFieldAddress` resolved by `_getNVVMStructFieldAddress` and whose declared
  field type exactly equals the loaded wrapper type.
  Rationale: this proves both the canonical producer and root storage role without walking an
  arbitrary operand graph or accepting every SSA value that happens to lower to an LLVM pointer.
  Date/author: 2026-08-31, Codex.
- Decision: require a recursive storage/value representation-compatibility predicate for `T`.
  Rationale: an LLVM load returns the physical `ParameterGroupStorage` pointee. It can be recorded
  directly as semantic `T` only when every recursive leaf has the same provider representation.
  Compact vector3 and global-to-generic `UserPointer` leaves are explicit counterexamples.
  Date/author: 2026-08-31, Codex.
- Decision: retain provider ABI revision 30 and use `emitLoad`.
  Rationale: the loaded wrapper already has the exact LLVM pointer type; no provider operation is
  missing.
  Date/author: 2026-08-31, Codex.

## Outcomes and Retrospective

Slice 149 unlocks three healthy discovery workloads at both O0 and O3:

- `language-feature/generics/generic-shader-object-cbuffer.slang`;
- `language-feature/generics/parameter-block-unify.slang`;
- `language-feature/tuple/tuple-parameter.slang`.

`bugs/parameter-block-load.slang` crosses the load-to-load parameter-group pointer shape and stops
at its next independent `sequential element pointer: Ptr<uint, ...>` blocker. It is not counted as
correct and receives no permanent direct directive.

Frozen corpus v1 remains exactly 452 rows and 427 healthy MVP references at 371 O0, 375 O3, and
371 both-mode successes, with zero old-correct regression. Discovery remains 82 selected rows and
72 healthy references, improving from 47/47/47 to 50/50/50 with zero old-correct regression. The
selected prefix passes 425/425. CUDA 12.9 `ptxas` accepts all eight measured discovery workloads at
direct O3 for SM70, SM80, and SM90. Provider ABI revision 30 remains unchanged.

The former four-row healthy `device-pointer-load-chain` cluster is eliminated. One row advances
into `aggregate-sequential-pointer`, making that and typed struct-field pointers the leading
remaining aggregate/pointer shapes. This is useful evidence for Slice 150 selection, but it is not
part of this slice's implementation.

## Context and Current Pipeline

Consider the linked form of this ordinary source operation:

```slang
struct Params { ParameterBlock<Impl> implementation; }
ConstantBuffer<Params> params;
Impl value = params.implementation;
```

CUDA specialization and entry-uniform collection produce this canonical sequence:

```text
Ptr<ConstantBuffer<Params>> = get_field_addr(globalParams, params)
ConstantBuffer<Params> = load(...)
Ptr<ParameterBlock<Impl>> = get_field_addr(..., implementation)
ParameterBlock<Impl> = load(...)
Impl = load(...)
```

The first validation pass already admits both loads' result types. The dominance pass calls
`_validatePointerValue` for the final `load Impl`; that function accepts explicit IR pointer types
and field/element producers but does not recognize the semantic `ParameterBlock<Impl>` value as the
provider pointer created by `_lowerParameterGroupType`. Emission already lowers the wrapper load to
that provider pointer and would call `NVVMIRBuilder::emitLoad` for `Impl`.

## Scope and Non-Goals

In scope are exact loaded parameter-group pointer classification, recursive storage/value
representation compatibility, immutable element loads, focused invalid-neighbor diagnostics,
promotion of useful discovery representatives, separate full corpus metrics, and durable docs.

Out of scope are arbitrary pointer-valued loads, writable parameter-group elements, parameter
groups produced by calls/phis/selects, compact vector3 or `UserPointer` storage conversion, helper
ABI generalization, array/field root widening, new provider callbacks, fixture-name dispatch,
syntax reconstruction, compatibility fallback, or changing corpus-v1 membership.

## Architecture and Invariants

- `IRFieldAddress` plus `_getNVVMStructFieldAddress` owns root and field provenance.
- The loaded field's declared type must be the exact `ParameterBlock<T>` or `ConstantBuffer<T>`
  type on the first `IRLoad`.
- The second load's expected pointee must be exactly the specialized `T`.
- `T` must have identical recursive `ParameterGroupStorage` and `Value` provider representations.
- A loaded parameter-group element pointer is read-only. Only an `IRLoad` consumer is admitted.
- Dominance/availability validation remains mandatory for the first load.
- Preflight and emission share the classifier, and emission marks the element load invariant.
- Existing generic builder operations express the complete lowering.

## Interfaces and Dependencies

Implementation remains compiler-side in `source/slang/slang-emit-nvvm.cpp` and the existing NVVM
type-classification module. A new internal predicate may be declared in
`slang-emit-nvvm-type-lowering.h` only if both preflight and emission need it. No public API,
provider callback, ABI revision, external dependency, or third-party corpus changes.

Stable repository shaders receive direct O0/O3 directives only after real-provider differential
runtime correctness. The exact 452 corpus-v1 identities continue through
`--workload-ids-from issue-nvvm-backend/census.slice-146.tsv`; discovery retains its separate
82-workload selection and 72 healthy denominator.

## Milestones

1. Capture the final linked producer chain for all four discovery rows and enumerate the exact
   element types and first downstream consumers.
2. Add a finite recursive compatibility predicate that rejects every known physical/value
   representation difference rather than comparing opaque provider handles after mutation.
3. Add one resolver for the exact loaded parameter-group pointer chain. Use it in
   `_validatePointerValue` to admit only immutable `IRLoad<T>` and in emission to select invariant
   load flags.
4. Run all four discovery rows at O0/O3. Record correct results or their next exact blocker; promote
   only stable representatives that protect distinct scalar/resource/nested/tuple combinations.
5. Run selected regression, complete frozen/discovery censuses, representative PTX assembly and
   measurements, formatting, diff hygiene, self-review, and commit Slice 149.

## Validation and Acceptance

Run every CMake build and test outside the sandbox with Windows-native tools. Acceptance requires:

- a focused negative test proving compact vector3 or device-pointer-bearing parameter-group
  elements do not cross the identity-only path;
- real O0/O3 differential runtime results for all four measured discovery rows;
- every promoted direct lane passing;
- the selected direct-NVVM prefix with no regression;
- frozen corpus v1 remaining exactly 452/427 with O0/O3/both and zero old-correct loss;
- discovery reporting the unchanged 82/72 selection, classifications, Pareto, and exact newly
  unlocked identities;
- direct O3 PTX assembly for representative SM70, SM80, and SM90 configurations;
- provider ABI revision 30 unchanged;
- Python/JSON syntax, formatting, `git diff --check`, and staged-scope validation.

## Failure and Recovery

If a newly admitted load reaches a provider type error, compare the recursive storage/value
spelling and narrow or correct the compatibility predicate; do not bitcast or retry the load. If a
row advances to another first blocker, retain that exact diagnostic and do not widen the slice.
Generated probes/census/measurement outputs stay below `build/` and can be regenerated safely.

## Artifacts and Hand-Off

Commit this completed plan, implementation, focused/permanent tests, Slice 149 frozen and discovery
TSV/JSON evidence, five-part report, and durable design/ledger updates. Keep raw IR dumps, logs,
PTX, and cubins under `build/`. The report must name all newly correct and merely advanced rows and
keep corpus-v1 and discovery percentages separate.
