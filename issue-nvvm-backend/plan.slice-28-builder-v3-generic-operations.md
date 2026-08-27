# Slice 28: Introduce a generic V3 NVVM builder interface

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user requires each
completed slice plan to ship with its implementation, so this plan will be committed with Slice 28.
It is queued behind Slice 27 and does not become the active ExecPlan until Slice 27 is committed.

## Purpose and Observable Result

After this slice, future scalar operations no longer require a new provider callback, host wrapper,
minimum-size prefix, capability method, fake callback, and identity string field for every Slang IR
opcode. A new private `SlangNVVMBuilderAPI_V3` expresses established integer unary, binary, and
comparison operations through three generic callbacks and Slang-owned stable operation enums. It
advertises independent semantic feature bits instead of one ordered "highest slice" capability.

The host prefers V3, uses frozen V2 only when the V3 export is absent, and rejects a present but
malformed V3 provider without falling back. Every program supported through Slice 26 produces the
same verified NVVM IR/PTX/runtime behavior through the real V3 provider, while exact older V2
providers retain their established subset through an internal compatibility adapter.

## Progress

- [x] (2026-08-27) Audited 32 provider construction callbacks, 17 ordered V2 minimum-size
  constants, 164 integer-operation references across the provider boundary, and the linear
  `NVVMIRCapability` enum.
- [x] (2026-08-27) Selected an independent V3 table plus frozen V2 fallback rather than extending
  V2 or breaking all old experimental providers.
- [x] (2026-08-27) Froze the 384-byte x64/212-byte x86 V2 layout and composed it as V3's immutable
  compatibility core; added a 256-bit feature set and three generic scalar-family callbacks.
- [x] (2026-08-27) Migrated established scalar emission and capability gating to the generic facade while
  retaining old-provider behavior.
- [x] (2026-08-27) Proved ABI negotiation, malformed-provider behavior, exact lowering, focused/preservation,
  `ptxas`, and runtime evidence; documented the settled boundary and applied pinned formatting.

## Surprises and Discoveries

- Observation: V2 already contains a partial generic design followed by per-slice specialization.
  Evidence: ADD/SUB share `SlangNVVMEmitIntegerBinary_2`, but multiply, bitwise operations, unary
  operations, and five comparisons each have dedicated callbacks and ordered minimum sizes.
  Consequence: V3 can generalize a behavior already proven at the ABI boundary instead of
  introducing an untested catch-all operation mechanism.

- Observation: the current capability enum is an ordering, not a set.
  Evidence: `_requireCapability` keeps the numerically greatest enum, so requiring the Slice 26
  resource prefix implicitly requires every earlier scalar callback.
  Consequence: V3 feature negotiation must represent independent families and test subset/superset
  relationships rather than prefix order.

- Observation: repeating the complete non-scalar V2 surface in an "independent" V3 table would
  duplicate lifecycle, serialization, memory, CFG, SSA, function, addressing, atomic, and resource
  callback slots without making future scalar growth cheaper.
  Evidence: the frozen V2 table has 32 construction/serialization callbacks, while only 13
  established scalar callbacks belong to the three repeated-signature families.
  Consequence: V3 is an independently versioned export, but it composes the complete frozen V2
  table as one compatibility core and puts only forward-growing semantics in V3 fields.

- Observation: x86's eight-byte alignment for the feature words creates both interior and terminal
  padding.
  Evidence: x86 offsets are feature words 224, unary 256, binary 260, compare 264; the terminal
  callback minimum is 268 while `sizeof(V3)` is 272. x64 has no padding gap and is 448 bytes.
  Consequence: the strict-C probe requires the minimum to fit rather than equal `sizeof`, and the
  unit test freezes both layouts explicitly.

- Observation: the first Debug preservation attempt was interrupted by the unrelated WebGPU API
  availability probe before registered tests ran.
  Evidence: verbose output stopped at the `render-test ... -wgpu` startup command. Re-running the
  exact preservation selectors with `-skip-api-detection` passed 10/10.
  Consequence: this is recorded as environmental test-harness evidence, not a backend failure.

## Decision Log

- Decision: freeze V1/V2 and add a clean V3 export rather than mutating V2 in place.
  Rationale: the experiment has explicit older-provider compatibility evidence. V3 gives future
  work an economical surface without making the cleanup depend on a packaging decision that has
  not yet been settled.
  Date/author: 2026-08-27, Codex.
  Revisit when: maintainers decide provider and host are always shipped atomically and explicitly
  authorize removal of all V1/V2 compatibility.

- Decision: V3 operations use Slang-owned enums, never LLVM enum numeric values.
  Rationale: LLVM versions are isolated implementation details and their C++ ABI must not cross the
  DLL. The provider maps stable wire operations to LLVM 14 instructions and validates types,
  ownership, availability, dominance, and insertion state before mutation.
  Date/author: 2026-08-27, Codex.
  Revisit when: an operation cannot be represented without exposing unstable LLVM structure; keep
  such an NVVM-specific semantic operation dedicated instead.

- Decision: preserve dedicated callbacks for structurally distinct operations in this slice.
  Rationale: branches, phis, calls, GEP forms, atomics, kernel annotations, serialization, and the
  raw resource ABI do not become clearer by forcing them through an untyped universal `emitOp`.
  Slice 28 consolidates only the repeated scalar unary/binary/comparison families proven to share
  validation and result rules.
  Date/author: 2026-08-27, Codex.
  Revisit when: a later family demonstrates another coherent typed signature.

- Decision: make V3 contain the complete frozen V2 table as one compatibility core rather than
  repeat every established callback as a second top-level V3 field.
  Rationale: V3 discovery/versioning and feature negotiation remain independent, while the shared
  core guarantees that lifecycle and structurally distinct operations have one wire spelling. New
  same-shaped scalar operations grow only an enum, mapping, provider switch case, and tests.
  Date/author: 2026-08-27, Codex.
  Revisit when: V1/V2 compatibility is intentionally removed; a later clean ABI can then omit the
  compatibility core entirely.

## Outcomes and Retrospective

The V2 table remains byte-for-byte 384 bytes on x64 and 212 bytes on x86. V3 is 448 bytes on x64;
on x86 its terminal callback minimum is 268 bytes and its padded complete size is 272 bytes. Four
64-bit words provide capacity for 256 semantic features. Bits 0 through 19 are, in order: scalar
memory, scalar control flow, scalar SSA, scalar functions, pointer arithmetic, array addressing,
integer multiply, AND, OR, XOR, NOT, negate, relaxed global-i32 atomic add, NVVM IR 2.0 assembly,
integer equal, not-equal, signed-greater, signed-less-equal, signed-greater-equal, and raw
`RWStructuredBuffer<int>` storage. The complete provider advertises word zero `1048575` and three
zero words.

The V2 adapter maps ADD/SUB and signed-less-than through the old control-flow prefix; multiply,
AND/OR/XOR, NOT/negate, and the five later comparisons through their exact old callbacks. Every
other established operation continues through the embedded compatibility table. The production
emitter now calls only the generic unary/binary/comparison facade for those scalar families.

Release provider and host builds passed. The focused prefix passes 192/192, including real V3
selection, every established direct/NVRTC differential PTX test, both `ptxas` classes, and the full
RTX 5090 runtime matrix. No established PTX classification or runtime result changed. Debug
preservation passes 10/10. `dumpbin` shows exactly V1/V2/V3 exports and only KERNEL32 plus delayed
SHELL32/ole32 dependencies; LLVM remains statically isolated. Pinned clang-format 17.0.6 and
`git diff --check` pass.

Self-review inventory: `_addFeature`, `_hasFeature`, and `_getV2Features` translate negotiated wire
semantics and do not inspect compiler IR; the three provider V3 dispatchers select existing
validated LLVM producers and reject unknown enums before mutation; `_fillBuilderAPIV2/V3` gives
standalone and composed exports one construction source of truth; the three facade methods perform
the explicit frozen-V2 compatibility mapping; and `_requireFeature` replaces the lossy maximum
operation without changing IR legality. All survive. There is no new AST/IR equivalence,
substitution, operand walk, syntax reconstruction, malformed-shape guard, or producer-side
special case. The exact shape reaching emission remains the Slice 26 canonical linked IR produced
by `linkAndOptimizeIR` and validated by `_validateNVVMFunction`; this slice changes only how those
already accepted operations are described across the provider boundary. Removing V3-first strict
loading fails `nvvmIRBuilderPrefersV3AndRejectsMalformedPresentV3`; removing feature independence
fails `nvvmIRBuilderNegotiatesV3Features`; removing generic dispatch fails
`nvvmSlangV3RoutesGenericScalarFamilies` and the real integration matrix.

## Context and Current Pipeline

`source/compiler-core/slang-nvvm-ir-builder-api.h` defines opaque module/type/value/block handles,
V1 core construction, and an append-only V2 table. `NVVMIRBuilder::load` discovers the exported
table, validates every coherent prefix, retains the provider library, and exposes one C++ method per
callback. `source/slang/slang-emit-nvvm.cpp` preflights linked IR into a single ordered
`NVVMIRCapability`, `source/slang/slang-emit.cpp` tests the corresponding wrapper predicate, and the
emitter invokes the specialized methods. `source/slang-llvm-nvvm/slang-llvm-nvvm.cpp` maps each
callback to LLVM 14 construction and serialization.

This boundary successfully prevents LLVM objects from entering Slang, but every scalar slice has
grown all four layers. V3 changes the wire vocabulary, not the ownership boundary.

## Scope and Non-Goals

In scope are a private V3 export, stable scalar operation enums, generic scalar callbacks, a
feature-bit set, V3-first discovery, strict malformed-V3 behavior, a V2 compatibility adapter,
provider identity updates, migration of established scalar operations, strict-C layout probes,
export allowlists, and focused tests.

Out of scope are new Slang IR operations, new scalar types, floating point, vectors, matrices,
casts, new address spaces, resource expansion, changing LLVM or libNVVM versions, removing V1/V2,
one universal untyped operation callback, and performance claims without measurements.

## Architecture and Invariants

V3 is an independently versioned C-compatible table. It contains the complete frozen V2 table as
one compatibility core for established module lifecycle, verified serialization,
type/function/block construction, memory/control-flow/SSA/function/addressing, and atomic/resource
operations, followed by these consolidated scalar families:

```text
emitIntegerUnary(operation, value, outValue)
emitIntegerBinary(operation, left, right, outValue)
emitIntegerCompare(predicate, left, right, outValue)
```

Initially declared enum values cover only the established behavior: bitwise NOT and wrapping
negation; ADD, SUB, MUL, AND, OR, and XOR; signed LT, EQ, NE, signed GT, signed LE, and signed GE.
Do not reserve speculative enum values. Unknown values fail without mutation.

The V3 header reports its complete table size, ABI version, LLVM/NVVM dialect tuple, pointer model,
and fixed-width feature words. Required core callbacks are non-null. Feature bits advertise
independent semantic families such as scalar memory, control flow, SSA, functions, addressing,
integer unary/binary/compare, relaxed global i32 atomic add, NVVM-2.0 assembly, and raw resource
storage. The direct emitter accumulates a feature set and requires that the provider contain every
requested bit. Table size describes wire compatibility; feature bits describe semantics.

If the V3 export exists but returns an invalid table, loading fails. Only absence of the V3 symbol
permits V2 discovery. The facade maps generic operations to established V2 callbacks when using an
old provider. New providers implement V2 as frozen compatibility adapters over the same internal
LLVM helpers; new semantic work adds V3 enum values or coherent callbacks/features only.

## Interfaces and Dependencies

Expected production files include:

- `source/compiler-core/slang-nvvm-ir-builder-api.h` for V3 wire enums/table/export;
- `source/compiler-core/slang-nvvm-ir-builder.{h,cpp}` for discovery, feature sets, identity, and
  V2/V3 facade dispatch;
- `source/slang-llvm-nvvm/slang-llvm-nvvm.cpp`, its C probes, and platform export allowlists for the
  provider implementation;
- `source/slang/slang-emit-nvvm.{h,cpp}` and `source/slang/slang-emit.cpp` for feature-set preflight
  and generic scalar calls; and
- the Slice 27 builder/emitter/integration test owners.

No public Slang API changes. No LLVM type or allocator-owned storage crosses the C ABI. V3 remains
private and versioned independently of libNVVM's own version.

## Milestones

1. Freeze the exact V2 x64/x86 layout, exports, identity, and established old-provider fixtures.
   Prototype an independent V3 table with scalar-family enums and fixed feature words. Promotion
   requires clean C/C++ layout probes, no LLVM types in the header, and fewer forward-growing
   callback families than V2; discard any universal descriptor design that weakens typed
   validation or requires serialized operands.
2. Implement provider-side generic scalar validation and construction from one source of truth.
   Make the V2 specialized exports call those helpers so V2 and V3 cannot drift semantically.
3. Implement V3-first loading, strict invalid-present behavior, feature-set identity, and a V2
   compatibility adapter in `NVVMIRBuilder`. Expose generic facade methods only to new emitter code.
4. Replace the linear `NVVMIRCapability` maximum with a feature set. Migrate every established
   unary/binary/comparison lowering to the generic facade without changing accepted Slang IR.
5. Add negotiation matrices for missing/malformed V3, independent feature subsets, unknown enums,
   no-mutation failures, exact V2 fallback, and V3/V2 output equivalence. Update the provider export
   allowlists and DLL-surface checks.
6. Run focused, preservation, real-provider, differential PTX, `ptxas`, and runtime evidence;
   update design/ledger and complete the plan.

## Validation and Acceptance

Build the isolated Release provider and Slang Release/Debug test targets outside the sandbox. Run
the full focused NVVM prefix and established Debug preservation 10/10. Run every established real
direct/NVRTC differential PTX, both relevant `ptxas` lanes, and GPU runtime matrix; no accepted or
rejected Slang program may change merely because V3 is selected.

ABI tests must cover x64/x86 sizes and offsets, strict-C compilation, exact V2 providers from prior
prefixes, absent V3 fallback, future-larger V3 clamping if the table permits it, missing core V3
callbacks, unknown required feature bits, and failure/no-mutation for every generic enum family.
Provider inspection must show only the explicitly allowed V1/V2/V3 exports and no process-visible
LLVM DLL. `git diff --check` and pinned formatting must pass.

Acceptance additionally requires a code audit showing that adding another established-width
integer binary or comparison operation would require an enum/mapping/test case rather than another
provider field, wrapper method, prefix macro, capability predicate, and identity field.

## Failure and Recovery

V3 is additive. If its prototype fails the promotion criteria, remove the V3 files/fields and leave
the frozen V2 path untouched; record the evidence before selecting a different family shape. If V3
output differs, compare provider assembly before libNVVM and fix the shared producer rather than
normalizing PTX. Never fall back from a malformed present V3 to V2 or NVRTC, because that would hide
a broken deployment.

Do not delete or stage `external/slang-binaries/`. Remove ABI probes and generated binaries that
are not durable tests before committing.

## Artifacts and Hand-Off

Retain V2/V3 layout tables, feature assignments, discovery/fallback matrix, generic-operation
assembly, export/dependency inspection, focused/preservation/PTX/`ptxas`/runtime results, and the
callback-growth comparison in this plan. Distill the settled V3 contract into
`docs/design/nvvm-backend.md` and capability status into the ledger. Commit this completed plan with
Slice 28; leave Slice 29's plan uncommitted until its implementation is complete.
