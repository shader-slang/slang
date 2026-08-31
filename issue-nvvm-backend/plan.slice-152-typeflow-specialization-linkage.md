# Keep existential specializations internally linked

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires the completed slice plan to be committed with this experimental backend.

## Purpose and Observable Result

After this slice, specialized function definitions have identities that reflect how they were
produced. A function cloned by typeflow for one existential call context is an internal
implementation variant rather than a second definition of the source function's external symbol.
A retained generic-pack specialization includes the exact canonical `IRTypePack` argument in its
specialized linkage digest. Modules can therefore contain these valid variants without presenting
duplicate LLVM function names to direct NVVM.

The slice targets the six frozen-v1 and two discovery workloads currently clustered as
`function-identity`. It fixes their shared canonical producer and then records any later blockers;
it does not rename functions in the NVVM emitter, synthesize fixture-specific suffixes, or admit
unrelated IR shapes exposed after the linkage invariant is repaired.

## Progress

- [x] (2026-08-31) Reconciled the Slice 151 Pareto and selected the eight-workload
  cross-corpus `function-identity` cluster.
- [x] (2026-08-31) Proved that the NVVM reachable-function walk already deduplicates function
  identity and that distinct reachable functions carry the same non-empty export name.
- [x] (2026-08-31) Captured the exact duplicate names for all eight workloads and traced the
  concrete clone to `lowerSpecializeExistentialsInFunc`.
- [x] (2026-08-31) Removed external linkage at the existential-specialization clone boundary;
  five workloads now compile through the real provider and one advances to an independent
  provider aggregate-pointer blocker.
- [x] (2026-08-31) Traced the two remaining collisions to specialized-linkage hashing through an
  empty `getTypeNameHint(IRTypePack)` spelling and added canonical type-pack spelling at that
  existing source of truth.
- [x] (2026-08-31) Added fourteen direct O0/O3 lanes across seven stable newly correct workloads;
  all promoted lanes pass through the real provider.
- [x] (2026-08-31) Ran all affected workloads at direct O0/O3 and captured the sole cascade as the
  provider-owned `by-value aggregate field pointer` operation without widening the slice.
- [x] (2026-08-31) Ran the 427/427 selected unit prefix, complete frozen-v1 and discovery corpora,
  and twelve-gate SM70/80/90 measurement set; updated artifacts and durable documentation.
- [x] (2026-08-31) Finalized diff, artifact-integrity, Python syntax, and promoted-lane
  checks; Slice 152 is ready to commit.

## Surprises and Discoveries

- `_visitNVVMFunction` maintains both an accepted-function set and completed-function set, so one
  `IRFunc*` cannot enter `_collectNVVMFunctionNames` twice. The collision is between distinct
  definitions, not a traversal bookkeeping error.
- In `static-method-dispatch.slang`, post-typeflow linked IR contains both
  `processStatic(ICompute)` lowered to the tagged `%Tuple` representation and
  `processStatic(MulOp)`. Both carry export
  `_SR28static_2Dxmethod_2Dxdispatch13processStatic...`, even though only the first is the externally
  named source function.
- `lowerSpecializeExistentialsInFunc` clones the complete base function with `cloneInst`, including
  its `IRExportDecoration`, and retains the base. This differs from other specialization producers:
  function-call specialization explicitly removes linkage from torn-off clones, and buffer element
  specialization removes linkage whenever original and specialized definitions coexist.
- Five frozen/discovery rows use the typeflow clone producer. After stripping clone linkage, all
  five compile through direct NVVM at O0/O3. `generic-interface-nested` advances to the independent
  provider diagnostic `by-value aggregate field pointer`.
- The variadic `size-of-tuple` and `variadic-pack-query-pack-conformance` rows use a second producer:
  `specializeLinkageDecoration` hashes each generic argument by `getTypeNameHint`, but that shared
  helper had no `IRTypePack` case. Both an empty pack and a multi-element pack therefore contributed
  the empty byte sequence and received the SHA-1 of empty input (`da39a3...`). The exact canonical
  pack contents, not a fixture or generated function ID, are the missing identity input.
- A broader test-server sweep was not valid regression evidence on this machine: its long-lived
  workers did not inherit `SLANG_NVVM_BUILDER_PATH`, and unrelated synthetic WebGPU lanes also
  failed during bind-group setup. The final rebuilt binary was instead checked with all fourteen
  promoted direct lanes in fresh processes, alongside the complete corpus runs and 427-test unit
  prefix.
- `extras/formatting.sh --check-only --modified` could not run because the Windows bash environment
  does not expose the repository's required `gersemi`, `clang-format`, `prettier`, or `shfmt` tools.
  The focused C++ changes were manually style-reviewed, Python syntax checks passed, and
  `git diff --check` passed.

## Decision Log

- Decision: fix the clone producer by removing linkage decorations from the existentially
  specialized clone.
  Rationale: the clone is cached under `IRSpecializeExistentialsInFunc` context and has a changed
  effective signature. It is an internal implementation variant, not an independent externally
  callable definition. Existing specialization passes establish the same ownership rule.
  Date/author: 2026-08-31, Codex.
- Decision: retain direct-NVVM duplicate-name rejection as an invariant check rather than assign a
  unique name to every collision in the emitter.
  Rationale: two externally linked definitions with one name are malformed upstream IR for every
  LLVM consumer. Renaming at emission would discard linkage semantics and make the producer bug
  invisible.
  Date/author: 2026-08-31, Codex.
- Decision: teach the existing `getTypeNameHint` source of truth to spell `IRTypePack` recursively,
  then keep `specializeLinkageDecoration` unchanged.
  Rationale: specialized linkage already delegates canonical argument spelling to this shared
  helper. Encoding pack structure locally in the clone pass would create a second type spelling and
  leave every other caller unable to distinguish canonical packs.
  Date/author: 2026-08-31, Codex.
- Decision: bound Slice 152 to function identity even if repaired workloads expose later ABI,
  aggregate, arithmetic, or operation blockers.
  Rationale: later diagnostics represent independent canonical invariants and must be selected by
  the cross-corpus Pareto rather than folded into one fixture-driven slice.
  Date/author: 2026-08-31, Codex.

## Outcomes and Retrospective

Frozen corpus v1 remains exactly 452 workloads and 427 healthy MVP references. O0/O3/both
correctness reaches 377/381/377, up five in each numerator with zero old-correct regression. The
newly correct rows are `size-of-tuple`, `buffer-struct-with-interface-field`,
`static-method-dispatch`, `this-return-chained`, and `if-let-1`. The sixth frozen identity row,
`generic-interface-nested`, advances to the independent provider aggregate-field-pointer failure.

Discovery remains exactly 82 workloads and 72 healthy native references. O0/O3/both correctness
reaches 54/54/54, up two with zero old-correct regression. The newly correct rows are
`array-of-interfaces-interproc` and `variadic-pack-query-pack-conformance`. Across all discovery
rows, each direct mode has 54 correct, 19 preflight, one provider, seven infrastructure, and one
runtime mismatch. Function identity is eliminated from both Pareto reports.

The selected NVVM unit prefix passes 427/427. All fourteen promoted direct lanes pass; complete
file runs for two shaders also observe their pre-existing unrelated synthetic WebGPU bind-group
failure. All twelve measurement gates assemble through CUDA 12.9 at direct O3 for SM70, SM80, and
SM90. Provider ABI revision 30 remains unchanged.

The result demonstrates why root-cause selection should remain producer-oriented: one measured
failure cluster contained two distinct specialization producers, but both violated the same
function-identity invariant. Repairing both sources unlocked seven complete workloads without any
new NVVM operation. The next Pareto no longer contains function identity; aggregate pointer/layout
transport and helper ABI are again the leading shared families.

## Context and Current Pipeline

Consider `static-method-dispatch.slang`. Its entry point first passes an interface value to
`processStatic(ICompute)` and later passes a statically known `MulOp` to the same source helper.
Typeflow analysis represents the first path with a tagged union and the second with the concrete
type. `lowerSpecializeExistentialsInFunc` clones the helper for the concrete call context and
transfers propagation and call-site information to that clone. `specializeFunc` then changes the
clone's signature to `Func(Float, MulOp)` while the base remains `Func(Float, Tuple)`.

The typeflow clone is currently made by `cloneInst`, which recursively copies the base function's
decorations and body. The copied `IRExportDecoration` continues to name the clone as the original
source declaration. Linking preserves both reachable definitions. `_collectNVVMFunctions`
correctly discovers both distinct callees, and `_collectNVVMFunctionNames` rejects their identical
non-empty physical symbols before provider mutation.

The variadic examples reach a separate generic specialization producer. `specializeLinkageDecoration`
starts from the generic's linkage and appends a SHA-1 digest of each specialization argument's
`getTypeNameHint` spelling. `IRTypePack` is a canonical hoistable type whose operands are the exact
pack elements, but `getTypeNameHint` previously fell through to a name decoration and emitted
nothing. Thus `tupleSize2<>` and `tupleSize2<int, int>` both received the same empty-input digest
despite having different signatures.

The canonical construction boundaries are therefore the typeflow clone and shared canonical type
spelling, not NVVM name collection. Once a typeflow clone loses linkage,
`_getNVVMFunctionName` returns an empty name and the existing deterministic internal-name path
assigns a collision-free LLVM symbol. Once `IRTypePack` spells its complete operands, retained
generic specializations receive different source-derived linkage digests. Name hints remain useful
for diagnostics and readable output but are not external linkage.

## Scope and Non-Goals

In scope are the exact `IRSpecializeExistentialsInFunc` clone boundary, canonical `IRTypePack`
spelling in `getTypeNameHint`, preservation of name hints, an exact duplicate-symbol diagnostic,
focused producer/emitter regression coverage, O0/O3
differential runs for all eight motivating workloads, stable promotion, complete separate-corpus
measurement, and durable design/ledger updates.

Out of scope are downstream collision suffixes, arbitrary name uniquing, changing source mangling,
removing linkage from unrelated clones without producer evidence, new provider callbacks or ABI
revision, later operation/type support, fixture-name checks, syntax reconstruction, compatibility
fallbacks, frozen-corpus identity changes, and corpus v2.

## Architecture and Invariants

- A typeflow clone whose signature is specialized for one existential call context is internally
  linked and cannot claim the base source function's import/export symbol.
- The source function retains its original linkage and remains the only definition of that symbol.
- Canonical `IRTypePack` spelling includes its arity, order, and recursive element spellings, so
  different packs contribute different specialized-linkage digest input.
- Name hints remain non-semantic diagnostic metadata and may be copied to internal clones.
- Every emitted direct-NVVM function has one distinct physical symbol before provider discovery.
- Duplicate non-empty linkage names remain a deterministic preflight error; the emitter never
  repairs malformed linkage by renaming externally visible definitions.
- Frozen corpus v1 and discovery retain separate fixed denominators and reports.

## Interfaces and Dependencies

The producer fixes are limited to `source/slang/slang-ir-typeflow-specialize.cpp`, which uses the
existing `removeLinkageDecorations` utility, and `source/slang/slang-ir-util.cpp`, which owns the
existing canonical type-name helper. Direct-NVVM diagnostic precision may touch
`source/slang/slang-emit-nvvm.cpp`, but provider ABI revision 30 and every builder interface remain
unchanged. Stable repository tests may gain direct O0/O3 directives only after real-provider
differential correctness is proven.

## Milestones

1. Strip linkage from the clone immediately after `lowerSpecializeExistentialsInFunc` creates it,
   with a producer comment explaining why the base and specialization can coexist.
2. Spell canonical `IRTypePack` values recursively in `getTypeNameHint`, so distinct generic-pack
   specializations receive distinct existing linkage digests without a second mangling path.
3. Add focused regression evidence that the same source helper can remain reachable in generic and
   concrete forms and that empty/non-empty pack specializations retain distinct symbols. Preserve
   exact preflight diagnostics for malformed duplicates where practical.
4. Build and run the focused test, then run all eight corpus rows at direct O0 and O3. Promote only
   stable correct workloads; classify every cascade at its exact first canonical shape and producer.
5. Run the selected direct-NVVM prefix and both complete corpora, regenerating separate TSV, JSON,
   Pareto, and representative measurement artifacts.
6. Update this plan, the Slice 152 report, design document, and capability ledger; complete the
   input-shape/unprincipled-change audit and commit the slice.

## Validation and Acceptance

All CMake builds and tests run outside the sandbox with Windows-native tools as required by
`AGENTS.md`. Acceptance requires:

- a post-typeflow probe shows the concrete clone has no `IRLinkageDecoration` while the base keeps
  the original export;
- focused direct-NVVM coverage reaches provider emission without a duplicate function symbol;
- each newly promoted workload is differentially correct through the real provider at O0 and O3,
  and each advanced-but-not-correct workload has its exact new shape, producer, and diagnostic;
- the selected direct-NVVM regression prefix passes with zero old-correct regression;
- frozen v1 remains exactly 452 workloads/427 healthy references and discovery remains exactly 82
  workloads/72 healthy references, each with separate O0, O3, both, classification, Pareto, and
  regression evidence;
- representative direct O3 PTX assembles for SM70, SM80, and SM90 where the established harness
  permits;
- provider ABI revision remains 30; and
- `git diff --check`, artifact integrity, and the input-shape/unprincipled-change audit pass, with
  no staged content from `external/slang-binaries/`.

## Failure and Recovery

If removing linkage changes a non-NVVM target's symbol behavior, inspect whether that target relied
on the malformed duplicate export or whether a different producer class is involved; do not put the
decoration back only for NVVM. If an affected workload reaches a new first blocker, record the
cascade and leave it for Pareto selection. Generated outputs under `build/` are reproducible.

## Artifacts and Hand-Off

Commit this completed plan because the user explicitly requires plans and implementation together
for this experiment. Also commit the producer fix, focused and promoted tests, Slice 152 frozen-v1
and discovery snapshots/reports, and durable design/ledger changes. The report must name the exact
clone producer, show why the cloned shape is valid but copied linkage is not, and distinguish newly
correct workloads from rows that only advance to later blockers.
