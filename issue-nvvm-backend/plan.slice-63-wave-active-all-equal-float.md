# Slice 63: Add public Float wave-active-all-equal

This ExecPlan follows `.agent/PLANS.md`. Keep it current. The user requires the completed plan to
ship with the slice, overriding the repository's default working-log policy for this experiment.

## Purpose and Observable Result

After this slice, direct NVVM compiles and runs public `WaveActiveAllEqual(float)`. The exact
`WaveMaskAllEqual(activeMask, floatValue)` helper remains semantically Float through Slang and the
facade, while the provider adapts the validated payload to PTX's native b32 match representation.

## Progress

- [x] (2026-08-28) Recorded the Slice 62 baseline: 411 names, SHA-256
  `bea39cafc76c97ab6cb2d31fcc12aa42f41fe9d3d4d324ca296e115cd5d4d3a4`, Release 411/411,
  wave/ABI 113/113, Debug 10/10, 528-byte x64/308-byte x86 V3 table, and 27,795 measured lines.
- [x] (2026-08-28) Audited exact Float source specialization, provider LLVM 14/LLVM 7 intrinsic
  contracts, the native representation boundary, and NVRTC PTX.
- [x] (2026-08-28) Appended independently negotiated feature 48/operation 14 and the
  provider-private Float-to-i32 bitcast.
- [x] (2026-08-28) Added provider/direct/capability/PTX/`ptxas`/RTX evidence through the public
  source path; mixed and uniform Float inputs agree with NVRTC.
- [x] (2026-08-28) Formatted, built, tested, hashed, measured, documented, audited, and removed the
  temporary probes.

## Surprises and Discoveries

- Observation: final specialization retains exact `Func(Bool, UInt, Float)` ending in
  `GenericAsm("_waveAllEqual($0, $1)")`; direct preflight stops only because no exact Float
  descriptor exists.
  Consequence: add one exact source-semantic descriptor. Do not match a helper name, erase Float
  in the emitter, or infer an overload from the common assembly text.

- Observation: LLVM 7 and the provider LLVM expose `nvvm_match_all_sync_i32p` and i64p, but no
  floating match intrinsic. CUDA's template accepts Float, and NVRTC emits a 32-bit load followed
  directly by `match.all.sync.b32`.
  Consequence: validate the facade's Float operand, bitcast it to signless i32 inside the provider,
  and reuse the audited native i32 aggregate call and predicate extraction.

- Observation: the predicate fixture's Boolean-vs-integer flag cannot represent Float, while its
  graph genuinely varies by payload type.
  Consequence: replace the flag with an explicit Boolean/Integer/Float test payload kind and share
  the real-provider graph verifier across all three all-equal rows.

## Decision Log

- Decision: append feature 48 `WAVE_MASK_ALL_EQUAL_FLOAT` and operation 14.
  Rationale: Float is a distinct Slang overload and facade contract even though PTX consumes its
  bit pattern through b32. Independent negotiation avoids claiming Float from either integer row.
  Date/author: 2026-08-28, Codex.

- Decision: perform one provider-private `bitcast float to i32` only after full argument and
  insertion-point validation.
  Rationale: LLVM bitcast preserves all 32 payload bits, matches NVRTC/PTX b32 behavior, and keeps
  the representation adaptation next to the native intrinsic rather than falsifying Slang type
  semantics upstream.
  Date/author: 2026-08-28, Codex.

- Decision: reuse the existing native aggregate signature and legacy declaration audit unchanged.
  Rationale: the intrinsic remains exactly `i32p(i32, i32) -> {i32, i1}` in LLVM 7 and 14; the
  ordinary bitcast requires neither a text rewrite nor another ABI-visible callback.
  Date/author: 2026-08-28, Codex.

## Outcomes and Retrospective

The exact `Bool(UInt, Float)` helper remains Float through the emitter, type lowering, facade, and
provider validation. Only after validation does the provider bitcast the payload to i32 for the
native b32 match call, then return its Bool predicate. Both normal and LLVM-7-compatible assembly
contain exactly one Float bitcast, aggregate `llvm.nvvm.match.all.sync.i32p` call, predicate
extraction, and exact declaration. V3 remains 528 bytes on x64 and 308 bytes on x86.

NVVM and NVRTC agree on `[64, 64]`, one global 32-bit load/store pair, two ballots, and one
`match.all.sync.b32`; CUDA 12.9 `ptxas` accepts both. On the RTX 5090, distinct ordinary Float lane
values produce false and uniform `3.25` produces true through both routes. This evidence does not
claim policy for NaNs or differently encoded equal values.

The seven new tests pass 7/7, the complete wave/ABI matrix passes 120/120, Release passes 418/418,
and Debug preservation passes 10/10. The sorted LF-terminated name set hashes to
`33720ee2997610b2d1823858e1e80641d44efce3d6b09b37d0271c70ec54c929`; removing the seven Slice 63
names reproduces Slice 62's 411-name hash
`bea39cafc76c97ab6cb2d31fcc12aa42f41fe9d3d4d324ca296e115cd5d4d3a4`. Explicit payload-kind
selection and shared all-equal assembly verification keep measured growth to 155 lines, from
27,795 to 27,950.

The final audit found no fallback, syntax reconstruction, helper-name matching, custom
equivalence, emitter coercion, public bitcast callback, or duplicate native bridge. The provider
bitcast survives because the exact Float helper is canonical and the native intrinsic boundary
owns its b32 representation. All arguments are validated before mutation, and the test fixture
refactor models a real payload-type dimension rather than hiding Float behind another flag.

## Context and Current Pipeline

Consider this kernel:

```slang
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<float, Access::Read, AddressSpace::Device> source)
{
    uint laneIndex = WaveGetLaneIndex();
    destination[laneIndex] = WaveActiveAllEqual(source[laneIndex]) ? 1 : 0;
}
```

CUDA target selection implements the public generic operation as
`WaveMaskAllEqual(WaveGetActiveMask(), value)`. Active-mask synthesis retains the established
ballot graph, and specialization produces exact `Bool(UInt, Float)` `_waveAllEqual($0, $1)`.
Float loads, values, helper parameters, entry pointers, calls, and stores already use the
established Float type role and provider f32 handle.

The generic intrinsic facade must preserve the Float handle to the provider. After validating the
module, insertion point, operation arity, i32 mask, f32 payload, ownership, and availability, the
provider bitcasts only that payload to i32 and calls the existing
`llvm.nvvm.match.all.sync.i32p`. It extracts aggregate element 1 as the semantic Bool result. The
native matching mask and integer bit-pattern value do not cross the provider ABI.

## Scope and Non-Goals

In scope are feature 48/operation 14, exact `Bool(UInt, Float)` descriptor selection, one
provider-private Float-to-i32 bitcast, native match-all reuse, explicit test payload kinds, one
public runtime row, provider/direct/capability/PTX/`ptxas`/RTX evidence, design, ledger, and this
plan.

Out of scope are Float arithmetic or comparison changes, Float bitcast callbacks in the facade,
half/double/i64/vector/matrix all-equal overloads, NaN or signed-zero language-policy claims,
`WaveMatch`, reductions, divergence stress, new type roles, aggregate ABI exposure, new callback
fields, or performance claims.

## Architecture and Invariants

Canonical Slang Float remains Float through exact signature matching, type lowering, facade
arguments, and provider validation. No emitter-side coercion or second semantic representation is
created. The provider adapts to the native intrinsic only after validating every input, so invalid
calls leave the module unmodified.

The emitter recognizes only exact `_waveAllEqual($0, $1)` plus `Bool(UInt, Float)`. The facade maps
operation 14 only to feature 48. The provider accepts i32/f32, bitcasts the payload to i32, calls the
same audited aggregate intrinsic, and exposes only the Bool predicate.

Shared tests parameterize genuine payload-type and graph dimensions. Each Float row retains its
independently registered feature/op, exact source/helper signature, bitcast assertion, PTX
mnemonic, and mixed/uniform runtime assertions.

## Interfaces and Dependencies

Append feature 48, operation 14, and a minimum-size alias to V3. Extend facade, exact descriptor,
provider case selection/adaptation, fake classification, predicate fixture tests, public
source/runtime row, design, ledger, and plan. Do not change table layout, ABI version, type-role
enum, callbacks, V2, exports, LLVM components, formats, or the legacy text rewrite set.

## Milestones

1. Append feature 48/operation 14 with unchanged V3 sizes and exact Slice 62 compatibility.
2. Match exact `Bool(UInt, Float)` while preserving the Float facade handle.
3. Validate i32/f32, bitcast f32 to i32, and reuse native match-all plus predicate extraction.
4. Generalize predicate test payload selection and add all seven Float evidence layers.
5. Format/build, run focused/full/Debug lanes, hash, measure, document, audit, remove probes, and
   commit.

## Validation and Acceptance

Build the standalone Release provider and Release/Debug main targets outside the sandbox. Run the
seven new names, complete wave/ABI matrix, generic-function and intrinsic compatibility/invalid
tests, unsupported boundary, full Release NVVM prefix, and Debug 10/10 preservation.

Accept unchanged 528/308-byte V3; exact Slice 62 compatibility; exact Float helper lowering with
no emitter coercion; one Float-to-i32 bitcast, aggregate match-all call, and predicate extraction
in both assembly forms; independent feature-48 E52016 before module construction; `[64, 64]` with
one load/store, two ballots, and one match-all; CUDA 12.9 `ptxas`; mixed Float values returning
false and uniform values returning true on RTX through both routes; hash continuity; bounded
growth; formatted code; completed audit; removed probes; clean diffs.

## Self-Review and Input-Shape Audit

Inventory feature/operation mappings, exact descriptor, provider bitcast, fake classification,
fixture payload-kind refactor, runtime row, and evidence. Prove the Float source/helper shape is
canonical, provider validation precedes mutation, semantic Float remains authoritative upstream,
and no helper-name match, syntax reconstruction, fallback, custom equivalence, public bitcast
callback, or duplicate native intrinsic bridge was added.

The Float helper is intentional target-selected IR. Its Float operand is the source of truth until
the provider reaches an intrinsic whose documented LLVM contract accepts only the payload's b32
representation. Adapting at that boundary mirrors NVRTC and avoids changing the producer. The
test-only explicit payload kind replaces a Boolean flag that no longer modeled the fixture's real
dimension; it does not merge operation-specific assertions.

## Failure and Recovery

If final IR differs from the exact helper, stop rather than match an alternative spelling. If the
bitcast changes legacy serialization beyond an ordinary LLVM instruction, audit the producer
module and LLVM 7 reader rather than adding a broad text rewrite. If runtime behavior exposes a
language-level Float equality distinction, stop before claiming a policy from one probe. Removing
feature 48/operation 14 and Slice 63 evidence restores Slice 62. Never stage
`external/slang-binaries/` or `tmp-slice-63-*` artifacts.

## Artifacts and Hand-Off

Retain the exact helper trace, normal/compatible LLVM assembly, NVVM/NVRTC PTX,
`ptxas`/RTX results, sizes, hashes, line growth, and audit. Distill durable evidence into the
design/ledger and commit this completed plan with Slice 63.
