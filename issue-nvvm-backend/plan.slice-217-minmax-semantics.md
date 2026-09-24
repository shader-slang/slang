# Audit nonsingleton masked min/max semantics

This bounded research ExecPlan follows `.agent/PLANS.md` and the NVVM exception requiring completed
plans/reports to be committed. Raw probes remain ignored under `build/`.

## Purpose and Observable Result

Determine whether the supported FP32 masked minimum/maximum recipe preserves CUDA source-helper
behavior for nonsingleton scalar/vector/matrix values, before proposing FP64 admission. Distinguish
an injected result absent from all inputs from order-dependent NaN payload or signed-zero differences.

## Progress

- [x] Read workflow, current status, accepted 215/216 evidence and CUDA reduction source.
- [x] Select bounded semantic research on clean base `195288be83128a19c4c699156a87bf4905e6bdd4`.
- [x] Verify accepted source/artifact/input identities and run GPU smoke.
- [x] Run independent bitwise oracle checks and bounded FP32 matrix.
- [x] Classify results, inspect PTX and write report/evidence/status.

## Surprises and Discoveries

Fresh-context delegation failed with `agent thread limit reached`. WORKFLOW explicitly permits
local execution when delegation is unavailable. The parent owns this research; no other worker writes.

## Decision Log

2026-09-24: Resume the deferred 215 gate after accepted 216 fixed singleton preservation. Compare
finite controls, infinities, zeros, all-NaN and positional NaN families in one bounded matrix so a
single algorithmic problem is characterized before selecting the next implementation slice.

## Outcomes and Retrospective

Completed 288 FP32 executions: NVRTC 96/96 exact; NVVM O0/O3 each 48/96 exact.
All-NaN nonsingletons produce injected infinities. Six retained-PTX probes establish the scalar
defect predates 216. Finite, infinity and singleton controls pass. FP64 work stops as planned;
next slice corrects the source min/max algorithm. No production change or FP64 admission.

## Context and Current Pipeline

`hlsl.meta.slang` produces canonical scalar and Multiple GenericAsm helpers. CUDA's WaveOpMin/Max
uses ordered comparisons, selecting the second operand on ties or unordered comparisons. For low-bit
contiguous power-of-two masks `_waveReduceScalar/Multiple` performs caller-seeded simultaneous XOR
stages; other nonsingletons scan original named-lane values in ascending order, seeded by the caller.
The NVVM scalar recipe instead starts from infinity and uses numeric min/max. Accepted 216 selects
the untouched original value for singleton masks. This research audits the remaining algorithm.

## Scope and Non-Goals

Durable edits are this plan, a five-part report, semantic evidence and STATUS only. No compiler,
provider, library, runner, corpus selection or oracle-contract edits. No builds, pushes or system
changes. Sum/product/prefix and batching are outside scope. No material runtime claims.

## Architecture and Invariants

The independent oracle compares IEEE raw words without host floating min/max: classify NaNs by
exponent/fraction, treat both zeros as equal, compare non-NaNs by sign-aware integer ordering, and
return the original selected word. Butterfly steps read a simultaneous previous state. Scan steps
read original lane inputs. Every named lane participates with the same explicit mask; nonparticipants
must retain output sentinels. Differential NVRTC agreement supplements this oracle.

## Interfaces and Dependencies

Native Ubuntu, optimized RelWithDebInfo, L4 SM89 target SM80, driver 580.126.09, CUDA 12.9.2,
NVRTC 12.9.86, LLVM 14, provider ABI 36. Source `build/nvvm-loop/slice-203-env.sh`; follow the local
slang-build skill. Compiler hash `fa55d1fdc41988e27e672d4a2ad92b93060293d101f4a7a076be3e68dd08298f`.
Provider hash `ae0e7859f6062a69f191cace4ae8d96340c074815866808c5b43467f41144372`.

## Milestones and Validation

1. Verify all accepted 216 source/artifact and runtime input hashes, then run:
   `python3 extras/validate-nvvm-runtime.py --config RelWithDebInfo --cuda-path "$CUDA_PATH" --architecture 80 --output build/nvvm-loop/slice-217-semantics/runtime`.
2. Generate one dynamic FP32 kernel with scalar, float2 and float2x2 min/max. Use eight masks:
   full, low16, high16, low15, high17, even, odd, singleton31. Use finite extrema, infinities,
   alternating signed zero, payload-distinct all-qNaN/all-sNaN/mixed-NaN, and single quiet/signaling
   NaN at first/middle/last named lane among finite values. Execute all three modes with bounded
   commands, assemble each distinct PTX, retain raw input/expected/actual words.
3. If FP32 exposes a supported compatibility defect, complete only this bounded pattern-family
   classification and stop before FP64 GPU probes. Record a minimal case and one responsible next
   slice. Otherwise extend the independent model to FP64 under a recorded plan revision.
4. Recheck identities, inspect responsible PTX/source and distinguish portable language guarantees,
   exact CUDA-helper behavior, and order-sensitive differences. No order difference alone establishes
   a universal compiler bug. Ambiguity is a valid result.

## Preservation and Acceptance

All 1,653 registered outcomes inherit accepted 216: 1,600 correct, 53 open failures and four resolved
histories. Its 1,035 unexecuted frozen cells remain explicitly inherited from full 214. Units 478 plus
one skip, toolkit 18 and six material compile/assembly cells inherit unchanged binaries. Research adds
no registered cells and does not advance cadence: latest full 214, one accepted implementation since.
Research 215's singleton defect remains separately resolved by 216. Only fresh research outputs and
smoke are claimed for 217. Exact evidence hashes and independent oracle self-checks gate acceptance.

## Failure and Recovery

Sandbox bwrap fails before execution; approved escalated commands are necessary. Retain failed
apparatus logs if corrected. GPU loss stops all further GPU work without driver changes or reboot.
Do not modify expected output to match GPU output. Uncertain contracts stay explicit.

## Artifacts and Hand-Off

Raw: `build/nvvm-loop/slice-217-semantics/`. Durable:
`plan.slice-217-minmax-semantics.md`, `report.slice-217-minmax-semantics.md`,
`semantic-evidence.slice-217.json`, `STATUS.md`. Local parent performs review and commit because the
agent thread limit prevented delegation; no production implementation occurs in this slice.

2026-09-24 acceptance: reviewed integer oracle hand checks, participation and marshaling, all mode
counts, PTX seed/combine instructions and retained baseline hashes. The six supplementary scalar
probes use accepted 214 PTX without overwriting history and establish preexistence. All source,
artifact and runtime input hashes rechecked after execution. Research accepted locally under the
recorded delegation limitation; full 214 and cadence one remain unchanged.
