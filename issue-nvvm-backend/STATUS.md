# NVVM development handoff

Updated 2026-09-24. Read [WORKFLOW.md](WORKFLOW.md) before starting or resuming the loop.

## Current state and next action

**Slice 216 is accepted: FP32 singleton masked min/max preserves original bits.**
Read the [completed plan](plan.slice-216-fp32-singleton-minmax.md),
[five-part report](report.slice-216-fp32-singleton-minmax.md), and
[result manifest](runtime-validation.slice-216.json). The typed singleton predicate and original-value
selection are shared with FP32 min/max; FP64 sum signed-zero seed behavior remains separate.
Dynamic scalar/float4/float2x2 coverage checks all32 singleton lanes, both zero signs, finite values,
infinities and signed payload-distinct quiet/signaling NaNs. Finite nonsingleton neighbors remain correct.

The defect first measured by [research215](semantic-evidence.slice-215.json) is resolved separately
from the registered53 failures. Exact original research source now preserves all24 output words
across12 executions; the four previously mismatching executions are fixed. This is no claim about
nonsingleton NaN/order semantics or portable payload guarantees.

**Next action: resume the deferred slice215 min/max semantic research gate.** Nonsingleton FP32
behavior remains explicitly unresolved, as do the deferred FP64, aggregate and order probes.
The singleton correction does not establish readiness to admit FP64 min/max. Keep independent
sum/product/prefix work and further batching optimization outside that research gate. No push authorized.

Latest accepted targeted implementation is 216. The latest full checkpoint remains 214, with one
accepted implementation slice since that checkpoint.
Rolling213/214/216 covers FP64 implicit shuffle admission, complex-driven process lifetime and proven
singleton correctness; research215 does not advance implementation cadence. Correctness took priority
in216; reconsider material-driven opportunities without inventing missing runtime contracts.

## Checkpoints and evidence

| Area                        | Record                                                                                                                     | Interpretation                                                                              |
| --------------------------- | -------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------- |
| Slice 216 accepted targeted | [Manifest](runtime-validation.slice-216.json), [frozen](census.slice-216.tsv), [discovery](discovery-census.slice-216.tsv) | 618 fresh cells:580 correct,38 unchanged failures. 1035 frozen cells explicitly inherit214. |
| Slice214 accepted full      | [Manifest](runtime-validation.slice-214.json), [frozen](census.slice-214.tsv), [discovery](discovery-census.slice-214.tsv) | All1650 cells fresh:1597 correct,53 known failures. Latest full checkpoint.                 |
| Slice215 accepted research  | [Evidence](semantic-evidence.slice-215.json)                                                                               | Separate singleton defect discovered on exact214 binaries; broader semantic matrix stopped. |

Frozen remains452identities/1356cells. Discovery adds one identity, reaching99/297cells.
Cumulative1653cells contain1600 correct and53 unchanged failures; four resolved histories remain.
Every fresh old cell exactly matches classification, return code, complete execution counts,
diagnostic and canonical shape. No missing/extra/duplicate cells. The original546 runtime source
contracts are unchanged; the new fixture adds one. Historical healthy denominators427/72 stay fixed.
Fresh321 selected frozen cells cover all wave/quad and double/helper/value/vector/matrix neighbors;
full discovery297cells is fresh. Other1035 frozen results are inherited214, not passes on216 source.

Fresh gates: focused10/10, smoke4/4, units478/478 plus the existing Windows-only skip, toolkit18/18,
and all6 material compile/assembly cells. No material runtime bindings/oracle exist. Frozen diagnostic
subset returns0 with eight known preflight stops; discovery returns2 for retained failures. Exact
structured comparison decides acceptance. Broad/uncertain impact would require full checkpoint;
none occurred. Full checkpoint required after three accepted implementation slices or before publishing.

## Unresolved failures and limitations

- All53 open failure records and four resolved histories retain first-known evidence and reproduction.
- FP64 min/max, nonsingleton FP32 min/max NaN/order behavior, quad reconvergence, prefix-min/max
  KernelContext pointers and ordinary FP64 vector-by-value compound shuffles remain separate work.
- Hardware masks remain scheduling-dependent; logical active-mask synthesis is unchanged.
- FP8/BF16/prelude and existing discovery infrastructure/output gaps remain visible.
- The material lacks application bindings, texture/LUT/input contract and runtime output oracle.
  Six cells compile/assemble; no material kernel correctness or speed claim.
- Slice214 batching remains explicit opt-in. Mandatory fresh reference work made complete invocations
  slower despite18.02355% paired compilation-lifecycle improvement; it is no routine accelerator.

## Environment and final evidence

Native Ubuntu24.04, branch `nvvm-backend`, repository `/home/skallweit/codex/agent-sandbox/slang`.
L4 SM89, driver580.126.09, targetSM80, CUDA12.9.2/NVRTC12.9.86, LLVM14, providerABI36. Use matching
optimized `build/RelWithDebInfo/{bin,lib}`, source `build/nvvm-loop/slice-203-env.sh`, and local
`build/nvvm-setup/slang-skills/skills/slang-build/SKILL.md`. Sequential suites, four corpus/build
workers maximum, two unit servers; `CMAKE_BUILD_PARALLEL_LEVEL=1`.

Tested base `230c3e0eae73be3b2ff01e26e3d346e12fe9b86c` plus final emitter/unit/fixture changes.
Compiler SHA256 `fa55d1fdc41988e27e672d4a2ad92b93060293d101f4a7a076be3e68dd08298f`.
Provider unchanged `ae0e7859f6062a69f191cace4ae8d96340c074815866808c5b43467f41144372`.
Raw evidence: `build/nvvm-loop/slice-216-before`, `slice-216-after`, and its `research-replay`.
Before/after fixture SHA256 `d838cc70fe2e4d88bb2b127be48304fce74219bfd6319d25122a8e84815fe381`.
No GPU loss, driver change, reboot, worker commit or push. Parent owns acceptance/local commit.
