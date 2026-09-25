# NVVM development handoff

Updated 2026-09-25. Read [WORKFLOW.md](WORKFLOW.md) before starting or resuming the loop.

## Current state and next action

**Slice 220 is accepted: discovery capacity is 50 through 128, with the current 100 entries unchanged.**
Read the [completed plan](plan.slice-220-discovery-capacity.md),
[five-part report](report.slice-220-discovery-capacity.md) and
[full result manifest](runtime-validation.slice-220.json). All 1,656 runtime cells freshly preserve
accepted outcomes: 1,603 correct and 53 registered failures. All six material compile/assembly cells
pass. An initial unfrozen partial run is explicitly excluded; the accepted run selects all 452
immutable frozen identities using `census.slice-195.tsv`.

**Slice 218 is accepted: FP32 masked min/max preserves its CUDA source algorithm.** Read the
[completed plan](plan.slice-218-minmax-algorithm.md), [five-part report](report.slice-218-minmax-algorithm.md)
and [result manifest](runtime-validation.slice-218.json). Scalar and aggregate leaves use caller-seeded
ordered comparison/selection, descending XOR butterfly stages for low contiguous power-of-two masks,
and ascending scans of original inputs otherwise. FP64 sum seed handling shares the mask classifier
but retains its behavior. Ordinary provider numeric min/max and FP64 admission remain unchanged.

All 288 cases from [research 217](semantic-evidence.slice-217.json) now match their unchanged independent
raw-word oracle, fixing its 96 mismatching executions. All 65,016 active output words and 64,008 inactive
sentinels match. The earlier [singleton finding](semantic-evidence.slice-215.json), fixed in 216, remains
covered. Both research findings are separate from the 53 registered failure histories.

**Slice 219 is accepted research: the FP64 CUDA-helper contract is established.** Read the
[report](report.slice-219-fp64-minmax.md), [plan](plan.slice-219-fp64-minmax.md) and
[evidence](semantic-evidence.slice-219.json). All 112 NVRTC scalar/double2/double2x2 cases match an
independent raw-word oracle, including adjacent finite values, subnormals and both halves of NaN
payloads. Direct NVVM O0/O3 still rejects the canonical double min helper before PTX output. No
FP64 NVVM runtime support is claimed, and no production or registered corpus change occurred.

**Next action: slice 221 admits FP64 masked min/max reductions with a dedicated discovery fixture.**
Use the independent raw-word behavior established in 219 and the typed source algorithm from 218.
Keep prefixes and provider numeric min/max unchanged. Prove the new fixture fails before admission,
replay the unchanged research cases on all three modes, and preserve all old identities/oracles.
Material runtime still needs its application bindings and expected output. No push is authorized.
Fresh-context delegation remains at the app's agent-thread limit; local work uses WORKFLOW's fallback
and does not imply an independent worker review.

Latest accepted implementation and full checkpoint: 220. Latest targeted implementation: 218.
Implementation slices since the full checkpoint: zero. Rolling implementation history 216/218/220
covers two proven correctness fixes and the capacity prerequisite for explicit new coverage;
research 215/217/219 does not advance cadence. Material execution remains blocked on its input contract.

## Checkpoints and evidence

| Area                        | Record                                                                                                                     | Interpretation                                                                      |
| --------------------------- | -------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------- |
| Slice 220 accepted full     | [Manifest](runtime-validation.slice-220.json), [frozen](census.slice-220.tsv), [discovery](discovery-census.slice-220.tsv) | All 1,656 cells fresh: 1,603 correct and 53 known failures. Latest full checkpoint. |
| Slice 218 accepted targeted | [Manifest](runtime-validation.slice-218.json)                                                                              | FP32 min/max algorithm correction; all outcomes freshly preserved in 220.           |
| Slice 214 accepted full     | [Manifest](runtime-validation.slice-214.json)                                                                              | Historical full checkpoint before singleton and source-algorithm fixes.             |

Frozen remains 452 identities/1,356 cells. Discovery remains 100 identities/300 cells. Every cell's
classification, return code, complete execution counts, diagnostic and canonical shape matches the
latest applicable accepted evidence. There are no additions, missing/extra/duplicate cells or inherited
runtime cells. All 548 runtime input contracts are unchanged. Historical healthy denominators 427/72
stay fixed; all 53 first-known failure records and four resolved histories remain intact.

Fresh 220 gates: discovery contracts 6/6, routing/reporter 32/32, protocol 15/15, GPU smoke 4/4 and
material compile/assembly 6/6. Compiler units 478 plus one existing Windows-only skip and toolkit
18 explicitly inherit 218 because their source, binaries and toolkit are unchanged. Both corpus
runners return two for retained failures; structured outcomes decide acceptance. No material runtime
correctness or performance claim is made.

## Unresolved failures and limitations

- All 53 open failure records and four resolved histories retain first-known evidence and reproduction.
- FP64 min/max admission, quad reconvergence, prefix-min/max
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

Tested base `c66d533b5b030220abe6ca2411049b68b3365e26` plus the bounded discovery loader/test change.
Compiler SHA256 `a13354a47acefc8684cbfb5ae52b84bcf0331a83e5310e9675f53a0e030107d7`.
Provider unchanged `ae0e7859f6062a69f191cace4ae8d96340c074815866808c5b43467f41144372`.
Raw evidence: `build/nvvm-loop/slice-220-before` and `slice-220-after`, including the explicitly
excluded initial partial run and both command snapshots. Parent verified 117 evidence references,
22 tested source hashes, 12 artifact hashes and 548 runtime input hashes.
No GPU loss, driver change, reboot or push. Local parent review/acceptance follows the recorded
fresh-context delegation limitation.
