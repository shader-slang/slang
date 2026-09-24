# NVVM development handoff

Updated 2026-09-24. Read [WORKFLOW.md](WORKFLOW.md) before starting or resuming the loop.

## Current state and next action

**Slice 214 is accepted with a full checkpoint.** Read the
[completed worker plan](plan.slice-214-complex-batching.md),
[five-part report](report.slice-214-complex-batching.md), and
[result manifest](runtime-validation.slice-214.json). Parent completed independent acceptance review;
no push is authorized.

The complex runner now has an explicit `--test-server` option for bounded batches of at most six
requests through the existing shared global session. The default fresh CLI path is unchanged.
Mandatory fresh references, exact PTX equality, entry/target checks, independent assembly, per-cell
deadlines and bounded process shutdown remain required. Failures preserve the completed prefix,
active failure, incomplete suffix and logs without retries or fallback. No compiler/provider/ABI
or shader/oracle change occurred.

**This is not a routine checkpoint accelerator.** The paired six-cell compilation lifecycle improves
18.02355% (9.991360 to 8.190561 seconds), but mandatory reference work makes both observed complete
invocations slower: one sample takes 13.40 seconds fresh versus 24.08 shared; actual default counts
(warmup 1, samples 3) take 43.87 versus 48.83 seconds. These whole-command values are single
observations, not repeated benchmark estimates. Keep the existing default and explicit opt-in.

Next action: start slice 215 as a bounded semantic research gate for FP64 masked min/max, the two existing frozen failures. Audit
CUDA comparison/select semantics, seeds, and XOR versus ordered reduction branches for NaNs, signed
zero and order dependence before considering production admission. No further batching optimization
belongs to this slice. Latest accepted implementation and full checkpoint: 214. Implementation
slices since the full checkpoint: 0. Rolling 210/213/214 covers shared literal correctness, FP64 implicit shuffle admission,
and complex-driven process-lifetime work; research 211/212 does not advance implementation cadence.

## Checkpoints and evidence

| Area                        | Record                                                                                                                     | Interpretation                                                                           |
| --------------------------- | -------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------- |
| Slice 214 accepted full     | [Manifest](runtime-validation.slice-214.json), [frozen](census.slice-214.tsv), [discovery](discovery-census.slice-214.tsv) | All 1,650 fresh cells exactly preserve 210+213; 1,597 correct and 53 unchanged failures. |
| Slice 213 accepted targeted | [Manifest](runtime-validation.slice-213.json)                                                                              | Added three FP64 implicit aggregate-shuffle cells; all retained in accepted 214.         |
| Slice 210 accepted full     | [Manifest](runtime-validation.slice-210.json)                                                                              | Previous full preservation baseline, combined with 213's additions/fixes.                |

Frozen 452 × 3 gives 1,333 correct, five infrastructure and 18 preflight cells. Discovery 98 × 3
gives 264 correct, 22 infrastructure, four runtime mismatch and four preflight cells. All five
stable fields match exactly: classification, return code, complete execution counts, diagnostic
and canonical shape. There are no missing, extra or duplicate cells, source/selection/oracle
changes, or additions. All 546 distinct runtime sources match accepted base 786a2452. Historical
healthy denominators remain 427 frozen and 72 discovery. Both full runners return 2 for retained
failures; their exit codes alone do not decide acceptance.

All 53 open failure records and four resolved histories retain their original first-known evidence
and reproduction. Every runtime row is fresh in 214; no runtime outcome is inherited. Compiler
units 477/477 plus one existing Windows-only skip and toolkit 18/18 explicitly inherit 213 because
their source/binary/toolkit hashes are unchanged. Fresh gates cover GPU smoke 4/4, routing/reporter
32/32, discovery contracts 4/4 and protocol/default/shared contracts 15/15. Six primary complex
cells compile/assemble in both paths; references and repeated timing work are auxiliary coverage.

## Unresolved failures and limitations

- Quad reconvergence, FP64 min/max, prefix-min/max KernelContext pointers and ordinary FP64
  vector-by-value compound shuffles remain separate support work.
- Hardware masks remain scheduling-dependent; logical active-mask synthesis is unchanged.
- FP8/BF16/prelude and existing discovery infrastructure/output gaps remain visible.
- The material has no application bindings, texture/LUT/input contract or runtime output oracle.
  Six complex cells compile/assemble; no material kernel correctness or speed claim.
- The batching evidence covers one native Linux host and finite sessions. Windows supervision is
  untested; POSIX process-group cleanup and pipe-holding descendants are tested.

## Environment and final evidence

Native Ubuntu 24.04, branch `nvvm-backend`, repository `/home/skallweit/codex/agent-sandbox/slang`.
L4 SM89, driver 580.126.09, target SM80, CUDA 12.9.2/NVRTC 86, LLVM 14, provider ABI 36. Use matching
optimized `build/RelWithDebInfo/{bin,lib}`, source `build/nvvm-loop/slice-203-env.sh`, and consult
local `build/nvvm-setup/slang-skills/skills/slang-build/SKILL.md`. Suites are sequential with four
corpus workers, two routing/reporter servers and `CMAKE_BUILD_PARALLEL_LEVEL=1`.

The performance run has two paired warmups and twelve measured pairs with alternating policy order
and balanced forward/reverse rotations. All 28 batches / 168 outputs are exact and all six distinct
PTX hashes assemble. Startup and final process exit are included. Shared per-cell phase medians
remain null; service latency is not an isolated compiler-call timing. A later parser-exception-only
correction is recorded with the exact timed source snapshot; final protocol and material checks use
final sources. Parent independently recomputed the timing and full preservation results.

Compiler library SHA256: `2775a5783a7dd1310ab9773d259bf1bdafc464a4f55a376c1fc22c472a6b1bf0`.
Provider SHA256: `ae0e7859f6062a69f191cace4ae8d96340c074815866808c5b43467f41144372`.
Tested base: `786a2452f576fc620c9eeb4019bb277a20f394cd`, plus final Python source hashes in the manifest.
Raw evidence: `build/nvvm-loop/slice-214-after`, `slice-214-performance`, `slice-214-shared-smoke`
and adjacent protocol logs. No GPU loss, driver change, reboot or worker commit occurred. The old
A6000 history remains in 201 without an established cause.
