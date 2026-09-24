# NVVM development handoff

Updated 2026-09-24. Read [WORKFLOW.md](WORKFLOW.md) before starting or resuming the loop.

## Current state and next action

**Slice 205 is accepted with a full checkpoint. The authorized loop continues.** The
[completed plan](plan.slice-205-reference-forwarding.md),
[five-part report](report.slice-205-reference-forwarding.md), and
[result manifest](runtime-validation.slice-205.json) retain the evidence. No next feature has started.

Slice 205 forwards canonical `OutParam<T>` and `BorrowInOutParam<T>` between mutable helper
parameters within the existing local copyable/helper storage domain. Exact pointee equality,
physical storage, access, layouts, address-space conversions and provider ABI 35 remain unchanged.
Two independently runnable fixtures cover nested numeric aggregates and pointer-bearing helper
aggregates, each failing before and passing afterward at NVVM O0/O3 with NVRTC O3 reference output.

Both `sample_buffer` direct complex cells now compile and assemble. Both `eval_buffer` direct cells
reject `sequential element pointer: Ptr<bool, addressSpace=2147483647, access=0, operands=4,
layout=ScalarLayout>`. The minimal matching path is the eval code's vector `isnan`/`isinf` checks;
retained slice203 IR corroborates their boolean-lane stores. Fresh diagnostics and inherited trace
are distinguished in the manifest. Rank this boundary against remaining wave transport; do not infer full material runtime correctness from compile/assembly support.

## Checkpoints and evidence

Latest accepted implementation and last full checkpoint are **205 (RelWithDebInfo)**.
Implementation slices since that checkpoint: **0**. Shared helper-call admission received a full
fresh replay before acceptance.

| Area                                           | Authoritative record                                                                                                                                                                             | Interpretation                                                                                                                                           |
| ---------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ | -------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Slice 205 full checkpoint, accepted            | [Manifest](runtime-validation.slice-205.json), [frozen outcomes](census.slice-205.tsv), [discovery outcomes](discovery-census.slice-205.tsv), [report](report.slice-205-reference-forwarding.md) | 1,617 fresh runtime cells: 1,556 correct and 61 known failures. All 1,611 previous exact outcomes and diagnostics preserved, plus six correct additions. |
| Slice 204 accepted full checkpoint             | [Manifest](runtime-validation.slice-204.json), [report](report.slice-204-texture-dimensions.md)                                                                                                  | 1,611 cells: 1,550 correct and 61 failures; baseline for 205.                                                                                            |
| Optimized transition, accepted full checkpoint | [Manifest](runtime-validation.optimized-checkpoint.json), [report](report.optimized-checkpoint.md)                                                                                               | Historical full baseline: 1,605 cells, 1,544 correct and 61 failures.                                                                                    |
| Complex support                                | `complex_corpus` in [slice-205 manifest](runtime-validation.slice-205.json), [source manifest](complex-corpus.manifest.json)                                                                     | Four cells compile/assemble: both NVRTC entries and both NVVM sample entries. Two NVVM eval cells reject boolean element pointers. Compile-only.         |

Slice-205 frozen counts are **449 / 438 / 438** correct over 452 identities; discovery counts are
**77 / 77 / 77** correct over 87 identities, for NVRTC O3 / NVVM O0 / NVVM O3. No missing, duplicate
or extra keys, outcome/diagnostic/count deltas or inherited runtime cells. All 1,550 previous correct
cells remain correct and six new cells pass. Historical healthy denominators remain 427 frozen
and 72 discovery. No frozen overlap or previous discovery contract changed.

## Unresolved failures and next candidates

The slice-205 manifest retains all 61 unresolved runtime failures with exact modes, diagnostics,
execution counts, log hashes, reproduction commands and prior failure history. The previous
checkpoint remains linked as provenance.

- **Complex material:** mutable forwarding is now independently executable; vector predicate
  helper boolean-element pointers are the minimal next eval blocker. Sample support is compile-only.
- **Wave/quad coverage:** eight identities still reject in both direct modes: quad-control, five
  wave-multi workloads and two wave-rotation workloads. Shared shuffle transport remains a candidate.
- **Other gaps:** frozen FP8/prelude/BF16 and reference/harness limitations, plus discovery
  infrastructure/output failures remain measured. Historical multisample NVRTC differences between
  Debug and optimized builds remain documented by the optimized checkpoint.
- **Material runtime/performance:** bindings, texture-object mapping, texture/LUT/material/input
  fixtures and output oracles remain absent. They prevent full material execution and performance
  claims, including for the newly compiling sample entry.

Rolling accepted history is 202 descriptor support, 203 undefined samplers, 204 dimension queries;
all are complex-driven with independent runnable fixtures. Pending slice 205 is also complex-driven.
After its acceptance the rolling three are 203, 204, 205.

## Current working environment

- Repository `/home/skallweit/codex/agent-sandbox/slang`, branch `nvvm-backend`, native Ubuntu 24.04.
- NVIDIA L4 SM89, driver 580.126.09; tests target SM80. No device loss occurred during this slice.
- CUDA root `/usr/local/cuda-12.9`, vendor 12.9.2; NVCC/NVRTC 12.9.86.
- Matching tools `build/RelWithDebInfo/bin`, libraries `build/RelWithDebInfo/lib`.
- Provider `build/RelWithDebInfo/bin/libslang-llvm-nvvm.so`, pinned LLVM14, unchanged ABI 35.
- `build/nvvm-setup/env.sh` selects Debug; source optimized overrides from
  `build/nvvm-loop/slice-203-env.sh`. Set `CMAKE_BUILD_PARALLEL_LEVEL=1` with outer `--parallel 4`.
  Use four corpus workers, two unit servers, sequential suites and no concurrent performance runs.
- Build skill `build/nvvm-setup/slang-skills/skills/slang-build/SKILL.md`.
- Local commits use `git -c user.name='Simon Kallweit'
-c user.email='skallweit@x11-0090.cl1c1.colossus.nvidia.com'`; no push is authorized.

Slice-205 gates pass: focused 14/14, runtime 4/4, units 473/473 with one Windows-only skip and
18/18 toolkit cells. Tested base is `dd30f64a7672f345c3f47a0c1b434d5cb3debb28` plus exact source
hashes in the manifest. Final compiler SHA256: `6aac36c1b7068c9f54e537a9c373a267151e902bc42908784ab1fac3a4c95892`.
Provider SHA256: `1f3ef9bd03de64838dc039a97ec30f4fe00cd33682d0d95b06abac895446124f`.
Raw logs, commands, IR and identity audits are under `build/nvvm-loop/slice-205-before` and
`build/nvvm-loop/slice-205-after`. The final manifest rechecks source/tool hashes after every gate.

The old A6000 GPU-loss incident remains historical in slice201; no cause was established, and no
driver change or reboot was performed here.

## Updating this handoff

The parent reviews exact preservation evidence and the final diff, accepts the checkpoint and
updates acceptance/cadence before the local commit. Do not resume beyond a maintainer stop without
an explicit resume request. Preserve every historical failure and evidence distinction.
