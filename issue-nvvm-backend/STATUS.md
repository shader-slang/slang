# NVVM development handoff

Updated 2026-09-24. Read [WORKFLOW.md](WORKFLOW.md) before starting or resuming the loop.

## Current state and next action

**Slice 206 is accepted with a full checkpoint.** Parent review verified the implementation,
exact preservation evidence and final source/binary identity. The
[completed plan](plan.slice-206-boolean-lanes.md),
[five-part report](report.slice-206-boolean-lanes.md), and
[result manifest](runtime-validation.slice-206.json) retain final evidence. No next feature has started.

Slice 206 legalizes nonescaping scalar lane reads/writes rooted in a private bool2/3/4 Var into
whole-vector value operations before direct NVVM preflight. Packed LLVM i1 vectors, physical
resource storage, address-space rules and provider ABI 35 remain unchanged. Two runnable fixtures
cover dynamic lane mutation/complete initialization and vector NaN/Inf classification with exact
independent output oracles; both fail before and pass after at NVVM O0/O3 with NVRTC O3 reference.
Shared/external and escaping lane addresses retain explicit negative coverage.

**Every registered complex cell now compiles and assembles**: eval_buffer and sample_buffer at
NVRTC O3 and NVVM O0/O3. No next complex compiler blocker was exposed. Material sources are
unchanged. Material bindings, texture/LUT/input and expected-output contracts are still absent, so
this establishes no material runtime correctness or performance claim.

Next, re-rank independent wave transport against other measured gaps.
Do not begin material execution without its application semantics. The authorized loop continues.

## Checkpoints and evidence

Latest accepted implementation and full checkpoint are 206 (RelWithDebInfo). Implementation slices
since the full checkpoint: 0.

| Area                                           | Authoritative record                                                                                                                                                                      | Interpretation                                                                                                                                 |
| ---------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------- |
| Slice 206 accepted full checkpoint             | [Manifest](runtime-validation.slice-206.json), [frozen outcomes](census.slice-206.tsv), [discovery outcomes](discovery-census.slice-206.tsv), [report](report.slice-206-boolean-lanes.md) | 1623 fresh runtime cells:1562 correct and 61 known failures. All 1617 previous exact outcomes/diagnostics preserved, plus 6 correct additions. |
| Slice 205 accepted full checkpoint             | [Manifest](runtime-validation.slice-205.json), [report](report.slice-205-reference-forwarding.md)                                                                                         | 1617 cells:1556 correct and 61 failures; baseline for 206.                                                                                     |
| Optimized transition, accepted full checkpoint | [Manifest](runtime-validation.optimized-checkpoint.json), [report](report.optimized-checkpoint.md)                                                                                        | Historical full baseline:1605 cells,1544 correct and 61 failures.                                                                              |
| Complex support                                | `complex_corpus` in [slice 206 manifest](runtime-validation.slice-206.json), [source manifest](complex-corpus.manifest.json)                                                              | All 6 compile/assemble cells pass. Compile-only.                                                                                               |

Slice 206 frozen counts are 449/438/438 correct over 452 identities; discovery counts are 79/79/79
correct over 89 identities, for NVRTC O3/NVVM O0/NVVM O3. No missing/duplicate/extra keys,
outcome/diagnostic/count deltas or inherited runtime cells. All 1556 old correct cells remain correct
and 6 additions pass. Historical healthy denominators remain 427 frozen and 72 discovery. No frozen
overlap or previous discovery contract changed.

## Unresolved failures and next candidates

The slice 206 manifest retains all 61 unresolved runtime failures with exact modes, diagnostics,
execution counts, log hashes, reproduction and prior failure history. Previous checkpoints remain
linked as provenance; no baseline reset.

- **Wave/quad coverage:** eight identities still reject in both direct modes: quad-control, five
  wave-multi workloads and two wave-rotation workloads. Shared shuffle transport remains a candidate.
- **Other gaps:** frozen FP8/prelude/BF16 and reference/harness limitations, plus discovery
  infrastructure/output failures remain measured. Historical multisample NVRTC differences between
  Debug and optimized builds remain documented by the optimized checkpoint.
- **Material runtime/performance:** application bindings and output oracle remain absent. Both
  entry points now have complete registered compile/assembly support.

Rolling accepted history is 204 dimension queries, 205 mutable forwarding, 206 Boolean lanes;
all are complex-driven with independent runnable fixtures.

## Current working environment

- Repository `/home/skallweit/codex/agent-sandbox/slang`, branch `nvvm-backend`, native Ubuntu 24.04.
- NVIDIA L4 SM89, driver 580.126.09; tests target SM80. No device loss occurred during this slice.
- CUDA root `/usr/local/cuda-12.9`, vendor 12.9.2; NVCC/NVRTC 12.9.86.
- Matching tools `build/RelWithDebInfo/bin`, libraries `build/RelWithDebInfo/lib`.
- Provider `build/RelWithDebInfo/bin/libslang-llvm-nvvm.so`, pinned LLVM 14, unchanged ABI 35.
- `build/nvvm-setup/env.sh` selects Debug; use optimized overrides from
  `build/nvvm-loop/slice-203-env.sh`. This also selects installed formatter tools.
  Set `CMAKE_BUILD_PARALLEL_LEVEL=1` with outer `--parallel 4`.
  Four corpus workers, two unit servers, sequential suites and no concurrent performance runs.
- Build skill `build/nvvm-setup/slang-skills/skills/slang-build/SKILL.md`.
- Local commits use `git -c user.name='Simon Kallweit'
-c user.email='skallweit@x11-0090.cl1c1.colossus.nvidia.com'`; no push is authorized.

Slice 206 gates pass: focused 16/16 plus escaping negatives 2/2, runtime 4/4, units 473/473 with one
Windows-only skip and 18/18 toolkit cells. Tested base is
`c17c9c92e63f55e959cf2b32504215f190b81e9e` plus exact source hashes in the manifest.
Final compiler library SHA256:
`f22b30cc8732d1794dc9ce33a8c5f7901892949dfae6c8eba574bfae30ade933`.
Provider SHA256: `1f3ef9bd03de64838dc039a97ec30f4fe00cd33682d0d95b06abac895446124f`.
Raw final evidence is under `build/nvvm-loop/slice-206-before` and `slice-206-after`.
The interrupted preformat replay is historical only under `slice-206-preformat`; the completed
final replay uses the exact formatted source and rebuilt binaries. The plan records that avoidable
interruption and a corrected diagnostic-harness annotation. No positive test contract changed.

The old A6000 GPU-loss incident remains historical in slice201; no cause was established, and no
driver change or reboot was performed here.

## Updating this handoff

Keep the next slice bounded and preserve every historical failure and evidence distinction.
The parent owns acceptance and local commits; one fresh worker owns each implementation slice.
