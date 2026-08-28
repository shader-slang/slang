# Slice 64: Introduce the V4 builder architecture

This ExecPlan follows `.agent/PLANS.md`. Keep it current. The user requires the completed plan to
ship with the slice, overriding the repository's default working-log policy for this experiment.

## Purpose and Observable Result

After this slice, the production provider and compiler prefer a compact V4 ABI that no longer
embeds V1/V2/V3 and does not grow a callback or feature bit for every scalar or wave overload. V4
queries independently versioned foundation, construction, and typed value-operation interfaces.
All 418 baseline NVVM tests pass through V4, while explicit V3/V2/V1 fallback remains usable.

## Progress

- [x] (2026-08-28) Audited the Slice 63 baseline and identified V3's duplicated feature/operation,
  append-only table, wrapper, provider, emitter-signature, fake, and test expansion points.
- [x] (2026-08-28) Chose a compact query-root V4 with frozen subinterfaces and typed operation
  signatures; V3 remains separate fallback rather than a nested V4 compatibility table.
- [x] (2026-08-28) Defined the compact V4 C ABI with explicitly versioned foundation,
  construction, and value-operation queries and strict host prefix validation.
- [x] (2026-08-28) Implemented provider, facade, and fake-provider V4 parity for all established
  structure, scalar, floating-point, wave, resource, atomic, and serialization capabilities.
- [x] (2026-08-28) Routed the production path through V4 first while retaining authoritative
  malformed-V4 rejection and explicit V3/V2/V1 fallback.
- [x] (2026-08-28) Formatted, built Release provider/main and Debug main, passed Release 419/419 and
  focused Debug 10/10, measured, documented, audited, and prepared the slice commit.

## Surprises and Discoveries

- Observation: V3 is 528/308 bytes because it embeds the complete V2 table, and the host validates
  individual suffixes and feature-to-callback coherence.
  Consequence: V4 must not embed a compatibility generation or append new callbacks to one root.

- Observation: one new wave overload currently changes a source signature enum, feature enum,
  intrinsic enum, feature mapping, provider switch, fake storage, and several test layers.
  Consequence: V4 negotiates one semantic operation plus a result/operand type signature.

- Observation: module lifetime/serialization, structural IR construction, and value semantics grow
  for different reasons.
  Consequence: split them into separately frozen/queryable interfaces rather than one universal
  callback table or an LLVM-shaped public surface.

- Observation: a family-only query plus a version field in the returned table would not let an old
  host request its frozen interface after the provider learns a newer version.
  Consequence: the query takes both family ID and requested version; unsupported versions return no
  interface before the host observes a table.

- Observation: the V3 scalar-control feature represents signed-i32 add, subtract, and less-than,
  not merely the first operation used to probe it.
  Consequence: temporary V4-to-V3 feature synthesis requires every operation in a bundled feature,
  preventing a partial provider from advertising a path it cannot fulfill.

- Observation: this machine's Debug test harness fails during unrelated graphics/WebGPU detection
  before NVVM tests begin.
  Consequence: the focused unit-only Debug preservation lane uses the harness-supported
  `-skip-api-detection`; Release still runs the complete PTX, `ptxas`, and runtime surface.

## Decision Log

- Decision: export a small V4 root containing immutable metadata and `queryInterface`.
  Rationale: future optional families add a new interface version or ID without changing the root.
  Date/author: 2026-08-28, Codex. Revisit only if cross-DLL table lifetime cannot be made explicit.

- Decision: define foundation, construction, and value-operation V4 subinterfaces.
  Rationale: lifecycle/serialization and structural handles benefit from typed callbacks, while
  repeated scalar and intrinsic semantics benefit from a generic typed operation descriptor.
  Date/author: 2026-08-28, Codex. Revisit if Slice 64 parity exposes an operation that cannot be
  assigned to one owner without recreating LLVM IR in the compiler.

- Decision: make the requested subinterface version an input to `queryInterface`.
  Rationale: table-internal version validation detects corruption, while an explicit request is
  what preserves old-host/new-provider compatibility across independently evolving families.
  Date/author: 2026-08-28, Codex.

- Decision: keep V1/V2/V3 immutable and load V4 first, then fall back only when the higher symbol is
  absent; a present malformed V4 provider is authoritative failure.
  Rationale: this preserves the proven loader rule and tests real compatibility rather than hiding
  provider bugs behind an older export.
  Date/author: 2026-08-28, Codex.

- Decision: keep the established typed facade methods as a temporary compatibility adapter in this
  architecture slice, backed by generic V4 operation descriptors.
  Rationale: this proves zero semantic drift across the complete emitter before Slice 65 makes one
  declarative catalog authoritative and removes duplicated mappings/tests.
  Date/author: 2026-08-28, Codex. Remove the duplication in Slice 65.

## Outcomes and Retrospective

The provider and compiler now prefer a compact V4 root that does not embed V3. The x64/x86 root is
40/36 bytes; foundation is 40/24, construction 224/116, value operations 24/16, the semantic type
descriptor 16, and the operation descriptor 40/32. Twenty-four semantic operation IDs cover all
established scalar, floating-point, and wave overloads. In particular, one all-equal ID covers
signed-i32, unsigned-i32, and float32 signatures without another callback, feature bit, or source
signature combination enum.

The host requests explicit interface version 1, validates and copies each known table, and rejects
unknown versions, missing callbacks, bad metadata, short tables, success-with-null, and a malformed
present V4 without falling back. V3/V2/V1 remain separate exports and fallback paths. The production
emitter reaches the same provider implementation through generic V4 typed descriptors; existing
facade methods and synthesized established feature bits are intentionally temporary until Slice 65.

The standalone Release provider and Release/Debug main targets build. `dumpbin /exports` shows
exactly V1, V2, V3, and V4 builder exports. The complete Release prefix passes 419/419, including
real compatible-assembly validation, CUDA 12.9 `ptxas`, and RTX/NVRTC runtime parity. The focused
Debug preservation set passes 10/10 with unrelated API detection disabled. The sorted
LF-terminated 419-name set hashes to
`c634caa999f2b191c85b37cc7885d39462bcef55406ef64dc04bd1a1d02590c9`; removing only
`nvvmIRBuilderNegotiatesV4InterfacesAndTypedOperations` yields 418 names and exactly Slice 63's
`33720ee2997610b2d1823858e1e80641d44efce3d6b09b37d0271c70ec54c929` hash. The five NVVM
test/support files grow by 558 lines from 27,950 to 28,508; Slice 65 owns the planned catalog and
test consolidation rather than obscuring this ABI migration with a simultaneous harness rewrite.

The self-review inventory retained semantic type construction/comparison, strict interface-table
validation, provider typed-signature dispatch, and the explicit V3 fallback adapter. These operate
on canonical emitter-owned semantic types or a valid older provider generation; none reconstructs
syntax, walks arbitrary IR graphs, or exposes LLVM. Unknown or malformed descriptors are rejected
before provider mutation. The review removed two misleading partial-capability behaviors: family
queries now request an exact version, and bundled scalar-control support now requires all promised
operations. The fake mirrors query-version rejection; production provider invalid/no-mutation
coverage remains the authority for exact LLVM ownership and insertion-point checks.

## Context and Current Pipeline

The compiler dynamically loads `slang-llvm-nvvm`, retains opaque module/type/value/block handles,
builds LLVM 14 IR behind that boundary, serializes audited LLVM-7-era NVVM IR 2.0 text, and sends it
to libNVVM. V3 freezes V2 as `compatibilityAPI`, appends generic scalar callbacks, and advertises 49
feature bits. The emitter separately maps canonical Slang IR to those features and operations.

V4 keeps opaque ownership and the LLVM shield. Its root queries a foundation interface for module
lifetime and serialization, a construction interface for types/functions/blocks/memory/control
flow/SSA/addressing, and a value interface that accepts stable semantic operations described by
Slang-owned result and operand type roles. The facade adapts V3 to the same internal operation model
when V4 is absent.

## Scope and Non-Goals

In scope are the V4 root and three subinterfaces, size/version validation, real and fake exports,
V4-first loading, authoritative malformed-provider behavior, generic typed scalar/wave operation
negotiation/emission, facade dispatch, full baseline parity, V3 fallback, design, ledger if status
changes, and this plan.

Out of scope are new source-language capabilities, removing legacy exports, exposing LLVM types or
opcodes, a serialized command stream, textual IR manipulation changes, broad test-file
reorganization, and numeric/vector/shared-memory expansion.

## Architecture and Invariants

Every interface begins with `structureSize` and `interfaceVersion`; the root additionally carries
ABI and LLVM/NVVM metadata. Queried provider tables remain valid for the lifetime of the retained
library. The host copies the understood prefix and rejects missing required callbacks, unknown
required versions, malformed sizes, and success without a table.

V4 value signatures use semantic type descriptors, not combinations such as `BoolUIntFloat` and
not LLVM classes. Operations identify semantics such as add, ordered comparison, read-lane-at, or
all-equal; result and operand descriptors identify Bool/signed integer/unsigned integer/float plus
width and lane count. Provider validation happens before mutation. Signed/unsigned distinctions
remain Slang-owned even where LLVM uses signless integers.

The construction interface contains stable structural actions whose argument shapes materially
differ: module types, functions, blocks, loads/stores, branches, phis, calls/returns, addressing,
kernel marking, and the established raw-resource/atomic boundary. New repeated value operations do
not add fields there.

## Interfaces and Dependencies

Modify the private ABI header, builder facade/header, provider/export list, Slang NVVM emitter,
fake provider and builder/emitter/integration tests, CMake export expectations if needed, design,
ledger only if capability state changes, and this plan. Do not change libNVVM discovery, the NVVM
IR 2.0 rewrite policy, registered compiler routing, public Slang API, or LLVM dependencies.

## Milestones

1. Freeze the V4 root/subinterface layouts, semantic type descriptors, operation IDs, and query
   rules with ABI/layout tests.
2. Implement strict facade initialization and V4-first/V3-fallback loading.
3. Populate V4 from the existing provider implementation and add generic value dispatch with
   complete pre-mutation validation.
4. Add fake-provider recording and prove representative structure, scalar, floating, wave, raw
   resource, atomic, serialization, invalid, and compatibility paths.
5. Run the full baseline and real provider lanes, document, self-review, and commit.

## Validation and Acceptance

Build Release provider/main and Debug main targets outside the sandbox. Run V4 ABI/loader malformed
and fallback tests, representative provider/facade invalid/no-mutation tests, wave/ABI 120/120,
Release NVVM 418/418, Debug preservation 10/10, real compatible-assembly verification, CUDA 12.9
`ptxas`, and at least the established representative RTX runtime parity set.

Accept only if the production load reports ABI 4; all established programs and diagnostic
boundaries remain unchanged; V3 fallback passes with the V4 symbol absent; present malformed V4
does not fall back; the root does not embed V3; new semantic overloads require no ABI callback or
feature bit; no LLVM object/type/opcode crosses the boundary; formatting and diff audit pass; and
`external/slang-binaries/` remains untouched.

## Self-Review and Input-Shape Audit

Inventory every new descriptor converter, V4-to-provider dispatch case, V3 adapter, validation
helper, and fallback. For each, identify the exact canonical Slang IR producer and prove the shape
is intentional. Reject any helper-name matching, syntax reconstruction, custom LLVM equivalence,
graph rediscovery, silently accepted unknown descriptor, or post-mutation validation.

The V3 adapter survives because an independently shipped V3 provider is valid input. It must map
from the same semantic operation model, not create a second lowering policy. The root/subinterface
split survives because the represented actions have genuinely different ABI shapes; do not hide a
bad split behind a catch-all callback solely to reduce field count.

## Failure and Recovery

If V4 cannot express an established canonical operation without raw LLVM concepts, stop and revise
the semantic descriptor rather than append a bespoke callback. If a subinterface cannot be safely
retained across the DLL boundary, copy its validated known prefix. If parity changes assembly/PTX
unexpectedly, compare V3/V4 provider calls before altering rewrites. Removing the V4 export and
facade path restores Slice 63. Never stage `external/slang-binaries/` or temporary probes.

## Artifacts and Hand-Off

Retain ABI layouts, queried interface/version inventory, operation/signature inventory, loader and
malformed evidence, V3 fallback evidence, full test names/hash, real provider assembly/PTX/runtime
results, code-size/line measurements, and the simplification audit. Distill durable architecture
and evidence into the design and commit the completed plan with Slice 64.
