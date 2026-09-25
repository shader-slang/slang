# NVVM development handoff

Updated 2026-09-25. Read [WORKFLOW.md](WORKFLOW.md) before starting or resuming the loop.

## Current state and next action

**Slice 233 is accepted.** Read the
[completed plan](plan.slice-233-fp16-minmax.md), [five-part report](report.slice-233-fp16-minmax.md)
and [targeted validation](runtime-validation.slice-233.json). Half masked MIN/MAX reductions and
inclusive/exclusive prefixes use the existing source-order recipe with finite exclusive seeds
`0x7bff`/`0xfbff`. No provider/ABI, frontend, aggregate classifier or tree/scan implementation changes.

All four original frozen MIN/MAX prefix direct cells are now GPU-correct. The next independent
boundary in the selected frozen domain is
`tests/hlsl-intrinsic/quad-control/quad-control-comp-functionality.slang`: both direct modes retain
`direct NVVM lowering does not support Slang IR instruction or shape 'RequireMaximallyReconverges'`.
The unchanged first-known record, full diagnostic and reproduction remain in the validation ledger.
Research 234 is accepted; see the
[quad contract report](report.slice-234-quad-reconvergence.md) and
[semantic evidence](semantic-evidence.slice-234.json). The markers are target requirements inside
a canonical CUDA GenericAsm helper, whose whole body source emission replaces. Standalone markers
are not CUDA no-ops. The next bounded implementation candidate is typed `_slang_quadAny`/
`_slang_quadAll` helper admission with both markers accounted for at that boundary, preserving
standalone rejection and the full-mask shuffle source contract. No compiler change was made.
Matrix prefix capability and other arithmetic families remain separate candidates.

Latest accepted implementation and targeted acceptance: 233. Latest full checkpoint: 229.
Implementation slices since full: two. A full checkpoint is required after the third implementation
before a fourth.
Rolling feature history: 229 narrow integer MIN/MAX, 231 64-bit integer MIN/MAX,
233 FP16 MIN/MAX. Research 230/232 does not advance cadence. Research-backed correctness takes
priority while the material application contract is absent; reconsider material work each slice.
No push is authorized.

## Checkpoints and evidence

- [Full checkpoint 229](runtime-validation.slice-229.json): 1,671 cells, 1,620 correct and 51 known
  failures; six resolved histories. Frozen 452 identities/1,356 cells and discovery 105/315.
- [Targeted 231](runtime-validation.slice-231.json): 1,674 cumulative cells, 1,623 correct and
  51 known failures; six resolved histories. This is the comparison baseline for 233.
- [Research 232](semantic-evidence.slice-232.json): independently derived raw binary16 source
  semantics, finite seeds and existing-operation controls. All 17,736 accepted raw binaries,
  source/input/expectation/result manifests and helper scripts remain untouched.
- [Targeted 233](runtime-validation.slice-233.json): 642 fresh cells and 1,035 explicitly inherited
  full229 cells; cumulative 1,677 cells, 1,630 correct, 47 unresolved failures and ten resolved
  histories. The only four old five-field transitions are preflight-to-correct prefix resolutions;
  complete prior failure records and histories accompany each transition and fresh proof.
  [Frozen census](census.slice-233.tsv); [discovery census](discovery-census.slice-233.tsv).
- Selected frozen 107 identities/321 cells: 319 correct, two unchanged quad preflight stops.
  The remaining 1,035 frozen cells inherit full229; no fresh execution is implied for them.
  Discovery 107 identities/321 cells: 291 correct, 22 infrastructure, four mismatch and four
  preflight outcomes. All 318 old discovery outcomes preserve classification, return code,
  complete execution counts, diagnostic and canonical shape exactly. The new fixture adds three
  correct cells separately. No missing, duplicate or unexpected cells; no baseline reset.
- The dynamic raw16 fixture passed NVRTC and rejected both direct modes on the accepted compiler
  before production edits. Its final source and TEST_INPUT directives stayed unchanged afterward.
  Closed-form expectations cover finite seeds, infinities, singletons, raw NaNs/ties, signed zero,
  normal/subnormal boundaries, scalar/vector2/vector4 operations and matrix reductions.
- Exact research replay passes 8,868 launches and 10,783,488 output words: 4,434 family and 4,434
  controls. It reads accepted232 input/expectation binaries unchanged and preserves all 192-word
  input regions and inactive sentinels. All 40 minimal direct family probes compile; six runtime
  PTX artifacts assemble. Twelve matrix prefix capability rejections remain exact. Six catalog
  checks retain four admitted half operations and two excluded numeric MIN/MAX descriptors.
- Fresh smoke 4/4 ran before expensive GPU suites; focused 3/3, units 479/479 plus one existing
  Windows-only skip, toolkit 18/18, discovery contracts 6/6 and all six material compile/assembly
  cells pass. All 92 excluded arithmetic signatures and 14 ordinary aggregate shuffle signatures
  preserve their exact before/after E52017 diagnostics.

All 554 old registered input hashes and all old discovery rows remain unchanged; the fixture adds
input 555. Historical healthy denominators 427/72 remain fixed. All 51 prior failure records survive
as 47 unresolved records and four complete newly resolved histories; six earlier resolved histories
remain unchanged. Compact evidence hash-addresses raw replay indexes instead of embedding every binary.

## Unresolved failures and limitations

- Quad `RequireMaximallyReconverges`, matrix prefix capability, arithmetic/bitwise families,
  ordinary aggregate shuffle policies and resource-bearing contexts remain independent.
- Hardware masks remain scheduling-dependent; logical active-mask synthesis is unchanged.
- FP8/BF16/prelude and existing discovery infrastructure/output gaps remain visible.
- Material runtime remains blocked by application bindings, texture/LUT/input and output oracle.
  Its six registered cells are compile/assembly checks only; no runtime or performance claim.
- Slice 214 batching remains opt-in; no new performance claim.
- Broad replay covers every binary16 encoding among full-mask lane/component slots, plus 69
  structured patterns under 14 masks. It is not exhaustive tuples, masks or caller combinations.

## Environment and final evidence

Native Ubuntu 24.04, branch `nvvm-backend`, repository `/home/skallweit/codex/agent-sandbox/slang`.
L4 SM89, driver 580.126.09, target SM80, CUDA 12.9.2/NVRTC 12.9.86, LLVM 14, provider ABI 36.
Use matching optimized `build/RelWithDebInfo/{bin,lib}`, source
`build/nvvm-loop/slice-203-env.sh`, and local `build/nvvm-setup/slang-skills/skills/slang-build/SKILL.md`.
Sequential GPU suites, at most four CPU workers, two unit servers; `CMAKE_BUILD_PARALLEL_LEVEL=1`.

Slice 233 tested base `54ec1bfe5d92f5f7361ae85a7815ae1767e7a46a` plus the recorded emitter patch;
both source_commit and source_revision retain that actual base. Final compiler-library SHA-256:
`92ae81d069aeda9a6ff2a61edec43f572b2af02bb7ec677fc444490ea9a966f1`.
Before compiler: `948d300ec9f21f9000d242fcd83ee109a97ec63e1a768511553bc64c35b21c08`.
Provider remains `ae0e7859f6062a69f191cace4ae8d96340c074815866808c5b43467f41144372`.
Every gate captures all 29 old tested-source paths, 12 artifacts and the final fixture hash.
Only emitter and discovery registration change among the old source paths. Final source/artifact/input
hashes were rechecked after the gates. Raw evidence lives under `build/nvvm-loop/slice-233-before`
and `slice-233-after`; `audit.json` records complete history, census and binary verification.
No device loss, driver/system change, reboot, worker commit or push.

Parent acceptance independently verified 669 unique compact evidence references, all current
source/artifact/input hashes, exact cumulative census preservation and complete failure histories.
All 8,868 raw GPU output buffers match accepted research expectations with unchanged input regions;
all 17,736 accepted research binaries remain intact. See `parent-acceptance-audit.json`.

## Research 234 handoff awaiting acceptance

Fresh smoke 4 passes. Exactly three frozen quad cells preserve all five outcome fields: NVRTC correct,
direct O0/O3 unchanged preflight. Dynamic research passes 2,048 launches/131,072 expected output
words with 512 shared input/expectation pairs, unchanged 96-word input regions and inactive sentinels.
Public NVRTC quad helpers and explicit-shuffle controls at NVRTC/direct O0/O3 cover all 16 Boolean
quad truth tables, 4/16/32-thread blocks, complete-quad exits, divergent work before common calls,
and divergent inline/noinline calls. Fourteen PTX artifacts assemble. Public direct probes still
reject; marker-free helpers independently expose `_slang_quadAny/All, signature=bool(bool)`.

SM80 synchronized shuffles support matching rendezvous across divergent branches. The frozen source
has complete quads and four matching shuffles per branch, so its behavior has a documented basis.
Partial-quad source reads remain undefined and were not launched or promoted to an oracle.
No CUDA-wide maximal-reconvergence or active-only-vote promise is inferred. Raw evidence is under
`build/nvvm-loop/slice-234-quad`; source revision/commit are actual
`f77142a2e8d54b66482dc85b4e99e2a6e28de777`. All 555 registered inputs, 30 tested source paths and
12 artifacts preserve 233. No rebuild, production/provider/ABI/runner/corpus changes or device loss.
Cumulative 233 remains 1,677/1,630/47 with ten resolved histories; full 229/cadence two and rolling
feature 229/231/233 are unchanged. Six material compile/assembly passes are inherited; its runtime
contract remains absent. Research does not advance implementation cadence.

Parent acceptance of research 234 verified 3,246 unique evidence files and unchanged source,
artifact and registered input hashes. Independent Boolean reconstruction of all 512 input cases
matched every saved expectation and all 2,048 complete GPU output buffers. The next bounded
implementation is typed CUDA quad helper admission; its acceptance must include the full frozen
and discovery checkpoint because it will be the third implementation since full 229.
