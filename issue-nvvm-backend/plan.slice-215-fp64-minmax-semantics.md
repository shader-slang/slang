# Audit masked floating-point min/max before FP64 admission

This ExecPlan follows `.agent/PLANS.md`. The NVVM workflow requires this completed research plan
and report to be committed by the parent; generated probes and raw logs remain ignored.

## Purpose and Observable Result

Determine the exact CUDA masked min/max algorithm, compare an independent raw-word oracle with
GPU output, and propose the smallest responsible next slice. No production implementation.

## Progress

- [x] 2026-09-24: Read workflow, plans standard, build skill, accepted 208/209/213/214 evidence.
- [x] 2026-09-24: Confirm native Linux, clean base `357d270587d132e1de92060b51e83f6a9be062b2`.
- [x] 2026-09-24: Verified all 15 accepted 214 source, 12 artifact and 546 runtime-source hashes; GPU smoke 4/4.
- [x] 2026-09-24: Traced scalar/aggregate source algorithm and ran early singleton FP32 gate: 12 executions, 8 correct / 4 mismatches (24 result words, 16 equal / 8 unequal).
- [x] 2026-09-24: Applied predeclared supported-defect stop. Broader FP64/aggregate/order matrix intentionally NOT executed.
- [x] 2026-09-24: Recorded five-part report, compact evidence and STATUS; parent owns acceptance and commit after handoff.

## Surprises and Discoveries

CUDA only butterflies low-bit contiguous power-of-two masks; high aligned subgroups scan.
Both scalar and aggregate irregular paths seed from the caller and scan original values.
Measured supported FP32 singleton qNaN/sNaN reductions return +/-infinity at NVVM O0/O3 instead
of preserving the input. NVRTC preserves both words. PTX attributes this to an injected infinity
seed and numeric min/max. First-known on accepted 214 binaries, not an introducing-revision claim.

## Decision Log

2026-09-24, worker: select the semantic gate over batching or unrelated vector admission. Research
only; exact CUDA algorithm behavior and portable API guarantees are distinct acceptance claims.
2026-09-24, worker/parent: stop wider investigation on measured FP32 singleton defect. Prioritize
216 singleton min/max correctness; retain FP64 admission and all nonsingleton questions as deferred.

## Outcomes and Retrospective

Research gate completed at its explicit stop condition. Singleton NaN source-helper identity is
violated on the already supported FP32 path. Source/IR shape is valid; the masked reduction recipe
owns the defect, not the numeric-min provider or front end. Parent directs 216 to reuse existing
singleton preservation where appropriate with scalar/vector/matrix raw-bit coverage. No wider
NaN/order conclusion or FP64 admission is claimed. All accepted 214 corpus evidence is inherited;
this research defect is separate from its 53 registered failures. Full checkpoint 214 and cadence 0
remain unchanged. No production changes, GPU loss, system changes, worker commit or push.

## Context and Current Pipeline

`WaveMultiMin/Max` overloads specialize into scalar or Multiple GenericAsm helpers. CUDA prelude
`WaveOpMin/Max` uses ordered comparison and returns the second operand for ties/unordered inputs.
`_waveReduceScalar/Multiple` uses XOR stages or caller-seeded ascending lane scan. Direct NVVM's
`_getNVVMMaskedWaveScalarIdentity` deliberately rejects FP64 min/max; admitted FP32 uses numeric
MIN/MAX and identity-seeded scanning. The two frozen FP64 failures remain unchanged obligations.

## Scope and Non-Goals

Only plan/report/evidence/STATUS are durable changes. No compiler, provider, prelude, shader contract,
manifest or runner edits; no commits, pushes, system changes, builds of compiler, or material claims.
Stop independent investigation upon a proven supported FP32 defect and hand off its minimal case.

## Architecture and Invariants

The independent oracle retains selected integer words, classifies NaNs by exponent/fraction,
and performs ordered comparisons without host min/max. Butterfly stages use simultaneous prior
lane states. Scan reads original lane words. Every named non-exited lane participates in every
same-mask shuffle; masks are explicit and no implicit scheduling cohort is assumed.

## Interfaces and Dependencies

Existing RelWithDebInfo compiler/provider ABI36; CUDA12.9.2 NVRTC12.9.86, LLVM14, L4 SM89, SM80 target.
Source `build/nvvm-loop/slice-203-env.sh`; native tools per local slang-build skill. Python ctypes
CUDA driver probe or existing runtime machinery; generated sources under ignored slice215 path.

## Milestones

1. Verify latest214 hashes, small runtime smoke; inspect canonical IR and provider semantics.
2. Generate and predeclare bounded oracle data; compile NVRTC and sample NVVM FP32, then execute.
3. Inspect emitted PTX, distinguish language requirements from exact CUDA helper behavior, record
   minimal implementation proposal or supported defect handoff.

## Validation and Acceptance

Fixed matrix: scalar FP32/FP64 plus representative two-component vector and 2x2 matrix; min/max;
full warp, low16, high16, 15/17 partitions, sparse even/odd and singleton31 (at most eight masks).
Input patterns: finite extrema, infinities, alternating signed zeros, all quiet NaNs, all signaling
NaNs, and one quiet/signaling NaN at first/middle/last named lane among finite values. Include mixed
NaNs. Dynamic buffers and raw-word outputs avoid constant folding. Record each oracle/output word.
Oracle drives expected values; NVRTC differential agreement is supplementary. FP64 direct preflight
must remain unsupported. Run FP32 first if a smallest defined contract can reveal a defect; a proven
defect supersedes the rest of the matrix under the explicit stop condition. No corpus rerun needed
if source/artifact identities match214. Preserve all 1,650 cells (1,597 correct / 53 known failures), four
resolved histories, six material compile/assembly cells, latest full 214 and cadence 0 as inherited.

## Failure and Recovery

Bound commands with timeout; GPU loss stops dispatch without driver changes/retries. Sandbox bwrap
fails before execution, so approved escalated commands are necessary. Apparatus errors may be
corrected locally; never redefine an expected result to match GPU output. Ambiguity remains a result.

## Artifacts and Hand-Off

Raw: `build/nvvm-loop/slice-215-semantics/`. Durable: this plan,
`report.slice-215-fp64-minmax-semantics.md`, `semantic-evidence.slice-215.json`, `STATUS.md`.
Parent owns independent acceptance/commit after explicit worker write-ownership return.

Parent acceptance, 2026-09-24: independently reviewed the dynamic-input reproduction, all twelve
result rows and generated NVVM PTX; verified all 21 baseline/raw evidence hashes and exact research
counts. Accepted as research at the predeclared supported-defect stop. Latest full checkpoint 214
and implementation cadence zero remain unchanged. Slice 216 owns the bounded singleton fix.
