# NVVM development handoff

Updated 2026-09-24. Read [WORKFLOW.md](WORKFLOW.md) before starting or resuming the loop.

## Current state and next action

**Slice 213 is accepted with targeted validation.** Read its
[completed worker plan](plan.slice-213-fp64-implicit-shuffle.md),
[five-part report](report.slice-213-fp64-implicit-shuffle.md), and
[result manifest](runtime-validation.slice-213.json). Parent completed independent acceptance
review and owns the local commit. No push is authorized.

The canonical implicit OutParam aggregate shuffle now admits Float64 leaves. The production
change removes only the deferred admission guard; existing typed recursion, raw hardware snapshot
followed by ballot(snapshot,true), and two-word scalar transport are reused. Exact-bit runtime
coverage checks finite high/low words, both zero signs, and quiet/signaling NaN payloads. Partial,
sparse and singleton branches use self-lane reads without assuming a fixed hardware cohort.

Latest accepted implementation: 213. Last full checkpoint: 210. Implementation slices since it: 1.
No full checkpoint is newly required:
provider, ABI 36, catalog, library, general lowering and runners are unchanged.

**Research 211/212 remains accepted.** Complete six-cell API batching improved 10.767% including
global lifetime, with all 168 outputs matching and six PTX hashes freshly assembling. See
[212 report](report.slice-212-batch-lifetime.md) and [measurements](batch-lifetime.slice-212.json).
This remains a performance candidate, not a production-runner speedup. A future protocol must retain
fresh sessions, per-cell deadlines, failure isolation and a full checkpoint before adoption.

Next: slice 214 will integrate bounded opt-in complex compilation batches using the existing
JSON-RPC test-server shared session. Retain fresh-process validation, exact per-cell results,
deadlines and failure isolation; measure complete process startup/teardown and require a full
checkpoint before acceptance. This follows the measured complex-corpus candidate without inventing
material runtime inputs. Rolling 209/210/213 covers mask correctness, shared literal correctness
and FP64 admission; research 211/212 does not advance feature cadence.

## Checkpoints and evidence

| Area                        | Record                                                                                                                     | Interpretation                                                                                 |
| --------------------------- | -------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------- |
| Slice 213 accepted targeted | [Manifest](runtime-validation.slice-213.json), [frozen](census.slice-213.tsv), [discovery](discovery-census.slice-213.tsv) | 615 fresh cells: 577 correct, 38 unchanged failures; 1035 frozen cells explicitly inherit 210. |
| Slice 210 accepted full     | [Manifest](runtime-validation.slice-210.json)                                                                              | 1647 fresh cells: 1594 correct, 53 retained failures; three additions, zero old deltas.        |
| Slice 209 accepted full     | [Manifest](runtime-validation.slice-209.json)                                                                              | Established ABI 36 hardware-mask contract and full preservation.                               |

Fresh frozen 107 x 3: 313 correct, eight preflight stops. Full discovery 98 x 3: 264 correct,
22 infrastructure, four runtime mismatch and four preflight cells. All 612 fresh old cells match
classification, return code, complete execution counts, diagnostic and canonical shape exactly.
Three additions pass; no missing or duplicate cells. Cumulative 1650 cells contain 1597 correct
and 53 open failures. All four resolved 208 histories remain. Historical healthy denominators
remain 427 frozen and 72 discovery; frozen v1 and every old source/oracle are unchanged.

Frozen diagnostic subset exits 0 because it has only preflight gaps. Discovery exits 2 for retained
infrastructure/output gaps. Neither exit substitutes for the exact structured comparison.

## Unresolved failures and limitations

The 213 manifest preserves all 53 open histories and four resolved histories, including 15 failures
inherited with unselected frozen cells. No failure was hidden or reset.

- Quad reconvergence, FP64 min/max and prefix-min/max KernelContext pointer shapes remain separate.
- Ordinary FP64 vector-by-value compound shuffles remain outside this aggregate admission.
- Hardware masks remain scheduling-dependent; logical active-mask synthesis is unchanged.
- FP8/BF16/prelude and discovery infrastructure/output gaps remain visible.
- Six complex cells compile/assemble; application bindings, textures/LUT/input and output oracle
  are absent. No material runtime, speed or correctness claim.

## Current environment and final gates

Native Ubuntu 24.04, branch `nvvm-backend`, repository `/home/skallweit/codex/agent-sandbox/slang`.
L4 SM89, driver 580.126.09, target SM80, CUDA 12.9.2/NVRTC 86, LLVM 14, provider ABI 36. Use matching
optimized tools/libraries in `build/RelWithDebInfo/{bin,lib}`, source `build/nvvm-loop/slice-203-env.sh`
and local `build/nvvm-setup/slang-skills/skills/slang-build/SKILL.md`. Four total CPU workers,
two unit servers, sequential suites and `CMAKE_BUILD_PARALLEL_LEVEL=1`.

Final gates: focused 6/6, smoke 4/4, units 477/477 with one existing Windows-only skip, toolkit
18/18, targeted corpora as above, complex 6/6 compile/assembly, and three focused PTX assemblies.
NVRTC O3 / NVVM O0 / NVVM O3 raw-read/ballot/word-shuffle counts are 4/4/32, 1/1/8 and 4/4/32.
O0 has four calls to the shared helper; both O3 modes retain all three divergent branches and all
four inline sites. Every ballot consumes its raw snapshot and true; no self-shuffle site is folded.

Compiler SHA256: `2775a5783a7dd1310ab9773d259bf1bdafc464a4f55a376c1fc22c472a6b1bf0`.
Provider SHA256: `ae0e7859f6062a69f191cace4ae8d96340c074815866808c5b43467f41144372`.
Tested base: `8075158ca3631ba72cd11df810ffc29d2628e3f7` plus manifest source hashes. Before-change
compiler matches accepted 210; the unchanged final fixture passes NVRTC and rejects both direct
modes at the exact canonical helper. All final hashes match. Raw evidence is under
`build/nvvm-loop/slice-213-{before,after}`. No GPU loss, driver change or reboot occurred; the old
A6000 history remains in 201 without an established cause.
