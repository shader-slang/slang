# NVVM development handoff

Updated 2026-09-25. Read [WORKFLOW.md](WORKFLOW.md) before starting or resuming the loop.

## Current state and next action

**Slice 232 research is accepted.** Read the
[completed plan](plan.slice-232-fp16-minmax.md), [five-part report](report.slice-232-fp16-minmax.md)
and [semantic evidence](semantic-evidence.slice-232.json). Production and registered results are
unchanged. [Slice 231 targeted acceptance](runtime-validation.slice-231.json) remains authoritative.

**Next action: a bounded FP16 MIN/MAX emitter implementation based on this contract.** The source
uses ordered raw half compare/select and finite exclusive seeds `0x7bff`/`0xfbff`, not infinities.
Admit only FP16 reductions and inclusive/exclusive prefixes through existing scalar/aggregate leaf
recipes. Half read/compare/SELECT/constants already pass source and direct controls; numeric half
MIN/MAX descriptors remain excluded. Replay unchanged research inputs/expectations at direct O0/O3
and preserve neighboring operation/capability boundaries. No implementation starts in slice 232.
The four original frozen prefix cells remain E52017 failures; matrix prefixes remain E36100/E36107.

Latest accepted implementation and targeted acceptance: 231. Latest full checkpoint: 229.
Implementation slices since full: one. Research 232 does not advance cadence. Rolling feature
history: 227 FP64 prefixes, 229 narrow integer MIN/MAX, 231 64-bit integer MIN/MAX; research232
establishes the next FP16 contract. Research-backed correctness takes priority while the material
application contract is absent; reconsider material work each slice. No push is authorized.

## Checkpoints and evidence

- [Research 232](semantic-evidence.slice-232.json): 5,912 exact GPU launches (1,478 source family,
  4,434 source/direct controls), 6,053,888 output words; 40 direct E52017 rejections, 12 matrix-prefix
  capability rejections, four PTX assemblies and fresh smoke4. Each mode uses 69 structured patterns
  under 14 masks plus 512 full-mask batches covering every binary16 encoding among lane/components.
  This is not exhaustive tuples or mask/caller combinations. All 29 source, 12 artifact and 554
  registered input hashes remain unchanged. Registered/unit/toolkit/material evidence below is
  inherited explicitly. Source/diagnostic/IR/binary evidence lives under
  `build/nvvm-loop/slice-232-fp16-minmax`; no provider/ABI or corpus changes.

- [Full checkpoint 229](runtime-validation.slice-229.json): 1,671 cells, 1,620 correct and 51 known
  failures; six resolved histories. Frozen 452 identities/1,356 cells and discovery 105/315.
- [Accepted research 230](semantic-evidence.slice-230.json): 2,128 source MIN/MAX launches and
  6,384 typed controls, all exact. Its 225 referenced raw artifacts remain untouched.
- [Slice 231 targeted](runtime-validation.slice-231.json): 639 fresh and 1,035 inherited cells;
  cumulative 1,674 cells, 1,623 correct and 51 known failures. Exactly three fixture cells are
  additions. [Frozen census](census.slice-231.tsv); [discovery census](discovery-census.slice-231.tsv).
  Targeted frozen 107 identities/321 cells has completed:
  315 correct, six preflight stops and exactly four diagnostic-only transitions above. All other
  five-field outcomes are unchanged. The remaining 1,035 frozen cells inherit full 229 explicitly.
- Discovery 106 identities/318 cells has 288 correct, 22 infrastructure, four mismatch and four
  preflight outcomes. All 315 old outcomes preserve all five fields exactly. No missing or duplicate
  cells. All 51 first-known records and six resolved histories remain; four prefix diagnostic
  histories append the prior failure and new evidence.
- The new fixture passes all three modes. It uses a word-wise unsigned comparison oracle with a
  sign-bit bias, independent of 64-bit min/max and wave operations. The identical final fixture
  passed NVRTC and rejected both direct modes on the accepted compiler before production edits.
- Research replay passes 12,768 launches and 28,600,320 output words: 6,384 MIN/MAX family and
  6,384 typed controls. Saved source/input/expectation/output hashes preserve accepted launches;
  independent reconstruction verifies every input and expectation. All 12 runtime PTX artifacts
  assemble; all 80 minimal direct family probes now compile. Research launches are separate from
  registered runtime cells.
- Fresh smoke 4/4, units 479/479 plus one existing Windows-only skip, toolkit 18/18 and discovery
  contracts 6/6 and all six material compile/assembly cells pass. All 92 excluded operation signatures and 14 ordinary aggregate shuffle
  signatures retain their before/after E52017 rejections; 24 matrix prefix capability cells remain.

All 553 old registered input hashes and every old discovery row remain unchanged; the new fixture
adds one identity/three cells and input 554. Historical healthy denominators 427/72 stay fixed.
Known failure first observations, reproduction commands, complete diagnostics and histories remain
part of the result ledger; no baseline reset or expected-output reduction is permitted.

## Unresolved failures and limitations

- FP16 masked MIN/MAX prefixes remain the next frozen diagnostic boundary. Research232 establishes
  the ordered source/finite-seed contract; implementation and direct family runtime proof remain.
- Arithmetic/bitwise families, matrix prefix capability, quad reconvergence, ordinary FP64
  vector-by-value shuffles and resource-bearing contexts remain separate.
- Hardware masks remain scheduling-dependent; logical active-mask synthesis is unchanged.
- FP8/BF16/prelude and existing discovery infrastructure/output gaps remain visible.
- Material runtime remains blocked by application bindings, texture/LUT/input and output oracle.
  Its six registered cells are compile/assembly checks only; no runtime or performance claim.
- Slice 214 batching remains opt-in; no new performance claim.

## Environment and final evidence

Native Ubuntu 24.04, branch `nvvm-backend`, repository `/home/skallweit/codex/agent-sandbox/slang`.
L4 SM89, driver 580.126.09, target SM80, CUDA 12.9.2/NVRTC 12.9.86, LLVM 14, provider ABI 36.
Use matching optimized `build/RelWithDebInfo/{bin,lib}`, source
`build/nvvm-loop/slice-203-env.sh`, and local `build/nvvm-setup/slang-skills/skills/slang-build/SKILL.md`.
Sequential GPU suites, at most four CPU workers, two unit servers; `CMAKE_BUILD_PARALLEL_LEVEL=1`.

Slice 231 tested base `51adf2c8c6ab9c61de4e87dfcc187ffca865d126` plus the recorded emitter patch.
Final compiler-library SHA-256:
`948d300ec9f21f9000d242fcd83ee109a97ec63e1a768511553bc64c35b21c08`.
Provider remains `ae0e7859f6062a69f191cace4ae8d96340c074815866808c5b43467f41144372`.
Before compiler library: `577c8eea1039a9b6090db4bfd993bc143f53bb78d5c75f9d4d26b4753e8a11af`.
Every gate captures all 28 accepted source hashes, 12 artifacts and the final fixture hash.
Only emitter and discovery registration change among those source identities; the new fixture is
recorded additionally. Both source_commit and source_revision record the actual tested base.
All final source/artifact/input hashes were rechecked after the gates. Raw evidence lives in `build/nvvm-loop/slice-231-before` and
`slice-231-after`. No GPU loss, driver change, reboot or push.

Parent acceptance verified 525 unique evidence references, 29 source and 12 artifact hashes, all
554 current inputs, exact fresh/inherited census outcomes, full failure histories and all 12,768
semantic replay launches.

Research232 tested unchanged accepted base `94209dfd6a3dc1a7ac729c0c0a928a598d570d58` in both
source revision fields. No rebuild occurred. Fresh controls preserve all binary16 input encodings,
including signaling NaNs. Smoke passed after the already-started sequential research suite; the
workflow ordering deviation and one auxiliary Python extraction error are retained in the report.
No shader fixture failure, existing-operation mismatch, device loss or system change occurred.

Parent acceptance of research 232 verified 130 unique compact evidence references and all 17,736
raw launch binaries. An independent integer-unit binary16 oracle reconstructed every input and
expected output and matched all 5,912 GPU launches. Source/artifact/input identities remain unchanged.
