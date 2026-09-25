# NVVM development handoff

Updated 2026-09-25. Read [WORKFLOW.md](WORKFLOW.md) before starting or resuming the loop.

## Current state and next action

**Slice 221 is accepted: scalar and aggregate FP64 masked min/max reductions execute correctly.**
Read the [completed plan](plan.slice-221-fp64-minmax-admission.md),
[five-part report](report.slice-221-fp64-minmax-admission.md) and
[result manifest](runtime-validation.slice-221.json). The existing typed source algorithm now admits
one-lane Float64 leaves, preserving caller seeds, ordered comparison/selection and source butterfly/
scan order. FP64 arithmetic singleton/seed handling, prefixes and provider numeric min/max are unchanged.

All 336 cases replayed from [research 219](semantic-evidence.slice-219.json) match their unchanged
independent integer oracle: 112 per mode, 75, 852 active doubles and 74, 676 inactive double sentinels.
The earlier FP32 findings from research 215/217 remain covered by their accepted216/218 fixes.

**Next action: bounded research 222 checks the observed unsigned firstbithigh behavior.** The first
new-fixture draft showed CUDA `U32_firstbithigh` complements words with bit31 set, producing unexpected
partition endpoints. Isolate scalar/vector uint and uint64 boundary cases with independent expected
indices, compare NVRTC/NVVM and inspect source helpers before selecting a fix. Retain the initial
221 fixture/diagnostic evidence; no established corpus oracle was changed. Do not fold prefix, quad
or vector-by-value support into this investigation. Material runtime still needs its input contract.
No push is authorized. Fresh-context delegation remains at the app's agent-thread limit; local work
uses WORKFLOW's fallback and does not imply independent worker review.

Latest targeted implementation: 221. Latest full checkpoint: 220. Implementation slices since full: one.
Rolling implementation history218/220/221 covers the FP32 correctness fix, explicit discovery capacity
and FP64 support admission. Correctness research takes priority; material runtime remains blocked on
bindings, texture/LUT/input and output oracle. Discovery capacity is 50 through 128; current count 101.

## Checkpoints and evidence

| Area                        | Record                                                                                                                     | Interpretation                                                                 |
| --------------------------- | -------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------ |
| Slice 221 accepted targeted | [Manifest](runtime-validation.slice-221.json), [frozen](census.slice-221.tsv), [discovery](discovery-census.slice-221.tsv) | 624 fresh cells: 588 correct, 36 known failures;1035 frozen cells inherit220.  |
| Slice 220 accepted full     | [Manifest](runtime-validation.slice-220.json)                                                                              | All 1656 cells fresh: 1603 correct, 53 known failures. Latest full checkpoint. |
| Slice 218 accepted targeted | [Manifest](runtime-validation.slice-218.json)                                                                              | FP32 source-algorithm correction, preserved through full 220 and targeted221.  |

Frozen remains452 identities/1356 cells. Discovery has 101 identities/303 cells. Cumulative 1659 cells have
1608 correct and51 open failures. Two prior FP64 min/max preflight cells become correct and three new
fixture cells pass. All other fresh classification/return-code/execution-count/diagnostic/canonical-
shape fields match220 exactly. There are no missing/extra/duplicate cells. All 548 old runtime inputs
and100 old manifest rows are unchanged. Historical healthy denominators 427/72 stay fixed.
All 53 prior first-known records survive: 51 open and2 newly resolved; four earlier resolved histories remain.

Fresh221 gates: focused16/16, GPU smoke4/4, units478/478 plus one existing Windows-only skip, toolkit18/18,
discovery contracts6/6, research 336/336 and material compile/assembly6/6. Frozen diagnostic mode returns
zero for six retained preflight stops; discovery returns two for known failures. Structured outcomes
decide acceptance. No material runtime correctness or performance claim is made.

## Unresolved failures and limitations

- All 51 open failure records and six resolved histories retain first-known evidence and reproduction.
- Quad reconvergence, prefix-min/max
  KernelContext pointers and ordinary FP64 vector-by-value compound shuffles remain separate work.
- Hardware masks remain scheduling-dependent; logical active-mask synthesis is unchanged.
- FP8/BF16/prelude and existing discovery infrastructure/output gaps remain visible.
- The material lacks application bindings, texture/LUT/input contract and runtime output oracle.
  Six cells compile/assemble; no material kernel correctness or speed claim.
- Slice 214 batching remains explicit opt-in. Mandatory fresh reference work made complete invocations
  slower despite 18.02355% paired compilation-lifecycle improvement; it is no routine accelerator.

## Environment and final evidence

Native Ubuntu 24.04, branch `nvvm-backend`, repository `/home/skallweit/codex/agent-sandbox/slang`.
L4 SM89, driver 580.126.09, target SM80, CUDA 12.9.2/NVRTC 12.9.86, LLVM 14, provider ABI 36. Use matching
optimized `build/RelWithDebInfo/{bin,lib}`, source `build/nvvm-loop/slice-203-env.sh`, and local
`build/nvvm-setup/slang-skills/skills/slang-build/SKILL.md`. Sequential suites, four corpus/build
workers maximum, two unit servers; `CMAKE_BUILD_PARALLEL_LEVEL=1`.

Tested base `40f19e54124e1b1d88625cfcc94e4e75b98bdec0` plus bounded admission/unit/fixture changes.
Compiler SHA256 `55bd12f280ee51def87219c767557e198cbd07b9b99f06d118a20209dfb46598`.
Provider unchanged `ae0e7859f6062a69f191cace4ae8d96340c074815866808c5b43467f41144372`.
Raw evidence: `build/nvvm-loop/slice-221-before` and `slice-221-after`, including unchanged-source
research replay and the initial fixture/structural apparatus corrections. Parent verified 132 evidence
references, 23 tested source hashes, 12artifact hashes and549 runtime input hashes.
No GPU loss, driver change, reboot or push. Local parent review/acceptance follows the recorded
fresh-context delegation limitation.
