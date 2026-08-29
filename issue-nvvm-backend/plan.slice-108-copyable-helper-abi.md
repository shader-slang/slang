# Carry copyable values across helper boundaries

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, direct NVVM helpers can accept and return the same flat copyable numeric structs
that the backend already loads, stores, and transports through structured buffers. Helper bodies
can extract a field from a first-class struct value, and mutable numeric `inout` parameters use the
same generic-pointer ABI already established for mutable struct borrows and numeric `out`
parameters. Existing `structured-buffer-of-struct.slang`, `typedef-member.slang`, and
`mutating-and-inout.slang` fixtures run through direct NVVM without changing their shader bodies or
expected outputs.

## Progress

- [x] (2026-08-29) Measured the aggregate-helper failures in existing compute fixtures and grouped
  flat copyable values and mutable local borrows separately from arrays, matrices, empty receiver
  structs, and resource-bearing structs.
- [x] (2026-08-29) Widened the canonical helper ABI and reachable-type collection to flat copyable structs and
  mutable numeric borrows.
- [x] (2026-08-29) Made field extraction select the representation established by the producer: first-class
  aggregate values for helpers and pointer-backed `byval` storage for CUDA entry parameters.
- [x] (2026-08-29) Extended fake-provider coverage for first-class aggregate parameters and numeric mutable
  borrows.
- [x] (2026-08-29) Promoted the three selected existing fixtures to direct runtime and direct PTX coverage.
- [x] (2026-08-29) Built, ran focused and complete NVVM tests, assembled generated PTX,
  self-reviewed, updated durable status and this plan, and prepared the complete slice commit.

## Surprises and Discoveries

- Slice 97 already established `asNVVMSupportedCopyableStructType`, aggregate construction,
  aggregate load/store, local layout checks, and provider `extractvalue`. The unsupported helper
  boundary is an artificial admission gap, not a missing LLVM operation or a second representation.
- CUDA entry-point aggregate parameters intentionally lower to generic pointers carrying LLVM
  `byval`, whereas ordinary helper parameters lower to first-class aggregates. Both are valid
  producer shapes. Field extraction must preserve that explicit ABI distinction rather than assume
  every semantic struct value is pointer-backed.
- `struct-in-generic.slang` retains an empty receiver value and
  `LoadFromUninitializedMemory` in addition to its copyable argument. Empty/undefined aggregate
  values are a separate construction boundary and are not needed to prove this slice.
- `array-param.slang`, `column-major.slang`, and `func-param-legalize.slang` require arrays,
  matrices, or resource-bearing aggregates. Admitting their helper signatures without matching
  value and operation support would weaken preflight.

## Decision Log

- Decision: reuse the existing flat copyable-struct classifier for helper parameters and results.
  Rationale: this is the backend's existing source of truth for aggregate values whose fields can
  be represented directly by LLVM. Creating a helper-specific struct family would duplicate the
  type contract and make future vector-field support diverge.
  Date/author: 2026-08-29, Codex.
- Decision: accept `BorrowInOutParam` in the existing local numeric pointer classifier and extend
  exact call compatibility from local `Ptr` to both numeric `out` and `inout` parameters.
  Rationale: the canonical IR types differ by source-level ownership, but the selected mutable
  local argument and helper parameter deliberately lower to the same typed generic pointer.
  Date/author: 2026-08-29, Codex.
- Decision: keep kernel `byval` field access pointer-backed and use aggregate extraction for helper
  values.
  Rationale: the distinct physical shapes are intentional ABIs established when function
  parameters are declared. The emitter should dispatch on that producer fact, not reconstruct or
  spill an aggregate merely to force one physical form.
  Date/author: 2026-08-29, Codex.

## Context and Current Pipeline

The direct NVVM path discovers a closed direct-call graph, preflights every signature and body,
lowers canonical IR types to provider handles, declares all functions, and then emits their bodies.
Copyable flat structs already lower to LLVM struct values and can live in checked local or
structured-buffer storage. Scalar-only structs can cross helper boundaries today because that was
the initial stateful-helper proof, but the signature checks use the narrower classifier and field
extraction only accepts a CUDA entry parameter.

The three selected fixtures exercise complementary paths:

- `structured-buffer-of-struct.slang` loads a two-integer struct, passes it by value, mutates a local
  copy, returns the struct, and stores it back;
- `typedef-member.slang` constructs a one-float struct, passes it by value, and extracts its field in
  the helper;
- `mutating-and-inout.slang` composes a struct-returning constructor, a mutable struct receiver, and
  a mutable integer `inout` parameter.

## Scope and Non-Goals

In scope:

- flat, non-empty structs whose fields satisfy `isNVVMSupportedNumericValueType` as first-class
  helper parameters and results;
- first-class helper struct field extraction via the existing aggregate operation;
- pointer-backed field extraction for the existing CUDA kernel `byval` representation;
- generic-address-space mutable numeric `BorrowInOutParam` and matching local `Ptr` arguments;
- direct runtime, direct PTX, fake-provider, real-provider, and negative-boundary coverage.

Out of scope:

- nested or empty structs, arrays, matrices, tuples, and resource-bearing structs;
- undefined aggregate construction and generic receiver erasure;
- external/exported C ABI guarantees for aggregate helpers;
- changing the builder ABI or adding a representation-specific builder operation;
- dynamic dispatch, recursion, or indirect calls.

## Architecture and Invariants

- `asNVVMSupportedCopyableStructType` remains the single definition of a first-class aggregate
  value accepted by this slice.
- A helper call argument must exactly equal its parameter type except for the explicit canonical
  local-`Ptr` to `OutParam`/`BorrowInOutParam` mutable-pointer equivalence.
- Every aggregate type stored in local or structured-buffer memory retains the existing CUDA/LLVM
  field-offset and size check.
- First-class aggregate field extraction receives an aggregate provider value; CUDA entry `byval`
  extraction receives the declared pointer and performs a typed invariant load.
- Unsupported aggregate families fail preflight before the provider sees a partial module.

## Interfaces and Dependencies

No production builder ABI revision is planned. The emitter and type-lowering admission rules reuse
`getStructType`, `emitAggregateElementExtract`, and the existing pointer type. The fake provider
must model a first-class struct function parameter so unit tests exercise the same generic API
contract as LLVM 14. Update the selected test directives, durable design status, and this plan.

## Validation and Acceptance

Acceptance requires focused fake-provider tests for by-value copyable helpers and numeric `inout`,
the three exact direct runtime fixtures with their existing expected data, direct PTX for all three,
CUDA 12.9 `ptxas -arch=sm_70` acceptance, retained negative diagnostics for nested/resource/array
aggregate boundaries, the complete `slang-unit-test-tool/nvvm` prefix, pinned clang-format, and
`git diff --check`.

Record exact counts, output values, PTX/cubin sizes, discovered next boundary, and the self-review
inventory.

## Self-Review and Input-Shape Audit

Inventory every widened classifier, compatibility branch, and representation dispatch. For each,
identify the exact canonical IR producer and remove it temporarily when practical to confirm which
fixture fails. Verify that helper aggregate parameters are first-class values created by function
declaration, CUDA entry aggregate parameters are intentionally pointer-backed by the `byval` ABI,
and numeric `BorrowInOutParam` comes only from a mutable local pointer at admitted calls. Do not
retain a fallback that guesses from a provider handle or silently accepts an unknown aggregate.

## Failure and Recovery

If LLVM verification, libNVVM, runtime execution, or `ptxas` rejects the widened contract, preserve
the emitted IR/PTX under ignored `build/` and stop at the actual ABI boundary. Keep unsupported
families rejected, do not special-case a fixture, do not reset unrelated work, and never stage
`external/slang-binaries/`.

## Outcomes and Retrospective

The helper signature and type-lowering gates now use the established flat copyable-struct
classifier for both parameters and results. `IRFieldExtract` accepts those same values: helper
fields use the existing first-class aggregate extraction callback, while the selected entry-point
struct path retains its intentional `byval` pointer and invariant load. Numeric
`BorrowInOutParam` is admitted alongside numeric `OutParam`, with one exact local-`Ptr` argument
compatibility rule covering both.

The focused source composes a struct containing Int32 and Float32x4 fields, a struct-returning
helper, a mutable numeric borrow, first-class scalar/vector field extraction, and a second by-value
helper call. It exposed three stale fake-provider assumptions rather than production gaps: generic
call pointer checking was struct-specific, local storage was struct-only, and extracted scalar
aggregate fields were not recognized as scalar values. The fake now checks pointer pointee kinds,
accepts the same copyable local value families, and uses one recorded aggregate-element type check
for scalar and vector values. The real LLVM/libNVVM provider accepted both optimized and
unoptimized versions throughout.

All twelve exact affected lanes pass 12/12, including the six new direct CUDA runtime/PTX lanes.
`structured-buffer-of-struct.slang` preserves pairwise-swapped output
`2, 1, 4, 3, 6, 5, 8, 7`; `typedef-member.slang` preserves the four Float32 thread-index values;
and `mutating-and-inout.slang` preserves its existing reference result. Their optimized PTX sizes
are 720, 674, and 685 bytes. CUDA 12.9.86 `ptxas -arch=sm_70` accepts all three and emits a
2,792-byte cubin for each. Focused positive/regression/negative units pass 3/3, and the complete
Release NVVM unit prefix passes 377/377.

The nested-helper negative was added to the existing pre-emission table. Direct probes of
`array-param.slang` and resource-bearing `func-param-legalize.slang` still stop with E52017
`helper function parameter`, before provider mutation. `struct-in-generic.slang` additionally
retains an empty receiver and undefined aggregate construction, so it remains a separate future
boundary rather than being admitted accidentally.

Self-review inventoried five changes. The copyable signature widening is required by the structured
buffer and typedef fixtures; the numeric-borrow classifier and exact compatibility branch are
required by the mutating fixture; the field-extraction dispatch is required because kernel `byval`
and helper aggregate parameters are both canonical but physically distinct; and the fake pointer,
local-storage, and aggregate-element generalizations model existing generic builder contracts.
No producer-side malformed IR, syntax reconstruction, opaque-handle guessing, new builder
operation, fallback, or fixture-specific production special case remains. Removing any production
branch restores the measured focused failure corresponding to that boundary.
