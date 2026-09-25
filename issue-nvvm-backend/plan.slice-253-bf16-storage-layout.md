# Qualify CUDA BF16 storage layout producers

This ExecPlan follows `.agent/PLANS.md`. The NVVM maintainer requires completed plans and reports
in each local slice commit; raw controls and logs remain under ignored `build/`.

## Purpose and Observable Result

Establish the concrete user-visible consequence of the BF4 CUDA layout mismatch recorded by
research240, then identify the smallest principled producer repair. Compare public reflection,
canonical IR layout and actual emitted CUDA storage for BF2/BF3/BF4 and neighboring controls.
This is research only; no compiler support, ABI or production source changes are authorized in253.

## Progress

- [x] 2026-09-25: Accepted and locally committed252 as
      `9a95c4e80099636ebf8d4af52a1dead134c4ce79`; clean checkout and current environment verified.
- [x] 2026-09-25: Read workflow, plan standard, STATUS, research240 and its durable vector contract.
- [x] 2026-09-25: Declare the bounded research and local execution fallback before controls.
- [x] 2026-09-25: All119 sources/12 artifacts/563 inputs exact252; small runtime4 passed.
- [x] 2026-09-25:36 actual CUDA/AST layout rows and72 IR rows measured; wrapped-pointer and
      StructuredBuffer controls prove reflection-packed BF4 has17/18 wrong fields. Both direct CUDA
      query modes execute with five wrong results; the saved pre252 compiler emits identical PTX.
- [x] 2026-09-25: Traced canonical AST format loss and IR CUDA rule; Natural sizeof is a separate
      intentional policy. Future repair is CUDA layout producers only, without BF storage admission.
- [x] 2026-09-25: Separate hand-derived audit verifies all layouts, outputs, before252 PTX and
      unchanged identities. Completed report/design/STATUS and compact research evidence for local acceptance.

## Surprises and Discoveries

The fresh-worker spawn failed with `agent thread limit reached`. WORKFLOW explicitly permits local
execution when delegation is unavailable. The parent is the only writer and performs the same
bounded plan, implementation-free research, separate oracle audit and acceptance locally. No fresh
independent agent review is claimed.

The initial NVCC host-executable probe failed in unrelated BF16 math overloads; device-only
SM80 compilation of the actual prelude succeeds. The first runtime oracle incorrectly treated
unqualified sizeof/alignof as CUDA ABI queries. Its BF2 metadata assertion failed; all actual field
reads were exact. That attempt is retained. The corrected protocol checks documented Natural
semantics separately, leaves shader/input/logical field oracles unchanged, and writes fresh paths.
Explicit CUDA __sizeOf/__alignOf queries then expose the actual direct-backend mismatch.

## Decision Log

- 2026-09-25, parent: Prioritize the known BF4 producer mismatch over new FP8 runtime conversions
  (unsettled overflow policy), texture queries (248 contract/toolkit limitations), or another
  compile-time optimization (requires fresh profiling after252). Research240 already qualifies
  BF register conversions;253 will not repeat that entire domain.
- 2026-09-25, parent: Compare actual prelude packing and reflection-derived packing as distinct
  input controls with independently expected logical values. Never change an existing corpus input
  or oracle to hide a mismatch. A direct preflight stop is not a runtime pass.

## Outcomes and Retrospective

The research proves one existing CUDA producer mismatch through public reflection, canonical IR,
actual CUDA ABI, typed record reads and explicit queries. The direct query control is runnable
without new BF16 storage support and returns five wrong values in both direct modes. Pre252 PTX
is identical. All119 source/12 artifact/563 input identities remain exact252. Separate oracle
audit verifies all final4096 words and retained preparation outputs; fresh delegation was unavailable.
Local acceptance verifies318 indexed artifacts and83 compact references, including an independent
formula for all36 Natural layout rows. Accepted full252/targeted233 and cadence0 remain authoritative.
The next bounded implementation
repairs the AST/IR CUDA layout producers, with full shared-layout preservation coverage.

## Context and Current Pipeline

Consider `struct Wrapped { uint16_t prefix; vector<BFloat16,4> value; uint16_t suffix; };` stored in
a structured buffer. Research240 measured actual CUDA BF4 size8/alignment2, while both layout
producers reported alignment8. `_createTypeLayout` preserves the canonical BF16 scalar type but
passes BaseType::Void to `CUDALayoutRulesImpl::GetVectorLayout` for its vector element. The generic
rule gives width4 alignment8. IR `CUDALayoutRules::calcSizeAndAlignment` special-cases Half, not BF16.
The actual prelude's BF3/BF4 structs contain scalar BF16 components; native BF2 has alignment4.
The research must prove how these facts reach reflection, buffer stride or byte addressing.

## Scope and Non-Goals

New isolated controls, source/IR traces, runtime proof and documentation only. Cover widths2/3/4,
wrapped fields, arrays/stride and scalar/Half/ordinary-vector neighbors. No production/API/ABI,
fixture/oracle/manifest, material, optimizer or new NVVM storage admission changes. Do not solve
the independent FP8 aggregate or dynamic-dispatch boundaries. Material runtime semantics are absent.

## Architecture and Invariants

Canonical BF16 remains distinct; physical storage must match the CUDA ABI at its responsible layout
producer. BF2 is4/4, BF3 is6/2 and BF4 is8/2 in the qualified prelude. LLVM value vectors do not
automatically supply those storage layouts. Reuse240's register/transport contract by reference.
Reject downstream offset patches, relaxed aggregate checks or a second semantic representation.

## Interfaces and Dependencies

Native Ubuntu24.04, L4SM89 driver580.126.09, CUDA12.9.2/NVRTC12.9.86, targetSM80, isolatedLLVM14.0.6,
providerABI41 and matching RelWithDebInfo. Inspect/source `build/nvvm-loop/slice-203-env.sh`; the
local slang-build skill was already read for this loop. Use native tools. Compiler library is
`10ffeb3246d56c9a835b1cd606e35a1a2cd6c8f9fcb3f6bfef36fed26413ea7b`; provider is
`5fe0b977e22b80acc5ee39147c69510a01c09563354a1a67bd9573d1cda1aeab`.

## Milestones

1. Snapshot all252 source/artifact/input identities and any reused240/226 harness dependencies.
   Run `python3 extras/validate-nvvm-runtime.py --config RelWithDebInfo --cuda-path /usr/local/cuda-12.9
--architecture 80 --output build/nvvm-loop/slice-253-bf16-storage-layout/runtime` under30minutes.
2. Inspect related tests; generate isolated source controls and independent byte-layout expectations.
   Compile actual CUDA prelude size/alignment/offsetof assertions, public reflection and emitted IR.
3. Run faithful NVRTC controls and explicitly record matching direct-mode support or rejections.
   Check every returned word and sentinel. Use finite focused physical controls only if needed to
   discriminate storage representations; do not repeat already-qualified conversion semantics.
4. Record exact producer-to-consumer trace, observable disagreement and future repair/test scope.
5. Verify unchanged252 identities, inherit full252 honestly, close evidence and complete five-part
   report, compact semantic evidence, design and STATUS. Parent accepts and locally commits.

## Validation and Acceptance

Require runtime4, exact requested control inventories with no omissions/duplicates, independent
layout/output oracles and actual executed counts. Compile/assembly/reflection are separate evidence
from GPU execution. All119 source,12 artifact and563 runtime inputs must remain exact252; unchanged
NatVis is the120th source snapshot when included. Full252's1701 outcomes/1662correct/39unresolved
and18resolved histories, unit/semantic/toolkit/complex gates remain inherited, not fresh253.
No full checkpoint, host compiler rebuild or cadence reset for unchanged-production research.
Max4 CPU workers total, sequential GPU suites,30minute bounds. Source/hash closure and diff check
are required. Separate oracle derivation and final evidence audit compensate for unavailable fresh
delegation without claiming a second reviewer.

## Failure and Recovery

Retain failed preparation/probes in distinct paths and diagnose before retrying. Do not modify old
accepted evidence. If the presumed mismatch lacks a real consumer consequence, report that result;
no production patch follows merely from a changed diagnostic. Stop GPU dispatches on device loss;
no system/driver change, reboot, push or publication. Stop investigating independent issues once
this bounded handoff is recorded.

## Artifacts and Hand-Off

Raw root `build/nvvm-loop/slice-253-bf16-storage-layout`; completed plan and
`report.slice-253-bf16-storage-layout.md`, compact semantic evidence, design and STATUS are durable.
Retain source snapshots, commands, outputs, layout records, failures and a closed raw hash index.
State fresh/inherited evidence and the exact next bounded action.
