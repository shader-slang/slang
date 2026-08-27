# Slice 46: Add wave lane index through a generic intrinsic family

This ExecPlan follows `.agent/PLANS.md`. Keep it current. The user requires the completed plan to
ship with the slice, overriding the repository's default working-log policy for this experiment.

## Purpose and Observable Result

After this slice, the direct NVVM route compiles and runs a CUDA kernel that stores
`WaveGetLaneIndex()` through a canonical `Ptr<uint, ReadWrite, Device>`. Canonical signed and unsigned
32-bit Slang integers share LLVM's signless `i32` representation while semantic signedness remains
available to preflight. One append-only intrinsic-family callback maps the exact CUDA-selected
`GenericAsm("_getLaneId()")` helper body to LLVM's NVVM lane-id intrinsic without exposing inline
assembly text to the provider.

## Progress

- [x] (2026-08-28) Recorded Slice 45 baseline: 298 names, SHA-256
  `71658634899192b09f2d12461c25a5efb9d85c3c4f2db7c285ba35ef35d44066`, Release 298/298,
  Debug 10/10, 520-byte x64/304-byte x86 V3 table, and 22,837 measured lines.
- [x] (2026-08-28) Audited `WaveGetLaneIndex`, CUDA target selection, helper closure/signatures,
  linked IR, scalar type lowering, and LLVM 14's `llvm.nvvm.read.ptx.sreg.laneid` declaration.
- [x] (2026-08-28) Appended and negotiated one generic intrinsic-family callback and feature 34;
  V3 is 528 bytes on x64 and 308 bytes on x86.
- [x] (2026-08-28) Admitted canonical unsigned i32 values, helper results, calls/returns, device
  pointers, pointer offsets, and
  exact stores while leaving signed arithmetic policy unchanged.
- [x] (2026-08-28) Added seven independently named provider/direct/capability/PTX/assembler/runtime
  evidence layers and advanced thirteen negative UInt fixtures to their honest later boundaries.
- [x] (2026-08-28) Formatted, built, tested, hashed, measured, documented, and completed the
  input-shape audit; prepared the complete slice for commit.

## Surprises and Discoveries

- Observation: after CUDA target selection and force inlining, the canonical linked IR does not
  contain a call named `WaveGetLaneIndex` that the backend should special-case. It contains a
  reachable `func WaveGetLaneIndex : Func(UInt)` whose sole block terminates with
  `GenericAsm("_getLaneId()")`; the entry point directly calls that helper and stores the result.
  Consequence: recognize the exact target-selected terminator where it is produced, retain the
  helper/call topology, and do not search names or reconstruct source syntax.

- Observation: preflight currently stops at `helper function result type`, before reaching the
  intrinsic body, because canonical `UInt` is excluded even though LLVM integers are signless.
  Consequence: extend the semantic scalar policy to 32-bit signed or unsigned integers and use the
  existing provider `i32` representation. Keep arithmetic/comparison admission signed until a
  future slice defines each unsigned operation's contract.

- Observation: exporting `GenericAsm` text would turn a closed semantic backend boundary into an
  arbitrary inline-assembly interface.
  Consequence: append an operation enum plus generic argument-vector callback; initially admit
  only zero-argument wave lane index and reject every other string and function shape.

- Observation: LLVM 14 serializes the lane-id declaration with `nofree`, `nosync`, `nounwind`,
  `readnone`, `speculatable`, and `willreturn`; the LLVM-7-era NVVM 2.0 parser rejects the newer
  attribute names.
  Consequence: audit the exact semantic intrinsic declaration and attribute set, then narrow only
  its serialized attribute group to `nounwind readnone`, with semantic/rewrite count equality.

- Observation: every historical exact-prefix fixture starts from the current complete fake V3
  table. Once feature 34 was added, six older fixtures advertised it while truncating the table to
  an earlier slice.
  Consequence: explicitly clear feature 34 in each historical prefix model. The wrapper continues
  to reject any provider that advertises a feature without its complete suffix.

- Observation: direct NVVM PTX uses `%laneid`, while NVRTC implements the CUDA prelude helper by
  flattening `%tid.{x,y,z}` and masking the result with 31.
  Consequence: compare the launch ABI, memory behavior, route-specific lane mechanism, assembler
  acceptance, and runtime values instead of requiring identical PTX instruction selection.

- Observation: the seven evidence layers and reusable intrinsic/fake/runtime base add 531 physical
  lines across the five measured files, from 22,837 to 23,368.
  Consequence: later target intrinsics extend the operation family and reuse the harness instead of
  adding operation-specific wrapper methods or another runtime compilation loop.

## Decision Log

- Decision: make canonical 32-bit integer representation sign-agnostic, but keep the established
  signed-only operation classifiers unchanged.
  Rationale: LLVM `i32`, memory storage, calls, returns, and lane-id results do not encode
  signedness; semantic Slang types still control which arithmetic operations preflight accepts.
  Date/author: 2026-08-28, Codex.

- Decision: append feature 34 `WAVE_LANE_INDEX` and one generic intrinsic callback keyed by a
  stable operation enum, rather than a dedicated lane-index callback or raw assembly string.
  Rationale: later wave and target intrinsics can extend the enum without growing the wrapper API
  for each operation, while the provider receives only audited semantics.
  Date/author: 2026-08-28, Codex.

- Decision: retain LLVM 14 as the semantic IR producer and adapt only the lane intrinsic's exact
  optimization attributes at the negotiated NVVM 2.0 text boundary.
  Rationale: LLVM continues to construct and verify the canonical intrinsic declaration/call; the
  narrow dialect writer handles a measured parser incompatibility without string-building IR or
  weakening the semantic producer.
  Date/author: 2026-08-28, Codex.

## Outcomes and Retrospective

Feature 34 appends one generic target-intrinsic callback and stable lane-index operation. The x64
V3 table grows from 520 to 528 bytes and x86 from 304 to 308 bytes. Exact Slice 45 and earlier table
prefixes remain accepted when feature 34 is clear; partial or null advertised suffixes fail without
wrapper mutation. Unknown operations are rejected before dispatch, and failed/null provider
outputs are cleared.

The final linked IR retains the canonical UInt helper/call topology. The fake graph records the
intrinsic in helper block 1, its generic UInt return, the kernel's generic call, the same call value
as pointer offset and store value, and the original UInt device pointer. Generic LLVM and audited
NVVM 2.0 text contain one lane-id declaration/call, one helper call/return, and one store. The
legacy writer verifies LLVM 14's exact six declaration attributes and emits the LLVM-7-compatible
`nounwind readnone` group with exact count equality.

NVVM and NVRTC agree on the `[64]` launch ABI, one global 32-bit store, and no load. Direct NVVM
uses `%laneid`; NVRTC flattens the thread ID and masks with 31. CUDA 12.9 `ptxas` accepts both, and
one 32-thread RTX 5090 warp writes exactly 0 through 31 through each route. The focused Slice 45/46
matrix passes 14/14; Release passes 305/305 with sorted LF-terminated name-set SHA-256
`a5d99d25f4218d69bf938e171083e49c3826150873a58506c42e2b8bcbf98dbb`; removing the seven new
names reproduces Slice 45's 298-name hash exactly. Debug preservation passes 10/10. The five
measured files grow by 531 physical lines, from 22,837 to 23,368.

## Context and Current Pipeline

Slice 45 accepts canonical signed i32 and Float helpers and uses a generic V3 call/return pair when
the complete helper signature is not all signed i32. Type lowering maps signed i32 to provider
integer width 32 and accepts only `Ptr<int, ..., Device>` for integer device memory. Preflight
classifies ordinary instructions and terminators before provider discovery, computes a semantic
feature set, and emission reuses the same reachable function closure.

`WaveGetLaneIndex()` is a target intrinsic in `hlsl.meta.slang`. CUDA target selection materializes
its CUDA spelling as a zero-operand `IRGenericAsm` terminator in a retained UInt helper. LLVM 14.0.6
already exposes that operation as intrinsic ID `nvvm_read_ptx_sreg_laneid`, returning i32.

## Scope and Non-Goals

In scope are canonical UInt scalar representation where operations are sign-independent, UInt
device pointer parameters, lane-indexed pointer addressing, and exact stores, UInt helper
result/call/return transport, the exact
zero-operand lane-index GenericAsm terminator, a generic intrinsic-family ABI/facade/provider/fake
path, and end-to-end lane values for one warp.

Out of scope are arbitrary GenericAsm, raw inline assembly transport, wave lane count, ballots,
shuffle/reduction/vote operations, convergence policy, masks, unsigned constants/arithmetic/
comparisons/conversions, UInt loads or phis unless naturally required, wider/narrower integers,
vectors, resource UInt elements, and performance claims.

## Architecture and Invariants

Canonical `Int` and `UInt` are distinct semantic Slang types but both lower to one cached provider
`i32` handle. Exact Slang type equality remains mandatory at pointers, calls, and returns. The
provider continues to validate opaque LLVM type equality, module ownership, insertion state, and
dominance. Signedness-sensitive IR operations continue to require `Int`; this slice broadens only
sign-independent scalar transport and storage.

The intrinsic callback receives a stable semantic operation plus an argument vector. Feature 34
requires the complete appended callback. Lane index requires zero arguments and an unterminated
insertion block, emits exactly one call to LLVM's NVVM lane-id intrinsic, and returns its i32 value.
Direct preflight admits only a non-entry helper returning UInt, with no parameters, whose sole
block terminator has exact text `_getLaneId()`. It requests generic scalar functions plus wave lane
index; emission invokes the semantic intrinsic callback and terminates the helper through the
existing generic valued-return callback.

## Interfaces and Dependencies

Append one feature bit, intrinsic-operation typedef/value, callback typedef/table field/suffix
macro, and facade method. Include LLVM intrinsic declarations in the isolated provider and use its
existing Core component. Extend central type policy, direct validation/emission, fake topology,
tests, design, ledger, and this plan. Add no ABI version, V2 field, export, library component,
source rewrite, or operation-specific wrapper method.

## Milestones

1. Append and negotiate feature 34 plus the generic intrinsic-family callback.
2. Generalize sign-independent i32 type transport and exact UInt device storage.
3. Classify and emit the canonical lane-index GenericAsm helper terminator.
4. Add named negotiation, provider, direct, capability, differential, `ptxas`, and runtime tests.
5. Format/build, run focused/full/Debug lanes, hash, measure, document, audit, and commit.

## Validation and Acceptance

Build the standalone Release provider and Release/Debug main targets outside the sandbox. Run all
new names, adjacent V3 table/prefix/provider-invalid tests, signed-i32 and Float helper
preservation, unsupported matrix, full Release NVVM prefix, and Debug 10/10 preservation.

Accept exact LLVM and negotiated NVVM text containing one lane-id intrinsic declaration/call and
one UInt helper call/return; direct fake topology from intrinsic result to helper return to kernel
call to UInt store; matching NVVM/NVRTC PTX launch ABI and lane-id behavior; `ptxas` acceptance;
one-warp runtime outputs 0 through 31; old-prefix compatibility; exact test-name continuity;
formatted code; completed input-shape audit; and clean diff checks.

## Self-Review and Input-Shape Audit

The new-helper/fallback/special-case inventory is the signed/unsigned i32 type helpers and transport
validator, the exact GenericAsm classifier, the generic intrinsic callback/facade/provider path,
the fake intrinsic representation, and the legacy lane-declaration attribute rewrite. All survive
the audit:

- CUDA target selection produces the exact valid input shape: a reachable, defined, zero-parameter
  `Func(UInt)` helper whose sole block terminates in one-operand
  `GenericAsm("_getLaneId()")`. The existing finite closure and terminator walk are the source of
  truth. Removing the classifier restores the E52017 GenericAsm failure, so target-selected direct
  preflight owns this already-canonical terminal shape; it does not rediscover a source intrinsic
  name or accept any other assembly string.
- Canonical `UInt` reaches entry/helper parameters, calls, returns, a device pointer, offset, and
  store. These roles are sign-independent in LLVM's `i32`; exact Slang type equality is still
  checked before lowering. `_validateInteger32Value` delegates canonical `Int` to its existing
  constant/SSA policy and admits only available UInt SSA values. UInt constants and every
  signedness-sensitive operation retain their former classifiers. Removing UInt transport fails at
  the helper result before the intrinsic, proving the type-lowering/preflight boundary owns it.
- The generic callback transports a stable semantic operation and argument vector, never raw asm.
  The facade owns feature/operation dispatch and failed-output clearing; the provider owns opaque
  LLVM module, insertion point, signature, and intrinsic construction. The fake adds one typed
  intrinsic value kind rather than a lane-specific call/return representation, and its result flows
  through the existing generic call/return/value graph.
- LLVM 14 constructs and verifies the canonical lane intrinsic. Its exact valid declaration carries
  six optimization attributes that the NVVM 2.0 parser cannot read. The legacy writer checks the
  declaration, return/argument shape, all six attributes, and exact semantic/rewrite counts before
  narrowing that one attribute group. This is a dialect-boundary adaptation of semantic LLVM IR,
  not a second IR producer or a repair for malformed upstream data.
- The historical-prefix changes clear only the newly appended feature in deliberately truncated
  fake tables. The initial 13/14 focused result proved the prior fixture was internally
  inconsistent; retaining the wrapper rejection is the principled compatibility behavior.

## Failure and Recovery

If libNVVM rejects LLVM 14's intrinsic spelling in NVVM 2.0 text, inspect the public NVVM IR
contract and add a narrow audited writer rule only if semantic LLVM IR and exact counts remain the
source of truth. If PTX optimizes helper topology, retain LLVM/NVVM text and runtime evidence.
Removing the appended callback, UInt transport policy, and GenericAsm classifier restores Slice
45. Remove temporary probe/output files and never stage `external/slang-binaries/`.

## Artifacts and Hand-Off

Retain old/new table sizes, exact final linked-IR producer, generic intrinsic fake topology,
LLVM/NVVM text, matching PTX ABI and lane behavior, `ptxas`, RTX/NVRTC results, counts/hashes,
line growth, and completed audit. Distill durable evidence to design/ledger and ship this completed
plan with Slice 46.
