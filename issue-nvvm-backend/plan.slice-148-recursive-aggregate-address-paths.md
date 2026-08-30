# Preserve canonical recursive aggregate address paths

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires the completed slice plan to be committed with the implementation for this experimental
backend.

## Purpose and Observable Result

After this slice, direct NVVM accepts a field-address chain whenever every step is proven by the
canonical `IRFieldAddress` producer, the parent field's exact aggregate type, and one already
admitted root storage role. The chain preserves the root's mutability instead of requiring every
parent address to be mutable. Canonical CUDA layout and the direct-NVVM storage algebra agree that
a nested parameter block or constant buffer is a pointer-sized field whose specialized element
type remains in the reachable type closure. This unlocks nested parameter-block and constant-buffer
fields without adding a provider callback or admitting unrelated pointer or ABI shapes.

The slice promotes only stable representatives that prove the recursive invariant. It reports
frozen-v1 and discovery metrics separately and leaves both denominators unchanged.

## Progress

- [x] (2026-08-30) Reconciled the Slice 147 Pareto and separated recursive field addressing from
  array relation, pointer-load, entry ABI, helper ABI, and physical-layout failures.
- [x] (2026-08-30) Probed final linked IR for nested parameter-block access and traced the first
  rejection to `_getNVVMStructFieldAddress` requiring a recursively resolved parent to be mutable.
- [x] (2026-08-30) Generalized recursive field-address provenance, parameter-group aggregate
  storage validation/layout, and reachable nested element types without changing provider ABI 30.
- [x] (2026-08-30) Proved real-provider O0/O3 runtime correctness and promoted the nested
  parameter-block and constant-buffer representatives; retained exact failures for adjacent shapes.
- [x] (2026-08-30) Added exact frozen-workload selection to the census runner after those promoted
  directives correctly appeared in discovery but must not change corpus-v1 membership.
- [x] (2026-08-30) Completed frozen-v1 and discovery censuses, 424/424 selected regression,
  promoted lanes, exploratory SM70/80/90 assembly, formatting, and self-review; updated durable
  evidence for commit.

## Surprises and Discoveries

- The discovery Pareto's 13 aggregate/pointer/layout rows are not one implementation shape. Seven
  first stop at `struct field address`; the rest stop at array-element relation, pointer-load
  chains, sequential pointer typing, or storage layout. Treating the broad family as one slice
  would combine unrelated invariants.
- In `bindings/nested-parameter-block-2.slang`, final linking produces a canonical sequence such as
  `load ParameterBlock<Scene>`, `get_field_addr Scene.sceneCb`, then `get_field_addr CB.value`.
  `_getNVVMStructFieldAddress` proves the first address but rejects the second solely because the
  parameter-group root is immutable. The child field belongs to the exact parent aggregate and is
  also immutable; no new representation is required.
- The existing generic builder already expresses every step with `emitStructFieldPointer` and the
  type-lowering cache already distinguishes parameter-group storage. This is compiler-side
  classification/legalization work, not a provider ABI gap.
- Native CUDA emits a nested parameter group as a pointer field such as
  `MaterialSystem_0* material_0`. `IRTypeLayoutRules::calcSizeAndAlignment` previously had no CUDA
  parameter-group case, so the canonical producer could not derive offsets for the enclosing
  `Scene` aggregate even after recursive provenance was fixed.
- The legacy fake provider records one hard-coded struct vocabulary and cannot distinguish the
  nested specialized struct identities involved here. Expanding that model would duplicate the
  real provider's generic type system. Real LLVM-provider compilation, PTX, differential runtime,
  and permanent repository lanes own this representation proof instead.
- The fixed sampler-array expected-failure source also compiles after exact sampler handles become
  valid pointer-sized aggregate storage leaves. Keeping it negative would contradict the same CUDA
  storage representation exercised by `cbuffer-legalize.slang`.

## Decision Log

- Decision: define the slice by recursive field-address provenance, not by the broad aggregate
  Pareto label.
  Rationale: a reusable invariant must state the exact accepted producer, type relation, and root
  storage role; array and ABI failures do not share those facts.
  Date/author: 2026-08-30, Codex.
- Decision: preserve mutability from the recursively resolved parent instead of requiring it to be
  true.
  Rationale: `IRFieldAddress(parent, key)` cannot make immutable storage mutable. Exact field-key
  lookup plus equality between the parent's field type and the child's base aggregate proves the
  path for both mutable and immutable roots.
  Date/author: 2026-08-30, Codex.
- Decision: retain the existing generic provider interface and builder ABI revision 30.
  Rationale: lowering already emits a typed struct-field pointer for an admitted path; the rejected
  operation is expressible without new provider behavior.
  Date/author: 2026-08-30, Codex.
- Decision: represent only CUDA parameter groups as pointer-sized fields in the shared canonical
  layout query and recursively validate their specialized element storage.
  Rationale: this is the representation produced by native CUDA emission. Keeping it in the
  target-owned layout producer makes enclosing field offsets available to every downstream
  consumer without teaching direct NVVM a fixture-specific synthetic layout.
  Date/author: 2026-08-30, Codex.
- Decision: do not expand the legacy fake-provider struct recorder for nested specialized types.
  Rationale: its deliberately finite hard-coded type model cannot prove distinct arbitrary struct
  identities economically. The generic LLVM provider and differential runtime tests exercise the
  actual contract; preserving obsolete expected-failure sources would make the fake model drive
  production representation.
  Date/author: 2026-08-30, Codex.
- Decision: execute corpus v1 through an exact checked-in ID/source selector.
  Rationale: promoting useful discovery regressions adds four new direct directives to ordinary
  repository enumeration. Historical corpus-v1 rows must remain exactly 452, so selection now
  verifies every frozen identity and rejects missing, duplicate, or source-drifted rows.
  Date/author: 2026-08-30, Codex.

## Outcomes and Retrospective

The exact frozen selector retains all 452 corpus-v1 workloads, 448 eligible sources, and the 427
healthy-MVP denominator even though repository discovery now sees 456 workloads across 450 eligible
sources. Frozen O0/O3/both correctness remains 371/375/371 with zero old-correct regression. One
failure advances from field addressing to a separate raw structured-buffer pointer shape. The
selected direct-NVVM prefix passes 424/424.

The separate 82-workload discovery corpus retains 72 healthy native references and reaches
47/47/47 O0/O3/both correctness, up from 45/45/45. The newly correct identities are
`bindings/nested-parameter-block-2.slang` and `compute/cbuffer-legalize.slang`. Two other rows
advance to the now-leading four-row device-pointer-load chain; three healthy rows retain exact
typed field-address diagnostics. No old discovery success regresses.

Both promoted files pass their two permanent direct lanes. CUDA 12.9 `ptxas` accepts the two new
representatives and three established discovery gates at SM70, SM80, and SM90. The new direct-O3
SM70 compile medians/PTX sizes are 255.4 ms/1166 bytes and 265.1 ms/1032 bytes, compared with
native NVRTC O3 at 377.1 ms/9179 bytes and 384.2 ms/9005 bytes. These remain uncontrolled smoke
measurements. The provider ABI stays at revision 30.

## Context and Current Pipeline

`IRBuilder::emitFieldAddress` is the canonical producer of `IRFieldAddress`. Final direct-NVVM
preflight calls `_getNVVMStructFieldAddress`, which resolves the base role, struct declaration,
field key, result pointee type, and allowed field type. Emission calls the same resolver and then
uses `NVVMIRBuilder::emitStructFieldPointer`; loads and stores validate the resulting pointer.

The resolver already admits fields rooted in the conventional global parameter block, loaded
parameter groups, mutable local/resource/helper storage, device copyable-value pointers, and
selected structured-buffer elements. Its recursive branch recognizes a parent `IRFieldAddress`,
but currently proceeds only when `parentAddress.isMutable` is true. This rejects nested immutable
parameter-group storage even though the parent path and exact child aggregate type are canonical.

Frozen corpus v1 remains 452 workloads with a 427 healthy-MVP denominator and Slice 146
O0/O3/both correctness of 371/375/371. The separate Slice 147 discovery corpus has 82 selected
workloads, 72 healthy native references, and O0/O3/both correctness of 45/45/45. Neither identity
set or denominator may change in this slice.

## Scope and Non-Goals

In scope are recursive `IRFieldAddress` resolution through an already admitted parent path, exact
field/type validation, mutability propagation, CUDA parameter-group storage layout, reachable
specialized element types, selected real runtime promotion, exact frozen-corpus selection, full
cross-corpus O0/O3 measurement, and durable documentation.

Out of scope are entry-point or helper ABI generalization, `global_param`, pointer-valued loads,
array element relation, non-parameter-group storage layouts, provider address-space fixes, a new builder
callback, fixture-name dispatch, syntax reconstruction, compatibility fallback, diagnostic
weakening, changing corpus-v1 rows, or declaring corpus v2.

## Architecture and Invariants

- A recursive child is admitted only when its base is an `IRFieldAddress` already resolved by the
  same function.
- The parent result pointee must be an admitted aggregate struct and must equal the parent field's
  declared type. This uses canonical IR identity/equality rather than a custom equivalence rule.
- Field selection remains key-based through `_findNVVMStructField`; declaration order is used only
  to produce the LLVM struct index after the key is found.
- `isMutable` is inherited from the parent path. A child never widens access.
- Parameter groups occupy one pointer-sized CUDA storage leaf, while recursive validation and the
  reachable-type closure retain and prove the exact specialized element type behind that pointer.
- Existing field-type checks remain the owner of what can be loaded or stored at the leaf.
- Preflight and emission call the same resolver, so accepted shapes cannot diverge downstream.
- Corpus v1 and discovery artifacts, denominators, and percentages remain separate.

## Interfaces and Dependencies

The implementation uses the existing classifiers in `slang-emit-nvvm-type-lowering.*`, direct
preflight/emission in `slang-emit-nvvm.cpp`, and the canonical target layout query in
`slang-ir-layout.cpp`. Stable repository runtime representatives receive direct O0/O3 directives
only after actual correctness is established. The corpus-v1 runner accepts a checked-in frozen TSV
as an exact workload selector; discovery remains dynamically separate.

No public API, provider callback, ABI revision, external dependency, or third-party corpus changes.

## Milestones

1. Capture final linked IR and smallest failing fake-provider source for immutable and mutable
   nested paths. Confirm the rejection is exactly the recursive mutability guard.
2. Refactor `_getNVVMStructFieldAddress` so recursive resolution proves the parent aggregate/type
   relation and propagates root role/mutability. Keep all leaf field checks unchanged.
3. Remove obsolete fake-provider expected-failure sources rather than duplicating arbitrary nested
   struct identities in its hard-coded recorder. Prove the general contract through the real LLVM
   provider and differential O0/O3 runtime lanes.
4. Run the 13 healthy discovery aggregate/pointer rows and relevant frozen-v1 rows at O0/O3.
   Promote only stable representatives that prove a new recursive semantic combination.
5. Run selected/full regression validation and both complete censuses, update separate TSV/JSON
   snapshots and reports, audit every new helper/special case, and commit the completed slice.

## Validation and Acceptance

Run all CMake builds and tests outside the sandbox with Windows-native tools. Acceptance requires:

- focused unit coverage proving immutable and mutable nested field paths reach the existing builder;
- real O0/O3 runtime correctness for every promoted representative;
- the selected direct-NVVM regression prefix with no old-correct regression;
- a complete frozen-v1 rerun reporting O0, O3, both, and unchanged 452/427 denominators;
- a complete discovery rerun reporting the 72 healthy denominator, O0, O3, both, all classification
  totals, remaining Pareto, and exact newly unlocked identities;
- no provider ABI revision or callback;
- formatting, `git diff --check`, and self-review of every new helper/fallback/special case;
- no staged content from `external/slang-binaries/`.

## Failure and Recovery

If an accepted recursive path reaches provider failure, inspect the emitted LLVM operation and its
typed base before widening the resolver. If parent field type and child base aggregate do not match
canonically, stop at preflight and trace the producer; do not recreate types or add structural
matching. If a workload advances to a different first blocker, record that cascade and count it as
unlocked only when it becomes correct. Generated census/probe output remains ignored under
`build/` and can be regenerated.

## Artifacts and Hand-Off

Commit the implementation, focused tests, promoted stable test directives, new Slice 148 frozen-v1
and discovery snapshots/reports, durable design updates, and this completed plan. The report must
name every newly unlocked workload and the next first blocker for advanced rows, while keeping the
two coverage denominators separate.
