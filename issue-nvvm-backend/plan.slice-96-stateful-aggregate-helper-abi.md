# Add stateful aggregate helper ABI

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, direct NVVM can represent a canonical copyable scalar-struct local whose address
intentionally crosses a helper-call boundary. The builder exposes exact generic local storage,
generic calls accept typed generic pointers, and direct lowering preserves
`BorrowInOutParam<Values>` as an LLVM pointer rather than inventing value-threading. The existing
`tests/compute/half-vector-compare.slang` fixture should pass direct CUDA runtime and PTX lanes.

## Progress

- [x] (2026-08-29) Reproduced the post-Slice-95 boundary: the first rejected construct is the
  scalar-struct result of `Values.$init`.
- [x] (2026-08-29) Traced the complete final-IR shape. `Values.next` takes
  `BorrowInOutParam<Values>`, mutates `m_index`, and is called repeatedly with one entry-local
  `Ptr<Values>`.
- [x] (2026-08-29) Audited generic SSA construction, address elimination, and out-parameter
  lowering. The pointer escape is intentional; existing passes cannot erase it without a new
  module-wide call-signature transform.
- [x] (2026-08-29) Added exact builder ABI revision 11 for generic local storage and generic
  pointer-valued function parameters, including fake and real-provider validation.
- [x] (2026-08-29) Admitted canonical scalar-struct helper results, scalar-struct locals, and
  matching `BorrowInOutParam` parameters in direct preflight and emission.
- [x] (2026-08-29) Added focused fake/real coverage and registered runtime/PTX lanes for the
  existing comparison shader.
- [x] (2026-08-29) Formatted, built, ran focused/full/CUDA validation, assembled PTX, completed
  self-review, updated durable docs and this plan, and prepared the complete slice commit.

## Surprises and Discoveries

- This local is materially different from Slice 95's vector temporary. `Values.next` mutates the
  same state across repeated calls, so the pointer is part of the canonical program semantics and
  not merely an unpromoted whole-value temporary.
- `lowerOutParameters(..., true)` can create a value-returning wrapper, but it does not rewrite all
  uses of a multiply-called original function. Applying it here would leave the old pointer helper
  alive; a complete value-threading solution would be a new module-wide transformation.
- The LLVM provider already supports typed generic pointers, aggregate loads/stores, and struct
  field GEPs. It currently lacks only local allocation and deliberately excludes pointers from its
  generic function-value classifier.
- The canonical call deliberately relates `Ptr<Values>` at the caller to
  `BorrowInOutParam<Values>` at the callee. Requiring ordinary `isTypeEqual` would reject the valid
  method ABI, while accepting arbitrary pointer pairs would erase a meaningful language contract.
- Reachable selected struct definitions remain module globals after linking. Preflight must retain
  the exact definitions referenced by accepted function results, parameters, and locals while it
  continues to reject unrelated global state.

## Decision Log

- Decision: preserve the canonical by-reference helper ABI with a typed generic pointer.
  Rationale: `BorrowInOutParam<Values>` is intentionally produced for a mutating method and the
  same local address is passed across several calls. A target-only value-threading rewrite would
  duplicate an existing semantic representation and require a substantial call-graph transform.
  Date/author: 2026-08-29, Codex.
- Decision: add local allocation as one exact structural builder operation.
  Rationale: the provider owns LLVM instruction construction and insertion-point validity. The
  compiler facade should request storage for a complete provider type, alignment, and name without
  exposing LLVM objects or textual IR.
  Date/author: 2026-08-29, Codex.
- Decision: limit this slice to nonempty selected scalar structs and their direct mutable helper
  pointers.
  Rationale: that is the complete measured source family. Arrays, vectors, arbitrary pointer
  address spaces, pointer phis, and returned pointers need independent source evidence and remain
  rejected.
  Date/author: 2026-08-29, Codex.

## Context and Current Pipeline

Consider the existing shader's state object:

    struct Values
    {
        int m_index;
        half next() { return values[m_index++]; }
    };

Final linked IR has a value-returning constructor with an internal `var Ptr<Values>`, a
`Values.next : Func(Half, BorrowInOutParam<Values>)` helper, and one entry-local `Ptr<Values>` passed
to every `next` call. Field mutation is already expressed canonically as `fieldAddress`, `load`,
and `store`. The direct backend currently accepts these operations only for established global or
resource pointers and has no way to create the local pointer.

LLVM's faithful representation is an entry-block `alloca %Values`, ordinary typed GEP/load/store,
and a helper parameter `%Values*`. This keeps the source mutation semantics explicit and lets LLVM
perform any safe scalar replacement later.

## Scope and Non-Goals

In scope:

- a builder operation that creates aligned generic local storage in the current function;
- generic-pointer parameters in non-variadic direct calls;
- first-class selected scalar structs as helper results and local load/store values;
- exact `BorrowInOutParam<scalar-struct>` helper parameters;
- canonical struct field address/load/store through selected local/by-reference pointers;
- the complete existing Half vector comparison as runtime/PTX evidence.

Out of scope:

- arbitrary local arrays/vectors/matrices/resources, address-space casts, pointer arithmetic,
  pointer returns, pointer phis, recursive calls, alias attributes, or lifetime intrinsics;
- converting by-reference helpers to value-returning wrappers;
- general aggregate layout beyond the existing selected scalar-struct family.

## Architecture and Invariants

- Local storage is emitted only for canonical function-local `IRVar` whose pointee is one selected
  scalar struct. It remains a real pointer because preflight proves accepted uses are loads, stores,
  selected field addresses, or exact helper arguments.
- A helper by-reference type is accepted only when it is exactly
  `BorrowInOutParam<selected-scalar-struct>`. Its provider type is the same generic typed pointer as
  the corresponding local `Ptr<Struct>`.
- Generic pointer values may cross a direct call only as parameters. They remain forbidden as
  results, phis, arithmetic operands, and ordinary value operations.
- Alignment comes from the selected struct's established physical field layout. Every emitted
  aggregate load/store uses that same natural alignment.
- The provider creates `alloca` in the owning function's entry block so loop-local source
  placement cannot cause repeated dynamic allocation.

## Interfaces and Dependencies

Advance `SLANG_NVVM_BUILDER_ABI_REVISION` and add one required construction callback plus facade
method for local storage. Extend the LLVM provider's generic-call type validation to accept typed
generic pointers as parameters while keeping value-only validation for returns and phis. Extend
the fake builder with exact storage/type traces.

Add narrow type classifiers and natural-alignment support in
`slang-emit-nvvm-type-lowering.*`, then update direct function closure validation, instruction
preflight, availability validation, and emission in `slang-emit-nvvm.cpp`. Add builder/emitter
unit coverage and test directives to the existing shader.

Validation uses the configured Release host build, standalone LLVM provider, CUDA 12.9, and
`ptxas -arch=sm_70`. Builds and tests run outside the sandbox per repository instructions.

## Milestones

1. Add and validate entry-block generic local storage in the facade, fake builder, and LLVM
   provider; cover success and malformed/cross-module calls.
2. Lower selected scalar-struct helper results and exact mutable aggregate parameters to
   first-class struct and typed generic pointer representations.
3. Validate and emit local `var`, aggregate load/store, local field addresses, and matching helper
   pointer arguments without relaxing unrelated pointer shapes.
4. Add focused fake tests and direct runtime/PTX lanes for `half-vector-compare.slang`, then compile
   and assemble its PTX.
5. Format, build, run the complete NVVM and changed-shader prefixes, perform the input-shape and
   special-case audit, update durable documents and this plan, and commit.

## Validation and Acceptance

Acceptance requires builder unit coverage for exact local allocation and pointer calls; focused
fake traces for the constructor and repeated stateful helper calls; the complete
`slang-unit-test-tool/nvvm` prefix; direct runtime and PTX lanes for the existing shader; CUDA 12.9
PTX assembly; pinned clang-format 17; and `git diff --check`.

Completed evidence:

- The standalone LLVM provider and Release host `slang-unit-test`, `slangc`, and `slang-test`
  targets build successfully.
- `nvvmIRBuilderBuildsLocalAggregatePointerCalls`,
  `nvvmSlangStatefulAggregateHelpersUseGenericLocalPointers`, and the existing unsupported-IR
  negative test each pass focused execution.
- The complete `slang-unit-test-tool/nvvm` prefix passes 369/369 with the standalone provider.
- All four enabled lanes in `tests/compute/half-vector-compare.slang` pass. The direct runtime lane
  returns `32, 32, 32, 32`, and its direct PTX FileCheck lane passes.
- The optimized direct module is 3,061 bytes of PTX. CUDA 12.9.86
  `ptxas -arch=sm_70` accepts it and emits a 3,688-byte cubin.
- The next direct probe, `tests/compute/half-structured-buffer.slang`, deterministically stops at
  the local `Thing` aggregate `var`; that mixed scalar/Half4 structure is the next independent
  boundary.

## Self-Review and Input-Shape Audit

Inventory every new classifier, helper, callback, and emitter branch before completion. The exact
shape is canonical and intentionally allowed: method lowering produces
`BorrowInOutParam<Values>`, the entry function owns the matching `Ptr<Values>` variable, and direct
calls pass that address to the helper. This slice must preserve that source of truth rather than
walk arbitrary operand graphs or reconstruct a value-threaded signature. Removing local storage or
the pointer-parameter admission must reproduce the measured `Values` failure; negative coverage
must prove unrelated pointer/local shapes still stop before provider mutation.

The completed inventory is:

- `asNVVMSupportedLocalScalarStructPointerType` and
  `getNVVMCopyableValueAlignment` survive as exact physical-role classifiers. They accept only the
  selected nonempty scalar-struct family and derive alignment from its canonical fields.
- `_isSupportedNVVMHelperParameterType` survives as the signature-role gate.
  `_isSupportedNVVMHelperArgumentType` survives because the canonical caller type is `Ptr<Values>`
  while the canonical method parameter is `BorrowInOutParam<Values>`; it requires that exact pair
  and ordinary pointee `isTypeEqual`, not a custom recursive equivalence.
- `NVVMStructField::isMutableLocal` survives to distinguish the intentionally mutable local/borrow
  field path from established read-only conventional-global fields.
- Reachable selected-struct collection survives because these canonical type definitions are
  referenced by accepted functions; unrelated global instructions still fail preflight.
- The provider's parameter classifier and `emitLocalStorage` survive at the shield boundary.
  Pointer admission is parameter-only, and the provider validates ownership, insertion point,
  type, and alignment before LLVM mutation.

No new fallback reconstructs syntax, walks arbitrary operand graphs, returns a silent default, or
patches textual IR. Removing any listed gate reproduces either the measured stateful-helper failure
or the focused malformed/cross-module negative cases.

## Failure and Recovery

If LLVM 14/libNVVM rejects an otherwise verified typed-pointer alloca/call module, record the exact
diagnostic and stop rather than adding textual patches or forced inlining. Generated dumps,
PTX, and cubins stay under ignored `build/`. Never reset unrelated work or stage
`external/slang-binaries/`.

## Artifacts and Hand-Off

Record provider IR, focused fake counts, runtime output, PTX/cubin sizes, full/focused test counts,
the next exact fixture stop, and the self-review inventory here. Distill the durable local-pointer
ABI into `docs/design/nvvm-backend.md` and update the capability ledger.

## Outcomes and Retrospective

The slice preserves the front end's canonical mutable-method ABI rather than teaching the direct
backend a second value-threaded representation. The builder gained one economical operation and
one generic parameter-role relaxation; all Slang-specific selection remains outside the LLVM
shield. Focused fake traces prove the compiler boundary, real-provider tests prove the LLVM graph,
the existing shader proves runtime behavior, and the unchanged unsupported-IR test proves that the
new path did not silently absorb unrelated locals.

The durable capability ledger is the Slice 96 section in `docs/design/nvvm-backend.md`. Generated
PTX and cubin artifacts remain under ignored `build/`. The next slice should begin from the measured
`Thing` local in `half-structured-buffer.slang`, not from a speculative general aggregate API.
