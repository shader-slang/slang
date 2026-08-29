# Complete aggregate value arrays and helper transport

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires the completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, direct NVVM compiles and executes the two remaining honest compute-corpus
candidates:

- `tests/compute/dynamic-dispatch-7.slang`, whose specialized implementation values cross ordinary
  helper boundaries; and
- `tests/compute/struct-default-init.slang`, whose default-initialized structs are constructed as a
  fixed array and selected dynamically.

The implementation must consume their canonical post-specialization aggregate values through one
generic value/array contract. It must not recover source interfaces or default-initializer syntax,
specialize by fixture name, or add aggregate-shape-specific callbacks to the LLVM provider.

## Progress

- [x] (2026-08-30) Completed Slice 129 as `f6b49b662`; Release provider/host builds, all six
  promoted resource-storage lanes, CUDA PTX assembly, and the complete NVVM prefix passed 398/398.
- [x] (2026-08-30) Re-probed both fixtures through the compute harness and captured their exact
  canonical final-IR producers, types, operations, and first diagnostics.
- [x] (2026-08-30) Reused the resource-capable struct and copyable-array contracts, consolidated
  the duplicate local-struct pointer classifiers, and repaired only the measured helper,
  construction, and extraction role gaps with focused positive and adjacent-negative coverage.
- [x] (2026-08-30) Promoted direct runtime and PTX/FileCheck lanes for both fixtures, preserved their
  unchanged results, inspected both PTX modules, and assembled both with CUDA 12.9.86.
- [x] (2026-08-30) Ran pinned formatting, Release provider/host builds, focused tests, the complete
  399/399 NVVM prefix, and `git diff --check`; updated durable status and completed the self-review.

## Surprises and Discoveries

- Slice 128's harness probe measured `dynamic-dispatch-7.slang` at `helper function parameter` and
  `struct-default-init.slang` at `makeArray`. These diagnostics must be revalidated after Slices 129
  and 130's fresh optimization pipeline; they are starting evidence, not an implementation recipe.
- The fresh final `dynamic-dispatch-7.slang` program has no interface or witness operation. Its only
  unsupported parameter is `BorrowInOutParam<Impl>`, where `Impl` recursively contains
  `Impl.TAssoc { Int base; }`. The local `Ptr<Impl>` already used the resource-capable classifier,
  but the borrow path still called the older direct-scalar-field classifier.
- `struct-default-init.slang` reaches direct emission as four ordinary `Test` helper results,
  `makeArray : Array<Test, 4>`, a dynamic `getElement`, and four keyed field extracts. Type lowering
  already accepts this copyable array; only construction and sequential extraction still requested
  the narrower numeric-array classifier.
- The real provider's sequential operation was already generic over LLVM vectors and arrays. For a
  dynamic array index it emits bounded constant `extractvalue` operations plus typed selects. The
  fake modeled this for scalar arrays but did not retain a selected struct's type through its
  legacy `VectorElement` value kind; one test-only inference extension made the generic topology
  observable without a provider ABI change.

## Decision Log

- Decision: group the two remaining aggregate-value fixtures into one larger slice, beginning with
  a shared input-shape audit and retaining only generic contracts that both final programs justify.
  Rationale: earlier slices already established recursively copyable/resource structs, generic
  aggregate construction and extraction, fixed local arrays, and by-value helpers. The measured
  boundaries are likely stale role restrictions around those interfaces, and addressing the
  family together is more economical than one callback-sized slice per operation.
  Date/author: 2026-08-30, Codex.
- Decision: keep `bound-check-zero-index.slang` excluded from promotion.
  Rationale: direct compilation already succeeds, but Slice 113 reproduced the fixture's documented
  CUDA runtime-result mismatch. Aggregate work cannot turn a known semantic mismatch into useful
  backend coverage.
  Date/author: 2026-08-30, Codex.
- Decision: replace the scalar-struct local-pointer classifier with the existing resource-capable
  classifier across local `Ptr` and `BorrowInOutParam` roles, while retaining the scalar-only
  condition for the established explicit thread-local-context spelling.
  Rationale: caller and callee point at the same canonical aggregate representation. Keeping two
  classifiers made the legal local `Ptr<Impl>` become illegal solely when its helper signature used
  `BorrowInOutParam<Impl>`. The thread-local global-context producer is a distinct measured scalar
  contract and is not widened without evidence.
  Date/author: 2026-08-30, Codex.
- Decision: route `makeArray` and integer-indexed `getElement` through
  `asNVVMSupportedCopyableArrayType` rather than introduce struct-array operations.
  Rationale: type lowering and local storage already use that exact type contract, and both the
  compiler and provider operations are aggregate-generic. The numeric-only checks were stale
  consumer restrictions over an established representation.
  Date/author: 2026-08-30, Codex.

## Outcomes and Retrospective

The final program shapes remain ordinary first-class LLVM aggregates. The specialized
dynamic-dispatch fixture uses nested `Impl`/`Impl.TAssoc` values plus a mutable helper pointer, and
the default-initializer fixture uses `Array<Test, 4>` construction plus a bounded dynamic selection.
Both runtime lanes pass unchanged, their PTX/FileCheck lanes pass, and their 645- and 1,222-byte PTX
modules assemble to 2,792- and 3,048-byte cubins. Release provider and host builds pass, as do the
focused aggregate unit, all six `dynamic-dispatch-7` lanes, both new `struct-default-init` lanes,
and the complete 399/399 NVVM regression prefix. Builder ABI remains revision 24.

The self-review found four retained changes. First, the consolidated resource-capable local-pointer
classifier owns the canonical `Ptr<Impl>` and `BorrowInOutParam<Impl>` produced after specialization;
reverting borrow admission restores `dynamic-dispatch-7`'s helper-parameter failure. Second and
third, copyable-array construction and sequential extraction own the canonical `Array<Test, 4>`
made by `makeArray` and indexed by `getElement`; reverting either widening restores the corresponding
`struct-default-init` preflight failure. Fourth, the fake provider's selected-aggregate type
inference is test-only and is required to observe the same generic extraction topology already
implemented by the real provider. Existing device-struct-pointer and array-pointer negatives remain,
explicit thread-local aggregate pointers remain scalar-only, and the slice adds no custom
equivalence, syntax reconstruction, compatibility fallback, fixture-name branch, or provider API.

## Context and Current Pipeline

The CUDA varying legalizer, specialization, any-value lowering, and optimization run before direct
NVVM validation. Prior traces show that `dynamic-dispatch-7.slang` has no indirect call at the
backend boundary: generic `Impl` construction and associated-type calls become concrete structs and
ordinary helpers. `struct-default-init.slang` similarly turns source default initializers into
explicit `Test` values and a canonical fixed array before selecting one element by thread index.

The builder already exposes generic LLVM struct/array types, aggregate construction/extraction,
local storage, element pointers, typed loads/stores, calls, and returns. Existing compiler-owned
classifiers distinguish numeric vectors, recursively numeric copyable structs, resource-bearing
structs, numeric arrays, and local arrays of copyable structs. The first task is to determine
whether the two failures are legitimate missing representations or inconsistent role gates over
operations already supported elsewhere.

## Scope and Non-Goals

In scope are the exact specialized aggregate helper parameters/calls/results, `makeStruct` and
`makeArray` values, fixed local/value arrays, dynamic element selection, keyed field extraction,
and numeric arithmetic exercised by the two fixtures; centralized classification; exact type and
layout checks; focused fake/real-provider coverage when a provider contract is newly exercised;
direct runtime/PTX lanes; PTX assembly; durable documentation; and this plan.

Out of scope are unspecialized interface calls, indirect/external calls, runtime-sized arrays,
arbitrary padding or explicit stride, recursive aggregate graphs, entry-point resource aggregates,
resource-bearing helper results unless produced by the selected traces, source default-initializer
reconstruction, new fixture-specific builder operations, compatibility aliases, the known bound-
check runtime mismatch, and unrelated shader families.

## Architecture and Invariants

- Classification begins with the canonical final IR type. It never consults source declarations,
  function names, interface witnesses, test directives, or diagnostics as semantic input.
- One source of truth owns each accepted aggregate role. A widened helper or array consumer must
  reuse that role or replace overlapping classifiers; it cannot reproduce a field-kind list.
- Struct fields are selected by canonical key and exact type. Array elements use exact element
  type and count; natural LLVM representation is admitted only where CUDA layout agrees.
- Aggregate values remain first class through generic builder operations. The compiler does not
  flatten them into callback arguments or reconstruct them from source syntax.
- Function signature, call, and return types must agree exactly. Unsupported pointer escape,
  storage role, layout, or recursion stops before provider module creation.
- Existing resource, numeric-vector, compact Float3 storage, and helper-reference contracts remain
  distinct unless the final traces demonstrate a principled common representation.

## Interfaces and Dependencies

Expected committed areas are direct NVVM type classification/lowering and validation/emission,
focused unit-test support, the two existing compute fixtures, `docs/design/nvvm-backend.md`, the
capability ledger, and this plan. The existing generic builder ABI should remain revision 24 unless
the fresh canonical IR proves a genuinely absent operation rather than a narrow compiler gate.

CUDA 12.9 libNVVM provides optimized PTX, CUDA 12.9.86 `ptxas -arch=sm_70` provides assembly
validation, and the compute harness remains authoritative for specialization and bound inputs.

## Milestones

1. Add temporary direct runtime probes to both fixtures, run their exact lanes, retain optimized IR
   and diagnostic evidence under ignored `build/slice130-*`, and trace every rejected value to its
   producer and existing classifier.
2. Consolidate the smallest generic aggregate value/array contract that owns the measured shapes;
   add focused positive topology plus adjacent rejection coverage, and avoid provider ABI changes
   unless an operation is truly missing.
3. Iterate both harness fixtures through all newly exposed operations. Promote passing direct
   runtime and static PTX/FileCheck lanes without changing source behavior or expected output.
4. Inspect and assemble every PTX artifact, run pinned formatting, Release provider/compiler/test
   builds, focused regressions, the complete NVVM prefix, and `git diff --check`; update durable
   status and this plan, perform the input-shape/revert audit, and commit.

## Validation and Acceptance

Acceptance requires focused evidence for the exact aggregate helper and array topology plus
adjacent invalid shapes; unchanged existing and new lanes for both selected fixtures (or an
explicitly narrowed coherent subset if fresh IR proves independent ownership); inspectable direct
PTX with expected entry points, aggregate-derived arithmetic, and output stores; CUDA 12.9
`ptxas -arch=sm_70`; Release provider and host builds; the complete
`slang-unit-test-tool/nvvm` prefix; pinned clang-format 17; and `git diff --check`.

The self-review inventories every new helper, fallback, classifier widening, recursion guard,
layout check, consumer branch, and special case. For each retained item, record the exact producer,
canonical shape, failing fixture/test without it, semantic source of truth, and why this layer owns
the operation. Remove any custom structural equivalence, syntax reconstruction, arbitrary graph
walk, source-name match, positional field assumption, duplicated type list, compatibility shim, or
change whose revert does not fail selected coverage.

## Failure and Recovery

If the two fixtures do not share a principled aggregate contract, preserve their final IR and
diagnostics under ignored `build/slice130-*`, narrow the slice to the largest complete coherent
subset, and record the other boundary for Slice 131. Do not weaken fixtures, modify expected
results, force inlining, bypass layout/type checks, silently use NVRTC, reset unrelated work, or
stage `external/slang-binaries/`.

## Artifacts and Hand-Off

Keep generated IR, LLVM, PTX, cubin, and logs under ignored `build/slice130-*`. Distill the final
aggregate contract, exact fixture/PTX evidence, exclusions, and next measured boundary into
`docs/design/nvvm-backend.md` and the capability ledger, then commit this plan with the
implementation as explicitly requested.
