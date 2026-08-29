# Carry nested copyable aggregates through helpers

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires the completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, direct NVVM supports layout-compatible nested copyable structs whose leaves are
selected numeric values across helper results, by-value parameters, mutable locals, keyed field
chains, aggregate loads/stores, and direct calls. The existing
`tests/compute/assoctype-nested-lookup.slang` fixture should pass direct runtime and PTX lanes with
its unchanged result `3`; source associated types themselves must remain absent from final NVVM IR.

## Progress

- [x] (2026-08-29) Completed Slice 117 as `cb84fddff` with mutable copyable structured-buffer field
  addressing and a 385/385 NVVM unit prefix.
- [x] (2026-08-29) Dumped the optimized associated-type fixture after `checkUnsupportedInst` and
  audited three canonical helpers: `ConcreteFoo` construction, `FooPair<ConcreteFoo>` construction,
  and a void consumer taking the nested pair by value.
- [x] (2026-08-29) Generalized the copyable-struct family, layout proof, retained-type closure,
  local/field addressing, type lowering, signatures, calls, and aggregate values for nested numeric
  leaves.
- [x] (2026-08-29) Promoted and validated the real fixture, preserved focused adjacent negative and
  flat-aggregate coverage, assembled PTX, ran the full NVVM prefix, documented, formatted, and
  self-reviewed the slice.

## Surprises and Discoveries

- Specialization has completely removed interfaces, associated types, and generic witnesses from
  the executable body. The retained `FooPair` has two direct `ConcreteFoo` fields, and each
  `ConcreteFoo` has one Float field. Treating this as dynamic-dispatch support would target the
  wrong representation layer.
- Void helper results are already legal. The first diagnostic names the helper result because the
  earlier first-level copyable classifier rejects the nested `FooPair` returned by its synthesized
  default constructor.
- Nested field addresses acquire the explicit scalar-layout pointer spelling after selecting an
  inner struct. They must be admitted from the already validated mutable parent field producer,
  not by widening the compact local-pointer classifier to arbitrary explicit pointers.
- Selecting only the outer struct declaration is insufficient in the general case. Its nested
  field type is a separate retained global declaration and must be included by a bounded copyable
  type-dependency closure rather than by relying on another helper to mention it independently.
- The fake provider intentionally assigns one handle and one field schema to every copyable struct.
  Accepting the old nested-helper source there would make an inner and outer struct falsely
  identical and weaken its exact-type assertions. The real LLVM provider and promoted fixture are
  the faithful nested-type regression; the focused fake tests remain responsible for flat
  aggregate contracts and adjacent rejection boundaries.

## Decision Log

- Decision: make the existing copyable-struct classifier recursive only through nonempty nested
  copyable structs, with selected numeric scalar/vector leaves.
  Rationale: this is the canonical final type tree and reuses the established value family. Arrays,
  matrices, Boolean storage, resources, pointers, opaque fields, and empty structs remain separate.
  Date/author: 2026-08-29, Codex.
- Decision: recursively verify CUDA/LLVM layout for every nested copyable struct and retain every
  declaration in that same type tree.
  Rationale: matching only the outer size and offsets could conceal an incompatible inner field
  layout, while retaining only the outer declaration makes type lowering depend on incidental
  references elsewhere in the module.
  Date/author: 2026-08-29, Codex.
- Decision: reuse existing struct, local-storage, field-pointer, aggregate load/store, function,
  call, and return operations without a builder ABI revision.
  Rationale: all provider callbacks already carry exact type handles and are recursively composable;
  the missing boundary is role-aware compiler classification.
  Date/author: 2026-08-29, Codex.

## Outcomes and Retrospective

Slice 118 carries recursively nested, layout-compatible numeric-leaf structs through helper
results, by-value parameters, locals, keyed field chains, aggregate loads/stores, direct calls, and
returns without changing the builder ABI. `assoctype-nested-lookup.slang` returns its unchanged
Float result `3` through the direct runtime lane, passes direct PTX FileCheck, and produces 486
bytes of PTX that CUDA 12.9.86 `ptxas -arch=sm_70` assembles into a 2,664-byte cubin. Its complete
fixture prefix passes 6/7 plus one ignored; the only failure is the pre-existing synthesized WebGPU
Dawn bind-group validation problem, while both CUDA lanes pass. The full NVVM unit prefix remains
385/385.

The implementation stayed at canonical construction boundaries: recursive classification follows
only direct semantic struct fields; layout recursion follows the same type tree; retained-type
closure starts only from already accepted signatures, locals, or resources; and nested field
addressing requires an exact resolved mutable parent producer. No source associated-type lookup,
generic witness handling, custom type equivalence, arbitrary operand traversal, LLVM text rewrite,
or new provider callback was introduced.

## Context and Current Pipeline

The optimized fixture contains these physical types and calls:

    struct ConcreteFoo { float x; };
    struct FooPair { ConcreteFoo a; ConcreteFoo b; };

    ConcreteFoo ConcreteFoo_init();
    FooPair FooPair_init();
    void test(FooPair pair);

`FooPair_init` owns a local `FooPair`, calls `ConcreteFoo_init`, stores the returned leaf aggregate
into both keyed fields, loads the whole pair, and returns it. `test` stores its by-value pair into a
local, selects `a.x` and `b.x` through two keyed field levels, mutates them, and writes their sum to
the established Float structured buffer. `computeMain` calls the pair constructor and then the void
consumer. This is ordinary nested aggregate and direct-call IR.

## Scope and Non-Goals

In scope are nonempty recursively copyable structs with selected numeric scalar/vector leaves;
recursive layout compatibility; retained nested type declarations; compact local pointers and
mutable nested field-pointer producers; whole nested aggregate values; direct helper results and
parameters; and runtime/PTX promotion of the existing fixture.

Out of scope are recursive-by-pointer source types, arrays or matrices in nested structs, Boolean
storage, padding synthesis, incompatible CUDA/LLVM layouts, resources or pointers as fields,
aggregate phis/selects, indirect calls, source-level dynamic dispatch, new ABI callbacks, and
unrelated conventional parameter-block widening.

## Architecture and Invariants

- `asNVVMSupportedCopyableStructType` remains the single structural classifier. Recursion follows
  only direct struct fields and must terminate at established numeric values.
- `_hasNVVMCompatibleStructLayout` verifies size and semantic field offsets at every nested struct,
  using canonical CUDA and LLVM layout rules without synthesizing padding.
- Retained-type selection computes only the nested copyable field dependency closure rooted at an
  already accepted signature, local, or resource type. It does not walk arbitrary operand graphs.
- `_getNVVMStructFieldAddress` admits an inner explicit-layout struct pointer only when its producer
  is an exact resolved mutable parent field and its pointee is the same nested copyable type.
- Type lowering recursively requests existing provider struct handles. Function preflight,
  declaration, calls, and returns use those same canonical handles.

## Interfaces and Dependencies

No builder ABI or LLVM provider change was required. The type-lowering classifier, direct emitter,
obsolete fake-provider negative, existing compute fixture, durable design status, and this plan are
the committed areas. CUDA 12.9 runtime and `ptxas` provide semantic and assembly evidence.

## Milestones

1. Generalize nested copyable classification, layout verification, dependency retention, and
   recursive provider type lowering.
2. Admit mutable nested field chains plus aggregate helper results, parameters, calls, and returns.
3. Preserve focused flat-aggregate traces and rejection of nested arrays, incompatible layouts,
   and unsupported pointer/resource field families; use the exact real provider for nested type
   identity.
4. Promote the existing associated-type fixture, validate runtime/PTX and `ptxas`, run the complete
   NVVM prefix, update durable status and this plan, self-review, format, and commit.

## Validation and Acceptance

Acceptance requires Release host builds; real-provider evidence for nested provider struct types,
helper result/parameter contracts, local storage, nested keyed field pointers, aggregate
loads/stores/calls/returns, and zero new provider features; focused flat-aggregate fake coverage and
adjacent negative coverage before provider mutation; direct runtime/PTX lanes for the existing
fixture; CUDA 12.9
`ptxas -arch=sm_70`; the complete `slang-unit-test-tool/nvvm` prefix; pinned formatting; and
`git diff --check`.

The self-review inventories recursive classification, layout recursion, dependency closure,
field-producer recursion, and all test adjustments. Confirm each traversal follows a canonical
type declaration rather than rediscovering context from arbitrary operands, and remove any silent
cycle fallback, custom type equivalence, source associated-type recognition, or generic explicit
pointer admission.

## Failure and Recovery

If a provider operation proves non-composable or runtime differs, preserve the exact final IR and
PTX under ignored `build/slice118-*`, narrow the slice to independently demonstrated nested type
transport, and record the next canonical stop. Do not flatten aggregates in the emitter, rebuild
source syntax, patch LLVM text, reset unrelated work, or stage `external/slang-binaries/`.

## Artifacts and Hand-Off

Keep generated IR, PTX, cubin, and logs under ignored `build/slice118-*`. Distill the final nested
aggregate contract, validation evidence, and next measured corpus boundary into
`docs/design/nvvm-backend.md`, then commit this plan with the implementation as explicitly
requested.
