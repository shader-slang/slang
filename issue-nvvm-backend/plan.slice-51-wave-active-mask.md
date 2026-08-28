# Slice 51: Produce the active wave mask with a synchronized ballot

This ExecPlan follows `.agent/PLANS.md`. Keep it current. The user requires the completed plan to
ship with the slice, overriding the repository's default working-log policy for this experiment.

## Purpose and Observable Result

After this slice, the direct NVVM route compiles and runs a CUDA kernel that stores
`WaveGetActiveMask()` for every lane in one full warp. The canonical synthesized
`waveMaskBallot(0xffffffff, true)` operation lowers through a separately negotiated provider
capability to `llvm.nvvm.vote.ballot.sync`, and NVVM agrees with NVRTC at PTX and runtime layers.

## Progress

- [x] (2026-08-28) Recorded the Slice 50 baseline: 333 names, SHA-256
  `57f52bd80e15eefb8a35bc51821d99a4b70c858f111535fde1fea3f90b2bb367`, Release 333/333,
  Debug 10/10, 528-byte x64/308-byte x86 V3 table, and 25,120 measured lines.
- [x] (2026-08-28) Audited the exact final linked IR producer, the established Bool/i32 value
  paths, and the identical LLVM 7/14 synchronized-ballot declaration.
- [x] (2026-08-28) Added feature 39/operation 5, direct ballot validation/emission, exact
  provider mapping, and canonical i32/i1 literal transport through existing callbacks.
- [x] (2026-08-28) Added seven provider/direct/capability/PTX/assembler/runtime evidence names and
  preserved the complete 42-test Slice 46-51 wave matrix.
- [x] (2026-08-28) Formatted, built, tested, hashed, measured, documented, and audited the
  completed slice.

## Surprises and Discoveries

- Observation: CUDA selection initially produces `waveGetActiveMask`, but the existing active-mask
  synthesis pass replaces it with `waveMaskBallot(0xffffffff, true)` and threads the result into
  the `WaveGetActiveMask(UInt)` identity helper.
  Consequence: support the canonical final ballot instruction directly. Do not match
  `__activemask()` text, helper names, or the earlier pre-synthesis operation.

- Observation: LLVM 7 and LLVM 14 define `llvm.nvvm.vote.ballot.sync` identically as
  `i32(i32, i1)` with inaccessible-memory and convergent semantics.
  Consequence: reuse the generic intrinsic callback with exact mixed argument types and extend the
  legacy declaration audit; no new callback field, inline assembly, or text rewrite is needed.

- Observation: active-mask synthesis appended a UInt entry-block parameter and matching argument
  at every call to an ordinary helper but left the helper value typed as its old zero-parameter
  `Func(UInt)`.
  Consequence: repair the producer with established `fixUpFuncType()` after the function transform.
  Keep the direct emitter's exact call arity/type validation instead of accepting the mismatch.

- Observation: the generic integer-constant provider used signed-width validation, for which i1
  `1` is outside the signed range even though it is canonical LLVM `true`.
  Consequence: accept precisely i1 value 1 in addition to the established signed-width contract;
  scope UInt literal admission to wave masks so unrelated unsigned-offset policy does not widen.

## Decision Log

- Decision: make synchronized wave-mask ballot the next bounded primitive.
  Rationale: it is the canonical producer for `WaveGetActiveMask()` after existing compiler
  synthesis, and active masks are a dependency of unmasked CUDA wave wrappers. Supporting the
  final IR shape is both smaller and more reusable than bypassing the synthesis pass.
  Date/author: 2026-08-28, Codex.

- Decision: append one feature and one intrinsic operation to the existing V3 suffix.
  Rationale: providers predating this slice must remain loadable and honestly advertise no ballot
  support, while the existing operation-defined callback already transports heterogeneous scalar
  handles.
  Date/author: 2026-08-28, Codex.

- Decision: repair synthesized helper types at the producer.
  Rationale: the added block parameter and call argument are canonical and intentional, while the
  stale function type is accidental. `fixUpFuncType()` is the existing single source of truth for
  rebuilding a function type from its actual parameters; an NVVM call exception would preserve
  malformed IR and weaken every downstream consumer.
  Date/author: 2026-08-28, Codex.

## Outcomes and Retrospective

The direct route now lowers the canonical synthesized ballot through feature 39/operation 5 and
the unchanged generic callback. Provider emission validates `(i32, i1)`, emits native
`llvm.nvvm.vote.ballot.sync`, and the legacy writer verifies its exact LLVM-7-compatible
declaration. Active-mask synthesis now repairs each transformed function's declared type from its
entry parameters, eliminating the producer inconsistency revealed by strict direct-call validation.

The standalone Release provider and Release/Debug main targets build successfully. The seven new
names pass 7/7, the complete Slice 46-51 wave matrix passes 42/42, intrinsic compatibility plus the
unsupported boundary passes 2/2, Release passes 340/340, and Debug preservation passes 10/10. The
sorted LF-terminated name set hashes to
`7abb718be35a0e9ad61202e3c8776c718f22c43a790c22df960140164cf5ce2b`; removing exactly the
seven Slice 51 names yields 333 names and the Slice 50 hash
`57f52bd80e15eefb8a35bc51821d99a4b70c858f111535fde1fea3f90b2bb367`. V3 remains 528/308
bytes. Seven evidence names add 392 measured physical lines across the five test/support files,
from 25,120 to 25,512. NVVM/NVRTC, CUDA 12.9 `ptxas`, and RTX 5090 runtime evidence all pass;
every lane in one full warp stores `0xffffffff` through both routes.

## Context and Current Pipeline

Consider this kernel:

```slang
[CUDAKernel]
void computeMain(uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> destination)
{
    uint laneIndex = WaveGetLaneIndex();
    destination[laneIndex] = WaveGetActiveMask();
}
```

CUDA target selection calls the intrinsic `__WaveGetActiveMask()`. During active-mask synthesis,
that operation becomes `waveMaskBallot(0xffffffff, true)` and the mask is passed to a canonical
identity helper. The final entry-point graph therefore contains an i32 mask constant, an i1 true
constant, the ballot result, lane-index pointer offset, and UInt store. Direct NVVM already owns
all surrounding value, pointer, call, and store shapes; only `kIROp_WaveMaskBallot` is rejected by
preflight.

The provider's operation callback maps stable facade operations to LLVM intrinsics after validating
argument count, exact types, ownership, availability, and insertion position. Its legacy assembly
writer validates every semantic NVVM declaration before LLVM 14 text is sent to LLVM 7-based
libNVVM.

## Scope and Non-Goals

In scope are one synchronized ballot feature/operation, exact `(i32, i1) -> i32` provider
construction, direct validation and lowering for the canonical `waveMaskBallot` shape, fake and
real fixtures, and the established seven evidence layers.

Out of scope are arbitrary predicate-producing source programs, ballot masks wider than UInt,
vote all/any/equal, divergent-control-flow stress, read-first or unmasked shuffle support,
active-mask synthesis algorithm changes beyond repairing its produced function type, new callback
fields, performance claims, and other wave operations.

## Architecture and Invariants

The active-mask synthesis pass remains the sole producer of the final ballot graph and repairs the
transformed helper's function type from its canonical entry parameters. Direct NVVM
admits `kIROp_WaveMaskBallot` only with UInt result/mask and Bool predicate, then requires the new
feature and lowers its two existing operands through the generic scalar value map.

The facade maps the stable ballot operation to exactly its feature. The provider requires exactly
`(i32, i1)`, performs every check before module mutation, and emits
`llvm.nvvm.vote.ballot.sync(mask, predicate)`. The legacy writer requires the exact declaration,
argument vector, result, and semantic attributes accepted by LLVM 7.

## Interfaces and Dependencies

Append feature 39 and intrinsic operation 5 plus a minimum-size alias to V3. Extend facade,
provider, emitter, fake, fixtures, tests, design, ledger, and this plan. Do not change the table
layout, ABI version, V2, exported symbols, LLVM components, or serialization format.

## Milestones

1. Append feature 39/operation 5 with exact Slice 50 compatibility and unchanged V3 sizes.
2. Validate and lower canonical `kIROp_WaveMaskBallot(UInt mask, Bool predicate) -> UInt`.
3. Emit and legacy-audit native `nvvm_vote_ballot_sync` through the existing callback.
4. Add provider/direct/capability/PTX/`ptxas`/RTX evidence for a full-warp active mask.
5. Format/build, run focused/full/Debug lanes, hash, measure, document, audit, and commit.

## Validation and Acceptance

Build the standalone Release provider and Release/Debug main targets outside the sandbox. Run the
seven new names, Slice 46-50 preservation names, generic-intrinsic compatibility/invalid tests,
the unsupported matrix, full Release NVVM prefix, and Debug 10/10 preservation.

Accept unchanged 528/308-byte V3 tables; exact Slice 50 provider compatibility; provider text with
one `llvm.nvvm.vote.ballot.sync(i32, i1)` call and its exact declaration; direct/fake operand
topology; matching NVVM/NVRTC PTX ABI and `vote.sync.ballot.b32`; CUDA 12.9 `ptxas` acceptance;
`0xffffffff` in every lane of a full RTX 5090 warp through both routes; name/hash continuity;
formatted code; a completed input-shape audit; and clean diff checks.

## Self-Review and Input-Shape Audit

Inventory the new feature/operation mappings, direct ballot case, provider type vector and legacy
declaration audit, fake result classification, fixtures, and evidence. For each, prove the handled
shape is canonical, checks precede mutation, existing semantic values remain the source of truth,
and no name matching, graph rediscovery, syntax reconstruction, fallback, or custom equivalence was
added.

The ballot shape reaching direct NVVM is intentionally canonical: active-mask synthesis produces
`waveMaskBallot(0xffffffff, true)` from `WaveGetActiveMask()` and passes the resulting UInt through
the existing identity helper. Its added helper parameter and call argument are also intentional,
but the old zero-parameter function type was an accidental stale spelling. Repairing it with
`fixUpFuncType()` at the producer makes the definition and call identical through the established
canonical builder path. The direct emitter consumes the final ballot at its normal target-lowering
boundary without name matching, bypassing synthesis, or weakening call validation.

The UInt all-ones mask and Bool true are canonical literals made by the synthesis producer. Direct
preflight admits the UInt literal only in the wave-mask role and preserves the prior unsupported
general UInt-offset case. Existing generic constant lowering reuses each literal's semantic type;
the provider's only contract extension is canonical i1 value 1, while signed i32 range checks remain
unchanged. No syntax or shadow bit representation is reconstructed.

## Failure and Recovery

If final linked IR differs, audit the producer and update the plan before implementation rather
than adding alternate spellings. If LLVM 14's generated declaration is not accepted by LLVM 7 or
libNVVM, isolate the exact signature/attribute mismatch before any rewrite. Removing feature 39,
operation 5, and the ballot emitter case restores Slice 50. Never stage
`external/slang-binaries/`.

## Artifacts and Hand-Off

Retain the exact final ballot IR, provider LLVM/NVVM declaration and call, PTX mechanism,
`ptxas`/RTX results, sizes, hashes, line growth, and completed audit. Distill durable architecture
and evidence into the design and capability ledger, then ship this completed plan with Slice 51.
