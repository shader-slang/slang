# Promote aggregate-backed dynamic dispatch

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, direct NVVM can compile and execute the existing
`tests/compute/dynamic-dispatch-13.slang` and `tests/compute/dynamic-dispatch-15.slang` fixtures. The
CUDA varying legalizer has already converted their existential interface values and witness calls
into concrete copyable payload structs, integer tags, ordinary helpers, and switches. Direct NVVM
must transport those canonical aggregates without recovering source interfaces or adding a
dynamic-dispatch-specific provider API.

## Progress

- [x] (2026-08-29) Completed Slice 124 as `4f043d13c`; Release provider/host builds and the complete
  NVVM prefix passed 392/392.
- [x] (2026-08-29) Re-probed both target fixtures at SM70 with direct NVVM and captured the shared
  first diagnostic, `raw structured-buffer numeric load`.
- [x] (2026-08-29) Captured final optimized IR for `dynamic-dispatch-13`: the former existential is
  a layout-compatible 24-byte tuple of two UInt2 tags plus a nested two-UInt payload, and dispatch
  is an ordinary UInt switch over concrete helper calls.
- [x] (2026-08-29) Generalized canonical structured-buffer value loads from numeric-only to the
  already-admitted exact copyable resource-element family. Focused fake coverage proves a loaded
  aggregate remains first class and can feed keyed field extraction.
- [x] (2026-08-29) Compiled both fixtures iteratively. Their remaining shared boundaries were
  generated complex bit casts, `makeStruct`, integer-switch unreachable defaults, and field reads
  from entry-point-produced aggregate values. `dynamic-dispatch-15` additionally required a local
  fixed array of copyable structs.
- [x] (2026-08-29) Promoted direct runtime/PTX lanes for both fixtures. They pass 6/6 and 5/5;
  1,150-byte and 4,793-byte PTX assemble with CUDA 12.9.86 `ptxas -arch=sm_70` to 2,920-byte and
  4,968-byte cubins.
- [x] (2026-08-29) Ran final Release provider/host builds, repaired an over-narrow local-array
  pointer consumer found by the first full-suite pass, and completed the NVVM prefix at 393/393.
  Formatting, self-review, fixture revalidation, and `git diff --check` also pass.

## Surprises and Discoveries

- Resource type lowering, pointer formation, and the later exact validator already accept the
  layout-compatible copyable struct element. The earlier preflight gate and the emission alignment
  still say “numeric” and call the numeric-only classifier. This is an internal contract mismatch,
  not evidence that the legalized tuple is an invalid resource representation.
- The optimized `dynamic-dispatch-13` representation is concrete before direct emission. Its
  `RWStructuredBuffer<IInterface>` inputs become `RWStructuredBuffer<Tuple>`, where `Tuple` has
  fields at offsets 0, 8, and 16 and natural size 24/alignment 4. Witness lookup becomes a UInt tag
  switch and ordinary direct helper calls. The NVVM backend should therefore remain unaware of the
  source interface.
- Any-value marshalling runs after the common required-pass inventory and can introduce complex
  aggregate bit casts even when the source module contained none. Running `lowerBitCast` at the
  direct-NVVM handoff exposes the promised leaf-level canonical form. That lowering also exposed
  redundant same-type bit casts, so `IRBuilder::emitBitCast` now preserves the existing value for
  identity casts at the construction boundary.
- LLVM has no first-class aggregate `bitcast`. Complex bit-cast lowering instead creates ordinary
  `makeStruct` values from leaf casts, which fit the existing generic aggregate constructor. The
  direct emitter's resolver had artificially limited that callback to `makeArray`; admitting exact
  copyable structs required no builder ABI change.
- The dispatch switches contain real unreachable default blocks. Forward-only builder ABI revision
  22 adds the generic `emitUnreachable` terminator and validates an active unterminated block. A fake
  return would change the control-flow contract.
- Entry-point struct parameters are pointer-backed because NVPTX carries them with `byval`, but an
  aggregate loaded inside the entry point is an ordinary first-class value. The previous emission
  branch classified every field read in the entry function as pointer-backed. It now keys on the
  actual `IRParam` producer, matching the physical representation selected during type lowering.
- `dynamic-dispatch-15` materializes `Ptr<Array<Tuple, 2>>`, stores two concrete erased payloads,
  and dynamically loads one. The new copyable-array classifier admits only nonempty natural-stride
  arrays of already-admitted numeric values or layout-compatible copyable structs; byte-address,
  device-array, and parameter-group classifiers remain separate and unchanged.
- The first complete NVVM run passed 391/393 and caught an input-role regression before provider
  mutation. Sequential indexing and pointer validation had replaced the established numeric-array
  pointer classifier with the new local-copyable-array classifier. That incorrectly rejected the
  canonical `OutParam<Array<float3, 4>>` and matrix-array helper parameters produced by legalization.
  Both consumers now retain the numeric pointer family first and use the exact compact
  `Ptr<Array<CopyableStruct, N>>` family only as a fallback. The two focused regressions and the
  subsequent 393-test run prove the roles remain distinct.

## Decision Log

- Decision: target both `dynamic-dispatch-13` and `dynamic-dispatch-15` as one larger prototype
  slice, starting from their shared aggregate structured-load boundary.
  Rationale: the fixtures exercise two sizes of the same legalized interface-payload strategy. A
  single proof followed by the adjacent family is more economical than one tiny slice per IR op.
  Date/author: 2026-08-29, Codex.
- Decision: use the existing copyable-type/resource classifiers and generic aggregate builder
  operations rather than introduce an existential, interface, or dynamic-dispatch callback.
  Rationale: all source-level existential operations have already been legalized away; the
  concrete optimized IR is the semantic source of truth at this layer.
  Date/author: 2026-08-29, Codex.
- Decision: normalize generated aggregate bit casts with the existing IR lowering pass rather than
  teach the provider to reinterpret LLVM aggregates.
  Rationale: LLVM aggregate bitcasts are not a legal first-class operation, while the established
  lowering already reconstructs the destination from exact leaf offsets and types.
  Date/author: 2026-08-29, Codex.
- Decision: add one generic unreachable callback, and otherwise compose existing type, load,
  pointer, aggregate-construction/extraction, call, switch, and return operations.
  Rationale: unreachable is a genuine missing CFG terminator; every other observed boundary was an
  unnecessarily narrow compiler-side classifier or producer distinction.
  Date/author: 2026-08-29, Codex.
- Decision: preserve the established numeric-array `Ptr`/`OutParam`/`BorrowInOut` family in generic
  sequential-pointer consumers and add copyable struct arrays only as an exact local-`Ptr`
  fallback.
  Rationale: helper-reference legalization intentionally produces the broader numeric pointer
  spellings, while the new fixture proves only local storage of copyable structs. Combining these
  producer roles in one widened classifier would admit unsupported copyable-array helper ABIs.
  Date/author: 2026-08-29, Codex.

## Context and Current Pipeline

The important part of final optimized `dynamic-dispatch-13` is equivalent to:

    struct AnyValue8 { uint field0; uint field1; }
    struct Tuple { uint2 typeTags; uint2 witnessTags; AnyValue8 payload; }

    Tuple v0 = rwstructuredBufferLoad(gCb, 0);
    uint case0 = selectConcreteCase(v0.witnessTags.x);
    AnyValue4 payload0 = unpackAnyValue8(packAnyValue8(v0.payload));
    int result0 = dispatchRun(case0, payload0, threadID);

The resource classifier already admits `Tuple` because it is a nonempty, recursively numeric-field,
layout-compatible copyable struct. The direct preflight nevertheless requires
`isNVVMSupportedNumericValueType` for `rwstructuredBufferLoad`, and emission asks for numeric-only
alignment. The later exact validation correctly compares the load result to the resource element.

## Scope and Non-Goals

In scope are canonical structured-buffer loads of already-admitted copyable resource elements;
their exact natural alignment and invariant policy; first-class nested aggregate transport needed
by the two fixtures; concrete helper calls/returns and UInt switches already present after varying
legalization; focused fake and real provider tests; direct runtime and PTX lanes; durable design
status; and this plan.

Out of scope are source interfaces or witness tables in direct emission, unspecialized indirect
calls, arbitrary aggregate layouts, padding or overlapping fields, runtime-sized arrays, recursive
types, nonnumeric fields, new provider callbacks, compatibility aliases, unrelated dynamic-dispatch
fixtures with different first boundaries, and any operation not reached by these two optimized
programs.

## Architecture and Invariants

- Resource element admission remains centralized in `getNVVMSupportedRawBufferType`; load result
  type must exactly equal the buffer's structured element type.
- Aggregate physical layout must remain the already-proven layout-compatible copyable contract.
  Direct emission never invents offsets or reconstructs interface syntax.
- Structured loads extract the existing resource data pointer, apply the exact element index, and
  load with `getNVVMCopyableValueAlignment`; read-only StructuredBuffer loads remain invariant,
  while RWStructuredBuffer loads do not gain that flag.
- Helper signatures, calls, returns, phis, switches, and aggregate operations continue through the
  generic builder interfaces with exact module ownership, dominance, and type equality.
- Any newly retained special case must name its canonical producer and a fixture that fails without
  it; otherwise remove it.

## Interfaces and Dependencies

Expected committed areas are direct NVVM validation/emission and type-lowering reuse, focused
fake/real-provider tests, the two existing compute fixtures, `docs/design/nvvm-backend.md`, and this
plan. No builder ABI change is expected unless an actual missing generic operation is demonstrated.
CUDA 12.9 runtime and `ptxas -arch=sm_70` provide end-to-end evidence.

## Milestones

1. Replace the numeric-only structured-load preflight/alignment with the existing exact raw-buffer
   element and copyable-value contracts; add focused aggregate resource-load coverage.
2. Recompile both fixtures, capture each next optimized-IR boundary, and extend only generic
   copyable aggregate/control-flow operations already represented by the builder.
3. Promote direct runtime and PTX lanes for both fixtures, inspect exact output, and assemble it
   with CUDA 12.9.86 `ptxas`.
4. Run Release provider/host builds and the complete NVVM prefix, update docs and this log, format,
   perform the input-shape audit, and commit.

## Validation and Acceptance

Acceptance requires focused fake coverage proving aggregate resource pointer/index/load topology,
exact aggregate type and alignment, and preserved invariant policy; real-provider LLVM/legacy
serialization where a newly exercised operation needs it; all lanes of both promoted fixtures;
PTX with expected SM70 entry points, loads, dispatch branches, and output stores; CUDA 12.9
`ptxas -arch=sm_70`; Release host/provider builds; the complete `slang-unit-test-tool/nvvm` prefix;
pinned formatting; and `git diff --check`.

The self-review inventories every helper, fallback, classifier widening, builder rule, and special
case. For each retained change, record the exact optimized producer and failing fixture. Reject any
source-name match, interface reconstruction, duplicate layout classifier, permissive aggregate
equivalence, inferred padding, provider-only fallback, or compatibility shim.

## Failure and Recovery

If either fixture retains source existential operations, indirect calls, invalid layouts, or another
boundary too large to complete coherently, preserve its optimized IR and diagnostics under ignored
`build/slice125-*`, narrow this plan around the largest demonstrable shared subset, and record the
remaining boundary. Do not weaken the fixture, specialize by source name, bypass exact type checks,
reset unrelated work, or stage `external/slang-binaries/`.

## Artifacts and Hand-Off

Keep generated IR, LLVM text, PTX, cubin, and logs under ignored `build/slice125-*`. Distill the
aggregate dynamic-dispatch contract, exact CUDA evidence, and next measured corpus boundary into
`docs/design/nvvm-backend.md`, then commit this plan with the implementation as explicitly requested.

## Outcomes and Retrospective

Direct NVVM now executes both target dynamic-dispatch fixtures without source-interface knowledge.
The implementation centralizes exact aggregate structured loads, lowers late complex bit casts at
the direct-NVVM handoff, constructs exact structs through the generic aggregate callback, preserves
first-class field extraction, supports dynamic local arrays of copyable structs, and represents
impossible switch defaults with ABI revision 22's generic unreachable terminator.

The self-review inventory retained seven principled additions: the structured-load resolver
consolidates three formerly duplicated exact checks; late `lowerBitCast` repairs an upstream pass
ordering gap exposed by any-value marshalling; identity-bitcast folding preserves canonical IR at
construction; `makeStruct` reuses the existing aggregate operation; `emitUnreachable` models a real
CFG terminator; entry-field emission keys on the exact `IRParam` producer; and the copyable-array
classifier owns the exact local `Var<Array<Tuple, 2>>` shape. The exact-op diagnostic also remains
because it reports the actual rejected canonical instruction. Broader experimental changes for
helper parameters, block parameters, whole-array loads, static aggregate indexing, selected-value
arrays, and storage roles were reverted because neither fixture required them. The full-suite
regression drill further restored numeric helper-array pointer handling while keeping copyable
arrays local-only. No custom equivalence, syntax reconstruction, source-name matching, permissive
fallback, or compatibility shim remains.

Final evidence is: focused aggregate coverage 1/1; repaired matrix-memory and helper-array tests
1/1 each; fixture lanes 6/6 and 5/5; CUDA 12.9.86 SM70 assembly for both generated PTX artifacts;
complete Release provider and host builds; and the complete NVVM prefix at 393/393.
