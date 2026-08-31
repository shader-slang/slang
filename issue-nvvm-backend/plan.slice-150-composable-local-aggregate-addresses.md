# Compose canonical local aggregate address paths

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires the completed slice plan to be committed with this experimental backend.

## Purpose and Observable Result

After this slice, direct NVVM accepts a struct-field address when its base is either an already
admitted local/helper reference root or the exact `IRGetElementPtr` selecting a struct from an
admitted fixed aggregate. Every path is proved from canonical producers, exact pointee identity,
and inherited access; the emitter continues to use the existing generic struct-field-pointer
operation and provider ABI revision 30.

The slice targets `compute/pointer-emit.slang` in discovery and
`language-feature/pointer/const-ref.slang` in frozen corpus v1. It does not treat every typed
field-address diagnostic as one root cause: global parameter blocks rejected because one sibling
field has an unsupported representation, resource-bearing structured-buffer elements, and shared
interface storage remain separate work.

## Progress

- [x] (2026-08-31) Reconciled the Slice 149 Pareto and inspected final linked IR for all three
  healthy discovery field-address rows plus the related frozen-v1 aggregate/pointer rows.
- [x] (2026-08-31) Separated conventional-global admission failures from local aggregate
  composition: `gh-6657-bindless-uniform` first needs descriptor-handle buffer storage, and
  `type-legalize-bug-1` first needs its parameter-group element representation.
- [x] (2026-08-31) Reused the exact local-copyable, helper-reference, and sequential-element
  classifiers in shared field resolution while preserving root access and provider ABI 30.
- [x] (2026-08-31) Added focused fake-provider coverage and proved O0/O3 real-provider correctness
  for `pointer-emit` and `pointer/const-ref`; promoted four stable direct lanes.
- [x] (2026-08-31) Completed the 426/426 selected regression, both complete corpora, nine-workload
  SM70/80/90 measurements, artifact generation, durable documentation, and self-review.
- [x] (2026-08-31) Finalized the completed implementation, plan, report, and evidence for the
  required Slice 150 commit.

## Surprises and Discoveries

- The discovery cluster `aggregate-struct-field-pointer` groups diagnostics by rejected operation,
  not by root representation. `gh-6657-bindless-uniform` and `type-legalize-bug-1` both carry the
  correct compiler-synthesized `GlobalParams` marker; their collected blocks fail because a
  different field is outside the admitted storage algebra. Widening field provenance would merely
  mask those earlier type contracts.
- `compute/pointer-emit.slang` first fails on
  `IRFieldAddress(IRGetElementPtr(local-array, index), field)`. The local array root and sequential
  element producer are each already admitted, but `_getNVVMStructFieldAddress` cannot consume the
  proven sequential result as its parent.
- `pointer/const-ref.slang` begins with direct local struct fields and later selects through a
  canonical `BorrowInParam<Thing>`. Helper ABI and pointer validation already admit that reference;
  field resolution does not yet reuse the same root-role classifier.
- The builder already expresses every path through `emitSequentialElementPointer` followed by
  `emitStructFieldPointer`. No provider callback or ABI change is indicated.

## Decision Log

- Decision: define Slice 150 by compositional local aggregate provenance, not the census cluster
  name.
  Rationale: producer, root role, and exact type relation are the reusable invariant; the global
  failures need different storage representations.
  Date/author: 2026-08-31, Codex.
- Decision: reuse the existing sequential-element and helper-reference classifiers from both
  preflight and emission.
  Rationale: these classifiers already prove canonical producer spelling, address space, layout,
  access, and pointee identity. A second structural matcher would create a competing source of
  truth.
  Date/author: 2026-08-31, Codex.
- Decision: preserve root mutability and reject writes through immutable borrows.
  Rationale: field selection cannot widen access. The existing pointer consumer validation owns
  this invariant once the resolver reports the inherited role.
  Date/author: 2026-08-31, Codex.
- Decision: keep provider ABI revision 30.
  Rationale: the existing generic operations already model the canonical chain.
  Date/author: 2026-08-31, Codex.

## Outcomes and Retrospective

Frozen corpus v1 remains exactly 452 workloads and 427 healthy MVP references. O0/O3/both
correctness reaches 372/376/372, up by one in each metric, with zero old-correct regression. The
newly correct identity is `language-feature/pointer/const-ref.slang#cuda-1`. All-tier direct
preflight failures fall from 53 to 52 in each mode; the other classification populations are
unchanged. The selected prefix passes 426/426.

Discovery remains 82 workloads and 72 healthy native references. O0/O3/both correctness reaches
51/51/51, up by one with zero old-correct regression. The newly correct identity is
`compute/pointer-emit.slang#discovery-1`. The remaining typed field-address group contains two
healthy conventional-global rows whose complete collected storage types need separate work, plus
one non-healthy texture row. Seven other healthy two-row clusters remain tied for the lead.

The four promoted direct lanes pass 4/4. The complete `pointer/const-ref` file also observes an
unrelated existing WGPU synthetic-lane failure on this machine; native CUDA and both direct lanes
pass. Three standalone measurements for `pointer-emit` give 347.1 ms/8584-byte PTX for NVRTC O3,
237.4 ms/8667 bytes for direct O0 SM70, and 242.3 ms/645 bytes for direct O3 SM70. CUDA 12.9
`ptxas` accepts that direct O3 output and eight established discovery gates at SM70, SM80, and
SM90. Provider ABI revision 30 remains unchanged.

## Context and Current Pipeline

`IRBuilder::emitElementAddress`/`IRGetElementPtr` produces a typed pointer to a selected aggregate
element. `_getNVVMSequentialElementPointer` proves the admitted array, index, result type, root
access, and storage representation. A following `IRBuilder::emitFieldAddress` selects a member of
that exact struct. `_validateNVVMFunction` and final emission both call
`_getNVVMStructFieldAddress`; emission then calls `NVVMIRBuilder::emitStructFieldPointer`.

The resolver currently composes through a parent `IRFieldAddress`, but not through an already
proved sequential element pointer. It also recognizes mutable local/resource helper roots but does
not reuse the exact immutable helper-reference classifier for member selection.

Frozen corpus v1 remains fixed at 452 workloads and a 427 healthy-MVP denominator, with Slice 149
O0/O3/both correctness 371/375/371. Discovery remains 82 workloads and 72 healthy native
references, with Slice 149 O0/O3/both correctness 50/50/50. Neither identity set or denominator may
change.

## Scope and Non-Goals

In scope are exact field selection from an admitted sequential struct-element pointer, exact field
selection from admitted helper references, inherited mutability, focused fake-provider coverage,
real O0/O3 differential validation, selective promotion, complete cross-corpus measurement, and
durable documentation.

Out of scope are descriptor handles in structured-buffer storage, empty or recursive parameter
group elements, groupshared interface aggregates, entry/helper ABI widening, arbitrary operand
graph traversal, structural type reconstruction, provider callbacks, compatibility fallbacks,
fixture-name checks, diagnostic weakening, corpus-v1 changes, or corpus-v2 declaration.

## Architecture and Invariants

- Every admitted field is produced by `IRFieldAddress` and found by semantic field key.
- A sequential parent must be accepted by `_getNVVMSequentialElementPointer`, and its exact result
  pointee must be the struct that declares the selected field.
- A helper-reference root must be an exact admitted parameter producer; its access qualifier is
  inherited by the selected field.
- Field result pointee identity must equal the declared field type.
- Preflight and emission call the same resolver. No accepted path can bypass typed consumer
  validation.
- The existing generic provider operations and ABI revision 30 remain sufficient.
- Frozen corpus-v1 and discovery results stay separate.

## Interfaces and Dependencies

Implementation is limited to direct-NVVM classification/emission and focused unit fixtures unless
an upstream producer audit proves otherwise. Permanent test directives may be added only to stable
repository workloads that become correct at both O0 and O3. Census scripts and manifests remain
unchanged unless measurement reveals a deterministic tooling defect.

## Milestones

1. Capture exact linked IR and assert that the rejected bases are canonical sequential-element or
   helper-reference producers with exact struct pointees.
2. Refactor field-address root resolution to reuse those established classifiers and propagate
   access without duplicating type logic.
3. Add focused positive coverage plus a negative proof that immutable references cannot be used
   for stores.
4. Run candidate real-provider probes at O0/O3, promote useful correct workloads, then run selected
   and full regression measurement.
5. Update plan/report/design/ledger and census artifacts, audit every helper/special case, and
   commit Slice 150.

## Validation and Acceptance

Run all CMake builds and tests outside the sandbox with Windows-native tools. Acceptance requires:

- focused unit coverage reaches the existing generic field-pointer operation for canonical local
  aggregate chains and preserves immutable access;
- every promoted workload is differentially correct through the real provider at O0 and O3;
- the selected direct-NVVM regression prefix passes with zero old-correct regression;
- complete frozen-v1 reporting keeps 452/427 fixed and reports O0, O3, and both;
- complete discovery reporting keeps 82/72 fixed and reports classifications, Pareto, and newly
  unlocked workloads;
- no provider ABI change;
- formatting with an approved repository formatter when available, `git diff --check`, JSON/TSV
  integrity, and self-review of every new helper/fallback/special case;
- no staged content from `external/slang-binaries/`.

## Failure and Recovery

If a candidate reaches a different first blocker, record the cascade and do not widen this slice
unless the new shape is another instance of the same exact compositional invariant. If a provider
failure appears, inspect the typed builder operands before changing classification. Generated
probe/census output under `build/` is reproducible and may be discarded; corpus snapshots are
updated only after complete successful runs.

## Artifacts and Hand-Off

Commit the completed plan, implementation, focused tests, promoted stable directives, Slice 150
frozen-v1 and discovery snapshots/reports, and durable design/ledger updates. The report must name
advanced-but-not-correct rows and their next first blocker separately from newly correct rows.
