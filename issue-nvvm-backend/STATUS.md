# NVVM development handoff

Updated 2026-09-24. Read [WORKFLOW.md](WORKFLOW.md) before starting or resuming the loop.

## Current state and next action

**The authorized loop has resumed. The optimized configuration checkpoint is accepted.** No new feature has been
selected or started. The [completed checkpoint plan](plan.optimized-checkpoint.md),
[report](report.optimized-checkpoint.md), and
[result manifest](runtime-validation.optimized-checkpoint.json) contain the fresh evidence.

Next, rank the remaining material undefined-read/sampler-placeholder
and wave transport candidates, then delegate one bounded feature slice. The sampler case still
needs its semantic audit; diagnostic movement alone is not acceptance. Slice 202 remains the latest
accepted implementation slice, with its [plan](plan.slice-202.md) and
[report](report.slice-202-texture-descriptor-conversion.md) retained.

Slice 202 supports exact UInt64 conversions for already-supported read-only CUDA texture
descriptors, preserving buffer descriptor rejection and provider ABI 35. The new executable
fixture checks actual floating-point/integer textures and 64-bit boundary payloads. Discovery now
normalizes an explicitly selected native CUDA contract while retaining frozen-source overlap and
duplicate rejection.

The matching RelWithDebInfo compiler/provider/test tools have now completed the required full
configuration-transition validation. All 1,544 slice-202 Debug correct cells remain correct; all
61 existing failures retain their classifications. One known multisample NVRTC diagnostic changes
from a Debug assertion to rejected malformed CUDA declarations; it remains a failure. No compiler,
provider, test, runner or selection source changed.

## Accepted checkpoints and evidence

Latest accepted implementation slice: **202 (Debug)**. Last accepted full checkpoint: **RelWithDebInfo configuration transition**.
Implementation slices since the last full checkpoint: **0**. No targeted-only slices are pending
replay. This configuration checkpoint establishes optimized evidence without advancing
the implementation cadence; continue the three-slice full-checkpoint rule in WORKFLOW.

| Area                               | Authoritative record                                                                                                                                                                                                    | Interpretation                                                                                                                                                           |
| ---------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| Optimized checkpoint, accepted     | [Manifest](runtime-validation.optimized-checkpoint.json), [frozen outcomes](census.optimized-checkpoint.tsv), [discovery outcomes](discovery-census.optimized-checkpoint.tsv), [report](report.optimized-checkpoint.md) | Fresh 1,605 runtime cells; all 1,544 correct cells and 61 failures preserved; no missing or duplicate runtime/complex cells.                                             |
| Slice 202, latest accepted         | [Runtime manifest](runtime-validation.slice-202.json), [frozen outcomes](census.slice-202.tsv), [discovery outcomes](discovery-census.slice-202.tsv), [report](report.slice-202-texture-descriptor-conversion.md)       | Full fresh 452 frozen + 83 discovery identities, 1,605 runtime cells. All 1,541 previous correct cells preserved; three new texture cells pass.                          |
| Current complex checkpoint         | `complex_corpus` in the [optimized manifest](runtime-validation.optimized-checkpoint.json), [source manifest](complex-corpus.manifest.json)                                                                             | Both NVRTC entries compile/assemble; both direct modes for both entries now reject `LoadFromUninitializedMemory`. Compile-only, not runtime proof.                       |
| Slice 201                          | [Manifest](runtime-validation.slice-201.json), [report](report.slice-201-wave-prefix-count.md)                                                                                                                          | Accepted wave-prefix support; its 61 wave identities are included in the full slice-202 replay.                                                                          |
| Historical preservation references | [Slice 195 frozen](census.slice-195.tsv), [slice 195 discovery](discovery-census.slice-195.tsv), [slice 200](runtime-validation.slice-200.json)                                                                         | Retained historical records; slice 202 compared the slice-195 outcomes plus the accepted slice-201 wave overlay and then completed a full fresh before/after comparison. |

Frozen correct counts are **449 / 438 / 438** for NVRTC O3 / NVVM O0 / NVVM O3.
Discovery correct counts are **73 / 73 / 73**, including the one added identity. All 1,605 runtime cells retain their slice-202 classifications, return codes and execution counts.
The single diagnostic change is explicitly reviewed in the optimized manifest: writable multisample
textures still fail NVRTC compilation. All workload contracts and the 61 failures remain explicit.

Frozen selection remains 452 identities with historical healthy MVP denominator 427. Discovery
now selects 83 identities, preserving historical healthy denominator 72 and reporting the one
runnable addition separately. Historical manifests and denominators were not rewritten.

## Unresolved failures and next candidates

The optimized manifest's `unresolved_failures` is the fresh failure ledger, retaining slice-202
first-known evidence references plus current IDs/modes, classifications, diagnostics, reproductions
and log hashes. `diagnostic_review` explains the one configuration difference. Its `complex_corpus`
records all six fresh compile/assembly cells; the historical slice-202 record remains unchanged.

- **Complex material:** descriptor conversions are now admitted. Both entries at direct O0/O3
  stop at `LoadFromUninitializedMemory`. The retained eval-buffer IR contains a `SamplerState`
  undefined read after conversion in `render.TextureHandle.sample`, passed to
  `render.ExplicitLodSampler.sample`; SSA `readVar` / `readVarRec` produce that shape. Uninitialized
  aggregate constructor results also occur. The generic diagnostic does not identify the first
  visited occurrence. Audit CUDA sampler-placeholder semantics separately from real uninitialized
  data before selecting the next fix; the [report](report.slice-202-texture-descriptor-conversion.md)
  records the trace and its limits.
- **Wave/quad coverage:** eight identities still reject in both direct modes: quad-control,
  five wave-multi workloads, and two wave-rotation workloads. Shared shuffle transport remains
  a possible prerequisite for rotation and has not been implemented.
- **Other current gaps:** frozen FP8/prelude/BF16 and reference/harness limitations, plus discovery
  infrastructure/output failures, were freshly replayed. The multisample NVRTC compiler assertion
  predates this slice and is also documented in the slice-200 report. L4 executes the FP8 NVRTC
  references that the old SM86 A6000 skipped; direct support gaps remain recorded.
- **Material runtime:** host bindings, texture-object mapping, texture/LUT/material/input fixtures,
  and output oracles remain absent. They prevent claims about full material execution and kernel
  performance, but do not prevent independently executable backend support slices.
- **Performance:** isolated fixed-subset host compile measurements are now recorded: Debug/optimized
  median ratios 1.228 and 1.270 for two supported fixtures, with identical PTX. They establish no
  general build or GPU kernel speedup. Material runtime/performance still needs the missing contract.

Rolling accepted history: 200 was correctness/validation, 201 was runnable wave support, and
202 is a complex-driven feature with real runtime coverage. The 202-204 window therefore already
contains one complex-driven feature. Re-rank from the accepted configuration checkpoint.

## Current working environment

- Repository: `/home/skallweit/codex/agent-sandbox/slang`, branch `nvvm-backend`, native Ubuntu 24.04.
- NVIDIA L4, SM89; driver 580.126.09; tests target SM80. Post-validation query was healthy
  (36 C, 4 MiB used).
- CUDA root `/usr/local/cuda-12.9`, vendor version 12.9.2; NVCC/NVRTC 12.9.86.
- Current tools: `build/RelWithDebInfo/bin`; test libraries: `build/RelWithDebInfo/lib`.
- Provider: `build/RelWithDebInfo/bin/libslang-llvm-nvvm.so`, isolated pinned LLVM14, ABI 35 unchanged.
- Debug artifacts remain available for assertion-focused checks and historical comparison.
- The ignored `build/nvvm-setup/env.sh` currently selects Debug and a four-job build limit. Inspect
  and override its compiler/provider/test paths for `RelWithDebInfo` before reuse; WORKFLOW records
  the default command examples. Optimized binaries are now fully validated. Suites ran sequentially
  with four corpus workers and two unit servers. The first root build may briefly have exceeded
  four compile processes through its two-object provider child; actual overlap was not measured.
  Future builds must set `CMAKE_BUILD_PARALLEL_LEVEL=1` with explicit outer `--parallel 4` to prevent
  nested oversubscription.
- Local build skill: `build/nvvm-setup/slang-skills/skills/slang-build/SKILL.md`. If absent, follow
  skill lookup and `docs/building.md` fallback. The configured build requires no new remote access.
- Git has no configured author identity. Local commits use the existing branch identity with
  `git -c user.name='Simon Kallweit' -c user.email='skallweit@x11-0090.cl1c1.colossus.nvidia.com'`.

Optimized gates passed: 4/4 runtime fixtures, 473/473 selected units with one Windows-only skip,
and 18/18 toolkit cells. Both complex NVRTC entries assembled; all four direct cells still reject
`LoadFromUninitializedMemory`. Raw evidence is under
`build/nvvm-loop/optimized-checkpoint-20260924`; tested source revision is
`ecfacff50002bd9b60f5250b56a2023a399581e5`. The manifest records all source/binary/toolkit hashes.

Additional slice-202 gates passed: 8/8 focused checks, 4/4 runtime fixtures, 473/473 selected units
with one Windows-only skip, 18/18 toolkit cells, six focused compiler/assembler commands, and four
runner regression tests. Raw before/after evidence is under `build/nvvm-loop/slice-202-before`
and `build/nvvm-loop/slice-202-after`; exploratory fixture evidence is under
`build/nvvm-loop/descriptor-probe`. Tested base revision and exact source/binary/toolkit hashes
are in the manifest; the accepted slice is the commit containing this handoff.

The old A6000 GPU-loss incident remains historical in the slice-201 manifest. No cause was
established, no driver was changed, and no reboot was performed in this slice.

## Updating this handoff

At each checkpoint, replace the current state and next action, link the active/completed plan and
newest accepted results, and update failures, candidates, environment changes, and cadence. Name
the last full checkpoint and count slices since it separately from the latest targeted acceptance.
Keep acceptance distinct from implementation progress. Do not resume beyond the maintainer's
recorded stop without a new resume request.
