# Slice 62: Admit unsigned loads and add public UInt wave-active-all-equal

This ExecPlan follows `.agent/PLANS.md`. Keep it current. The user requires the completed plan to
ship with the slice, overriding the repository's default working-log policy for this experiment.

## Purpose and Observable Result

After this slice, direct NVVM compiles and runs public `WaveActiveAllEqual(uint)`. The direct scalar
memory path admits canonical unsigned 32-bit load results, and the exact
`WaveMaskAllEqual(activeMask, uintValue)` helper lowers through the established native
`llvm.nvvm.match.all.sync.i32p` adaptation.

## Progress

- [x] (2026-08-28) Recorded the Slice 61 baseline: 404 names, SHA-256
  `40f3eba7cfb2602716a16b54d942cf09e34e9f2171835889a1dea43cb1e10d0a`, Release 404/404,
  wave/ABI 106/106, Debug 10/10, 528-byte x64/308-byte x86 V3 table, and 27,669 measured lines.
- [x] (2026-08-28) Audited exact UInt source specialization, the E52017 unsigned-load boundary,
  downstream type roles, provider intrinsic reuse, and NVRTC PTX.
- [x] (2026-08-28) Admitted canonical UInt load results and appended independently negotiated
  feature 47/operation 13 with an exact `Bool(UInt, UInt)` descriptor.
- [x] (2026-08-28) Added provider/direct/capability/PTX/`ptxas`/RTX evidence through the public
  source path; mixed and uniform UInt inputs agree with NVRTC.
- [x] (2026-08-28) Formatted, built, tested, hashed, measured, documented, audited, and removed the
  temporary probes.

## Surprises and Discoveries

- Observation: the public UInt probe stops at E52017 `load result type`, before exact GenericAsm
  matching. `asNVVMSupportedDeviceScalarPointerType`, `NVVMTypeUse::Value`, helper parameters,
  calls, and provider i32 values already accept canonical UInt; only the preflight load gate tests
  `isNVVMSignedI32Type` instead of the established `isNVVMInteger32Type` classification.
  Consequence: fix the load consumer's inconsistent type gate. Do not coerce UInt to Int, add an
  overload-specific bypass, or reconstruct the loaded value later.

- Observation: after the load, specialization retains exact `Func(Bool, UInt, UInt)` ending in
  `GenericAsm("_waveAllEqual($0, $1)")`. Signedness does not alter LLVM's signless i32
  representation or the bitwise CUDA match instruction.
  Consequence: append a distinct source-semantic feature/operation and exact signature row, while
  reusing the provider's native i32 match-all implementation.

- Observation: NVRTC emits the same `[64, 64]`, `ld.global.u32`, two synchronized ballots,
  `match.all.sync.b32`, and `st.global.u32` structure as the signed overload.
  Consequence: share graph scaffolding but retain separate source, type, feature, operation, and
  runtime cases.

## Decision Log

- Decision: admit canonical UInt as a scalar load result at the existing memory preflight boundary.
  Rationale: the source pointer's pointee is UInt, the load result preserves that canonical type,
  and every downstream type/value/provider contract already maps it to i32. Rejecting only this
  producer is inconsistent and forces no new representation.
  Date/author: 2026-08-28, Codex.

- Decision: append feature 47 `WAVE_MASK_ALL_EQUAL_UINT` and operation 13.
  Rationale: Int and UInt share LLVM bits but remain different Slang semantic overloads and exact
  helper signatures. Independent negotiation avoids claiming every 32-bit all-equal overload from
  one descriptor.
  Date/author: 2026-08-28, Codex.

- Decision: reuse `llvm.nvvm.match.all.sync.i32p` and its provider-private predicate extraction.
  Rationale: PTX match-all is bitwise b32, LLVM integers are signless, and signedness affects no
  native instruction operand or result. A second provider adaptation would duplicate one contract.
  Date/author: 2026-08-28, Codex.

## Outcomes and Retrospective

The unsigned source reaches its canonical exact helper after changing only the load preflight's
inconsistent signed-only classification to the established 32-bit integer classification. Exact
pointee/result identity and the downstream semantic UInt type remain intact. Feature 47 and
operation 13 independently negotiate `Bool(UInt, UInt)` while the provider reuses the signless i32
native match-all intrinsic and returns only its Bool predicate. V3 remains 528 bytes on x64 and 308
bytes on x86.

Both normal and LLVM-7-compatible assembly contain one aggregate
`llvm.nvvm.match.all.sync.i32p` call, predicate extraction, and exact declaration. NVVM and NVRTC
agree on `[64, 64]`, one global 32-bit load/store pair, two ballots, and one
`match.all.sync.b32`; CUDA 12.9 `ptxas` accepts both. On the RTX 5090, distinct UInt lane values
produce false and uniform `23` produces true through both routes.

The seven new tests pass 7/7, the complete wave/ABI matrix passes 113/113, Release passes 411/411,
and Debug preservation passes 10/10. The sorted LF-terminated name set hashes to
`bea39cafc76c97ab6cb2d31fcc12aa42f41fe9d3d4d324ca296e115cd5d4d3a4`; removing the seven Slice 62
names reproduces Slice 61's 404-name hash
`40f3eba7cfb2602716a16b54d942cf09e34e9f2171835889a1dea43cb1e10d0a`. Parameterizing the two
real-provider all-equal rows keeps measured growth to 126 lines, from 27,669 to 27,795.

The final audit found no new fallback, syntax reconstruction, custom equivalence, helper-name
matching, or duplicate provider bridge. The retained load gate fixes the sole inconsistent
consumer of a canonical UInt load shape; the exact descriptor is the semantic boundary, and the
shared provider case is the correct signless representation boundary.

## Context and Current Pipeline

Consider this kernel:

```slang
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<uint, Access::Read, AddressSpace::Device> source)
{
    uint laneIndex = WaveGetLaneIndex();
    destination[laneIndex] = WaveActiveAllEqual(source[laneIndex]) ? 1 : 0;
}
```

CUDA target selection implements the public generic operation as
`WaveMaskAllEqual(WaveGetActiveMask(), value)`. Active-mask synthesis retains the established
ballot graph, and specialization produces exact `Bool(UInt, UInt)` `_waveAllEqual($0, $1)`.

Direct preflight in `source/slang/slang-emit-nvvm.cpp` currently accepts Float loads separately and
only signed Int loads otherwise. This contradicts `source/slang/slang-emit-nvvm-type-lowering.cpp`,
where device pointers, entry parameters, helper parameters, and ordinary values intentionally
accept both Int and UInt as distinct canonical Slang types mapping to one LLVM i32 representation.
The load validation already proves pointer/result type identity, ownership, availability, and
dominance, so broadening the initial scalar classification is the missing producer contract.

After preflight, exact GenericAsm matching calls the generic intrinsic callback. The facade maps
operation to feature, and the provider validates two i32 values before calling
`llvm.nvvm.match.all.sync.i32p` and returning aggregate element 1 as Bool.

## Scope and Non-Goals

In scope are canonical UInt scalar load results, feature 47/operation 13, exact
`Bool(UInt, UInt)` descriptor selection, native match-all reuse, one public runtime row,
provider/direct/capability/PTX/`ptxas`/RTX evidence, design, ledger, and this plan.

Out of scope are UInt stores as a separately claimed public copy capability, unsigned arithmetic
or phis, Float/i64/vector/matrix all-equal overloads, new type roles or provider types, aggregate
ABI exposure, `WaveMatch`, reductions, divergence stress, new callback fields, or performance
claims.

## Architecture and Invariants

Canonical Slang UInt remains distinct from Int for exact signature matching and type relations.
`NVVMTypeLoweringContext` continues to map both to the provider's signless i32 handle. The load
producer is legal only when its exact pointer pointee matches its result type and all established
ownership/dominance checks pass.

The emitter recognizes only exact `_waveAllEqual($0, $1)` plus `Bool(UInt, UInt)`. The facade maps
operation 13 only to feature 47. The provider shares the i32 match-all case, validates both operands
before mutation, and exposes only the semantic Bool predicate.

Shared tests parameterize genuine graph dimensions. Each UInt row retains independently registered
feature/op, exact source/helper signature, PTX mnemonic, and mixed/uniform runtime assertions.

## Interfaces and Dependencies

Append feature 47, operation 13, and a minimum-size alias to V3. Extend facade, exact descriptor,
provider case selection, fake classification, predicate fixture tests, public source/runtime row,
design, ledger, and plan. Change the preflight load classification from signed i32 to canonical
integer32 and its operation diagnostic from signed-i32 to integer32. Do not change table layout,
ABI version, type-role enum, callbacks, V2, exports, LLVM components, or formats.

## Milestones

1. Admit exact UInt load results through the existing scalar-memory producer/consumer checks.
2. Append feature 47/operation 13 with unchanged V3 sizes and exact Slice 61 compatibility.
3. Match exact `Bool(UInt, UInt)` and reuse native match-all plus predicate extraction.
4. Add provider/direct/capability/PTX/`ptxas`/RTX evidence through the public source path.
5. Format/build, run focused/full/Debug lanes, hash, measure, document, audit, remove probes, and
   commit.

## Validation and Acceptance

Build the standalone Release provider and Release/Debug main targets outside the sandbox. Run the
seven new names, complete wave/ABI matrix, generic-function and intrinsic compatibility/invalid
tests, unsupported boundary, full Release NVVM prefix, and Debug 10/10 preservation.

Accept unchanged 528/308-byte V3; exact Slice 61 compatibility; canonical UInt load reaching exact
helper lowering without an Int coercion; one aggregate match-all call and predicate extraction in
both assembly forms; independent feature-47 E52016 before module construction; `[64, 64]` with one
load/store, two ballots, and one match-all; CUDA 12.9 `ptxas`; mixed UInt values returning false and
uniform values returning true on RTX through both routes; hash continuity; bounded growth;
formatted code; completed audit; removed probes; clean diffs.

## Self-Review and Input-Shape Audit

Inventory the UInt load gate, feature/operation mappings, exact descriptor/provider sharing, fake
classification, fixtures/runtime row, and evidence. Prove the load's source pointee/result relation
is canonical, provider validation precedes mutation, semantic signedness remains authoritative for
descriptor selection, and no coercion, helper-name match, syntax reconstruction, fallback, custom
equivalence, or duplicate provider bridge was added.

The unsigned load is intentional source IR produced from `Ptr<uint, Read, Device>` and is already
accepted by the common pointer and value type contracts. Fixing its sole inconsistent consumer is a
producer-side capability completion, not a downstream patch. The exact UInt helper is likewise
canonical target-selected input. LLVM's signless i32 and PTX b32 semantics make provider reuse the
correct representation boundary while Slang retains the overload distinction upstream.

## Failure and Recovery

If admitting UInt loads exposes arithmetic, phi, store, or ABI shapes beyond this graph, stop and
audit them independently rather than broadening more gates. If final IR differs from the exact
helper, stop rather than match an alternative spelling. Removing the load admission,
feature 47/operation 13, and Slice 62 evidence restores Slice 61. Never stage
`external/slang-binaries/` or `tmp-slice-62-*` artifacts.

## Artifacts and Hand-Off

Retain the exact load/helper trace, normal/compatible LLVM assembly, NVVM/NVRTC PTX,
`ptxas`/RTX results, sizes, hashes, line growth, and audit. Distill durable evidence into the
design/ledger and commit this completed plan with Slice 62.
