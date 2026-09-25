# NVVM development handoff

Updated 2026-09-25. Read [WORKFLOW.md](WORKFLOW.md) before starting or resuming the loop.

## Current state and next action

**Slice 237 is accepted as the latest full checkpoint.** Read the
[completed plan](plan.slice-237-clock.md), [five-part report](report.slice-237-clock.md),
[full validation](runtime-validation.slice-237.json) and raw `slice-237-after/audit.json`.
Exact GenericAsm `clock` uint() and `clock64` int64_t() now select ABI 37 semantic operations.
The shared catalog enforces signatures and the existing canonical helper resolver owns admission;
the provider emits side-effecting inline PTX special-register reads. No frontend/library/runner
change. Invalid types, arity and noncanonical bodies still reject before provider loading.

Independent parent acceptance passes. Latest accepted implementation/full checkpoint: 237;
latest targeted acceptance: 233; implementation slices since full: zero. Rolling feature history
is 233 FP16 MIN/MAX, 235 quad helpers and 237 live clocks. No push is authorized.

Next action: research the measured scalar BF16 boundary and its source conversion/arithmetic
contract before selecting a bounded implementation. The matrix mismatch is already documented as
CUDA's target-wide layout limitation; arbitrary prelude text and resource boundaries remain separate.
Material runtime bindings, textures/LUT/input and output oracle are still absent; retain its six
compile/assembly support cells without a runtime or performance claim.

## Checkpoints and evidence

- [Full 235](runtime-validation.slice-235.json) remains immutable comparison input: 1,680 cells,
  1,635 correct, 45 unresolved and 12 resolved histories. Research 236 remains accepted and immutable.
- Full 237 freshly reruns 452 frozen identities/1,356 cells using explicit immutable slice 195
  selection: 1,343 correct, five infrastructure and eight preflight outcomes. Only original
  `slang-extension/realtime-clock.slang#cuda-1` direct O0/O3 cells transition to GPU-correct.
  [Frozen census](census.slice-237.tsv).
- Discovery freshly reruns 108 old identities/324 cells plus one new fixture/three correct cells:
  109 identities/327 cells, 297 correct, 22 infrastructure, four mismatch, four preflight outcomes.
  [Discovery census](discovery-census.slice-237.tsv). Frozen sources and all old oracles are unchanged.
- Total 1,683 fresh cells/1,640 correct/43 retained failures/14 resolved histories. All 1,680 old
  cells compare exactly on classification, return code, complete execution counts, diagnostic and
  canonical shape except the two clock fixes. No missing/extra/duplicate/inherited cells, lost
  support or baseline reset. The 12 older resolved histories and each newly resolved whole prior
  failure record remain intact; all 43 unresolved first-known records/reproductions remain.
- Final-source smoke 4/4 precedes expensive suites; fixture 3/3, units 481/481 plus one existing
  Windows-only skip, toolkit 18/18, runner contracts 6/6 and material compile/assembly 6/6 pass.
  Two new real-provider units check wrong-sign/arity query and emission rejection, distinct
  sideeffect serialization in both LLVM dialects, and no clock-intrinsic/convergence substitution.
  Ten negative source cases extend the existing pre-provider rejection matrix.
- Final readable fixture passed NVRTC and rejected direct O0/O3 before production edits; its
  source/TEST_INPUT never changed afterward. Dynamic 0/2/5/16-round clock order/bracket/progress
  checks and independent affine expectations pass on final source in all three modes.
- Research replay runs 60 launches on all 12 unchanged research 236 buffers: public NVRTC/O0/O3
  and sideeffect-control O0/O3. A separate checker independently evaluates 11,040 clock tuples,
  66,240 active words, 184,320 total output words, unchanged 5,760 input words and inactive sentinels.
  Five assemblies pass. No timestamp equality is required; historical intrinsic countermodel
  failures remain unchanged research evidence. All 241 indexed research artifacts preserve hashes.

## Unresolved failures and limitations

- Clock observations are nondeterministic per-SM wrapping counters. The bounded modular predicates
  make no cross-SM synchronization, frequency, wall-time, memory-fence or performance claim. The
  tests account mathematically for low-word wrap without claiming observed hardware wrap.
- BF16/FP8/prelude and existing infrastructure/output gaps remain visible among 43 failures. Other
  arithmetic/bitwise families, matrix prefix and resource contexts remain independent work.
- Material's six cells are freshly checked compile/assembly support only. Runtime/performance
  remain blocked by missing bindings/textures/LUT/input/output oracle. Slice 214 batching is opt-in.

## Environment and final evidence

Native Ubuntu 24.04, branch `nvvm-backend`, repository `/home/skallweit/codex/agent-sandbox/slang`.
L4 SM89, driver 580.126.09, target SM80, CUDA 12.9.2/NVRTC 12.9.86, LLVM 14, provider ABI 37.
Use matching optimized `build/RelWithDebInfo/{bin,lib}`, the inspected
`build/nvvm-loop/slice-203-env.sh` and local
`build/nvvm-setup/slang-skills/skills/slang-build/SKILL.md`. Maximum four CPU workers, two unit
servers, sequential suites and 30-minute bounds; `CMAKE_BUILD_PARALLEL_LEVEL=1`.

Tested source_commit/source_revision is actual base `cc0982c68365933cbd6b7885e94cc033d3eea566`
plus recorded patch. Compiler-library SHA256:
`a89e9b370b03e62a62fe5f6becaab312399d5cec60bac6d75a53ff649a75c19f`.
Provider SHA256: `dafc5a557ce6f83d358c89956910af9761e352bb2f70f5efc6d5e7bc7f8a89ea`.
Before compiler/provider remain full 235's `ca34db1a349ae8716785032a0a3b01b3e6cf8455f3137e9358e9d1ad4eca63cf`
and `ae0e7859f6062a69f191cace4ae8d96340c074815866808c5b43467f41144372`.

Each gate captures 35 source paths (all 31 prior paths plus API header/catalog/builder unit/fixture),
12 artifacts and 557 registered input hashes. All 556 old input hashes remain exact. Raw roots are
`build/nvvm-loop/slice-237-before` and `slice-237-after`; final-source patch, gates, provenance,
replay raw buffers, separate checker and complete history/reference audit are retained there.
Initial formatter invocation lacked the environment tool paths and changed nothing; the proper
explicit-path formatter then ran, unrelated historical hunks were exactly reversed, and final build
and every gate ran afterward. No edited executing scripts, incomplete passing gates, GPU loss,
driver/system change, reboot, worker commit or push.

Parent acceptance verifies 672 unique compact references and 917 unique references including
research artifacts, all 35 source/12 artifact/557 input hashes, exact old outcomes and histories,
and every complete clock replay buffer. See `slice-237-after/parent-acceptance-audit.json`.
Only two old clock cells resolve and three fixture cells are added; full checkpoint 237 resets
implementation cadence to zero.
