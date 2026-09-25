# Isolate unsigned firstbithigh behavior

This ExecPlan follows `.agent/PLANS.md` and the NVVM completed-plan commit exception. Fresh-context
workers remain unavailable at the app's agent-thread limit; the parent uses WORKFLOW's local fallback.

## Purpose and Observable Result

Determine whether the bit31-dependent CUDA firstbithigh observation from the slice 221 fixture is a
compiler defect, which types/modes are affected, and the responsible layer. Use runtime-loaded words
and independent integer bit-length expectations, preserving signed negative semantics separately.
Research only: no production/test manifest/source-contract changes or support claim.

## Progress

- [x] 2026-09-25: Select on clean accepted221 base `c10071446b77cb53a47a1518c839422bac024f45`.
- [x] Read standard-module contract, CUDA32/64 helpers and existing frozen tests.
- [x] Run small GPU smoke, dynamic scalar/vector32/64 probe in all three modes, assemble PTX.
- [x] Compare exact words, confirm prior-source presence, identify responsible layer and handoff.
- [x] Verify unchanged accepted artifacts/inputs, complete report/evidence/STATUS and local commit.

## Surprises and Discoveries

`hlsl.meta.slang` explicitly distinguishes unsigned highest1 from signed-negative highest0.
CUDA `U32_firstbithigh` casts to int32 and complements negative words, while I32 delegates to it.
U64 uses the unsigned word directly; I64 complements only signed-negative input before delegation.

## Decision Log

2026-09-25: Prioritize the observed correctness issue over independent unsupported features.
Use research before editing a shared prelude because a later fix requires full-corpus validation.
HLSL reference agrees with the local contract: negative signed inputs search for a zero bit;
unsigned overloads are separate. https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/firstbithigh
Do not infer direct NVVM behavior from CUDA source behavior; measure both.

## Outcomes and Retrospective

Accepted research on 2026-09-25: NVRTC has 24 wrong unsigned32 scalar/vector words; both NVVM
modes and all signed32/64 and unsigned64 cases pass. No production or registered corpus changes.

## Context and Current Pipeline

Standard `firstbithigh<T>` selects `$P_firstbithigh($0)` with nvvmFirstBitHigh semantic metadata.
CUDA source emission chooses U32/I32/U64/I64 helpers in `prelude/slang-cuda-prelude.h`; vector overloads
map each component to the scalar intrinsic. Direct NVVM instead resolves FIRST_BIT_HIGH by typed
descriptor. The input shape is canonical integer scalar/vector; source helper signedness is the
candidate responsible boundary, not front-end syntax or representation.

## Scope and Non-Goals

Probe uint/int/uint64/int64 scalar and two-component vector values, plus input/output preservation.
No production edits, new registered fixture, provider/ABI/library change, broad backend sweep,
bit-index fix, other intrinsic investigation or material execution.

## Architecture and Invariants

For unsigned N-bit word v, expected index is v.bit_length()-1 (zero maps to0xffffffff). For signed
negative input represented by v, complement within N bits before the same calculation. Vector
components independently use the corresponding scalar expectation. Runtime loads prevent constant
folding; raw uint32 outputs retain all-ones sentinel exactly.

## Interfaces and Dependencies

Existing optimized221 compiler55bd12f280ee51def87219c767557e198cbd07b9b99f06d118a20209dfb46598;
providerABI36 ae0e7859f6062a69f191cace4ae8d96340c074815866808c5b43467f41144372.
Native Ubuntu L4 SM89 target80 CUDA12.9.2/NVRTC12.9.86 LLVM14. Source env203; no build.

## Milestones and Validation

1. Small runtime gate4/4. Generate a dynamic Ptr<int> CUDAKernel and Python driver using existing
   CUDA-driver probe mechanism. Exercise powers-of-two and adjacent values at every32/64bit position,
   zero, all-ones, sign boundaries and alternating bits in batches of32 lanes.
2. Compile/assemble NVRTC O3 and NVVM O0/O3, execute all batches; independently derive expected indices
   with integer bit_length and signed complement. Record every actual/expected/input word and command.
3. Verify accepted221 source/artifact and 549 runtime-input hashes unchanged. Record supported failures
   separately from51 registered failures. Inspect previous committed prelude to establish predating221;
   do not claim an introducing revision. Preserve full 220/cadence 1 with research only.

## Failure and Recovery

Stop on GPU loss without driver changes/reboot; bounded commands. Classify compile rejections and
runtime mismatches separately. New observed wrong output gets concrete reproduction/trace, no oracle
weakening. No push. Do not change the prelude until research is accepted and a fix plan defines full
checkpoint obligations.

## Artifacts and Hand-Off

Raw `build/nvvm-loop/slice-222-semantics`: probe/source/PTX/cubins/commands/results/smoke/provenance.
Durable semantic-evidence, completed plan and five-part report, STATUS. Select next bounded action
from measured results; known unsupported diagnostics alone are not correctness evidence.

2026-09-25 outcome: 96 unique 32-bit/192 unique 64-bit inputs,18 GPU executions, 6912 checked words;
NVRTC 2280/2304 words match with 24 unsigned32 mismatches, NVVM 2304/2304 each. Three PTX assemblies
and runtime smoke 4/4 pass. All 23 tested sources, 12 artifacts and 549 runtime inputs match221. Prelude is
byte-identical to full 220, establishing pre-existing behavior without an introducing-revision bisect.
Full220/cadence 1 and registered 1659 cells1608 correct51 failures, 6 resolved histories remain unchanged.
Next 223: move CUDA32 signed complement into I32 helper, add dynamic regression and require full gates.
