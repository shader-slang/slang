# NVVM development handoff

Updated 2026-09-24. Read [WORKFLOW.md](WORKFLOW.md) before starting or resuming the loop.

## Current state and next action

**Slice 207 is accepted with a full checkpoint.** Parent review verified the implementation,
exact corpus transitions, lower-target compatibility evidence and final source/binary identity. Read the
[completed plan](plan.slice-207-wave-rotation.md),
[five-part report](report.slice-207-wave-rotation.md), and
[result manifest](runtime-validation.slice-207.json).

Both unchanged frozen rotation sources now execute correctly at NVVM O0/O3. At CUDA SM7+, scalar
rotation composes the existing tagged indexed shuffle; vector rotation applies it componentwise.
The provider transports bool, narrow integers, half and 64-bit scalar payloads through native
32-bit shuffle words, preserving exact bits. Existing 32-bit emission, operation IDs and ABI 35
remain unchanged. Original lower-target CUDA branches preserve the public SM5 capability.

Two independent per-lane fixtures pass NVRTC O3/NVVM O0/O3. They cover every vector component,
wraparound/cluster boundaries, signed and unsigned widths, Boolean patterns, high 64-bit words,
floating signed zero/NaN payloads and valid partial masks. Three malformed semantic signatures
reject before provider discovery. Wrong actual LLVM operands leave a real provider module
byte-identical to its clean control.

All six registered material cells still compile and assemble. Material bindings, texture/LUT/input
and expected-output contracts remain absent, so this establishes no material runtime correctness
or performance claim. Next, re-rank the six remaining wave/quad identities
against the other measured gaps. No next independent feature has started; the authorized loop continues.

## Checkpoints and evidence

Latest accepted implementation and full checkpoint: 207 (RelWithDebInfo). Implementation slices
since the full checkpoint: 0.

| Area                                           | Authoritative record                                                                                                                                                                      | Interpretation                                                                                                                                           |
| ---------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Slice 207 accepted full checkpoint             | [Manifest](runtime-validation.slice-207.json), [frozen outcomes](census.slice-207.tsv), [discovery outcomes](discovery-census.slice-207.tsv), [report](report.slice-207-wave-rotation.md) | 1629 fresh runtime cells: 1572 correct, 57 retained known failures. Four expected old-cell fixes and six correct additions; no other exact-field deltas. |
| Slice 206 accepted full checkpoint             | [Manifest](runtime-validation.slice-206.json), [report](report.slice-206-boolean-lanes.md)                                                                                                | 1623 cells: 1562 correct and 61 failures; exact baseline for 207.                                                                                        |
| Optimized transition, accepted full checkpoint | [Manifest](runtime-validation.optimized-checkpoint.json), [report](report.optimized-checkpoint.md)                                                                                        | Historical baseline: 1605 cells, 1544 correct and 61 failures.                                                                                           |
| Complex support                                | `complex_corpus` in [slice 207 manifest](runtime-validation.slice-207.json), [source manifest](complex-corpus.manifest.json)                                                              | All 6 compile/assemble cells pass. Compile-only.                                                                                                         |

Slice 207 frozen counts are 449/440/440 correct over 452 identities; discovery counts are 81/81/81
correct over 91 identities, for NVRTC O3/NVVM O0/NVVM O3. All 1562 previous correct cells remain
correct. Exact keys, classifications, return codes, full execution counts, diagnostics and canonical
shapes match accepted 206 except the four expected rotation fixes. No missing/duplicate/extra old
keys or inherited runtime cells. Six additions pass separately. Historical healthy denominators
remain 427 frozen and 72 discovery. No frozen source or old discovery contract changed.

## Unresolved failures and next candidates

The slice 207 manifest retains all 57 unresolved runtime failures with exact modes, diagnostics,
execution counts, log hashes, reproduction and prior failure history. The four resolved rotation
cells remain linked through baseline and exact transition records; no baseline reset.

- **Wave/quad coverage:** six identities still reject in both direct modes: quad-control and five
  wave-multi workloads. Reconvergence and partitioned reductions remain separate features.
- **Other gaps:** frozen FP8/prelude/BF16 and reference/harness limitations, plus discovery
  infrastructure/output failures remain measured. Historical multisample NVRTC differences between
  Debug and optimized builds remain documented by the optimized checkpoint.
- **Legacy lower-target probes:** integer scalar64/vector4 rotation and clustered rotation pass
  four source-call-site tests and four actual SM50/SM60 NVRTC/PTX assembly probes. The adapter's
  established SM75 clamp requires an explicit downstream architecture override. A broader frozen
  source probe at explicit SM50 exposes unavailable half-math declarations in the unchanged CUDA
  prelude; this exploratory limitation is retained separately in the manifest, not counted as a
  pass or mixed into the registered SM80 runtime ledger.
- **Material runtime/performance:** application bindings and output oracle remain absent.

Rolling accepted history is 205 mutable forwarding, 206 Boolean lanes and 207 wave rotation.
Slices 205/206 are complex-driven and 207 is wave-driven, satisfying the complex-workload cadence.

## Current working environment

- Repository `/home/skallweit/codex/agent-sandbox/slang`, branch `nvvm-backend`, native Ubuntu 24.04.
- NVIDIA L4 SM89, driver 580.126.09; runtime tests target SM80. No device loss occurred.
- CUDA root `/usr/local/cuda-12.9`, vendor 12.9.2; NVCC/NVRTC 12.9.86.
- Matching tools `build/RelWithDebInfo/bin`, libraries `build/RelWithDebInfo/lib`.
- Provider `build/RelWithDebInfo/bin/libslang-llvm-nvvm.so`, pinned LLVM 14, unchanged ABI 35.
- `build/nvvm-setup/env.sh` selects Debug; use optimized overrides from
  `build/nvvm-loop/slice-203-env.sh`. Set `CMAKE_BUILD_PARALLEL_LEVEL=1` with outer `--parallel 4`.
  Four corpus workers, two unit servers, sequential suites and no concurrent performance runs.
- Build skill `build/nvvm-setup/slang-skills/skills/slang-build/SKILL.md`.
- Parent local commits use `git -c user.name='Simon Kallweit'
-c user.email='skallweit@x11-0090.cl1c1.colossus.nvidia.com'`; no push is authorized.

Slice 207 gates pass: focused 8/8, runtime 4/4, units 474/474 with one Windows-only skip,
18/18 toolkit cells, all six complex cells, full frozen/discovery, four legacy source checks and
four actual lower-target compile/assembly probes. Tested base is
`1214f6b4d969ee0a5395dce748a5d460899eac92` plus exact source hashes in the manifest.
Compiler library SHA256: `a4ffa6b02d200436875e148cf87ee5e2553c4bad02e02951d322562e8314efd9`.
Provider SHA256: `6da3abff11e6b67a1dcd5e3b12f341bc7cbfa356fd29c9d7b5dcad8e20efd676`.
Raw evidence is under `build/nvvm-loop/slice-207-before` and `slice-207-after`.
Both final runtime fixture hashes match the before runs; final source/artifact hashes match after
the gates. The compile-only legacy fixture was added afterward and validated independently without
changing compiler/provider binaries or the runtime selection.

The old A6000 GPU-loss incident remains historical in slice 201; no cause was established, and no
driver change or reboot was performed here.

## Updating this handoff

Keep the next slice bounded and preserve every historical failure and evidence distinction.
The parent owns acceptance and local commits; one fresh worker owns each implementation slice.
