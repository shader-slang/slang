# NVVM development handoff

Updated 2026-09-24. Read [WORKFLOW.md](WORKFLOW.md) before starting or resuming the loop.

## Current state and next action

**Slice 202 is accepted. The loop stops after this local commit at the maintainer's request.**
There is no unfinished feature acceptance, and slice 203 has not been selected or started.
The [completed plan](plan.slice-202.md) and [report](report.slice-202-texture-descriptor-conversion.md)
record the implementation, validation, and representation audit.

Slice 202 supports exact UInt64 conversions for already-supported read-only CUDA texture
descriptors, preserving buffer descriptor rejection and provider ABI 35. The new executable
fixture checks actual floating-point/integer textures and 64-bit boundary payloads. Discovery now
normalizes an explicitly selected native CUDA contract while retaining frozen-source overlap and
duplicate rejection.

On a future explicit resume request, verify the checkout/build/device and run the small runtime
gate. Use the full current-host slice-202 records below as the accepted baseline, then rank the
next bounded candidates. The material's new `LoadFromUninitializedMemory` blocker is a candidate;
no change for that blocker is included in slice 202. A new host/toolchain still needs a full
baseline as required by WORKFLOW.

## Accepted checkpoints and evidence

| Area                               | Authoritative record                                                                                                                                                                                              | Interpretation                                                                                                                                                           |
| ---------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| Slice 202, latest accepted         | [Runtime manifest](runtime-validation.slice-202.json), [frozen outcomes](census.slice-202.tsv), [discovery outcomes](discovery-census.slice-202.tsv), [report](report.slice-202-texture-descriptor-conversion.md) | Full fresh 452 frozen + 83 discovery identities, 1,605 runtime cells. All 1,541 previous correct cells preserved; three new texture cells pass.                          |
| Current complex checkpoint         | `complex_corpus` in the [slice-202 manifest](runtime-validation.slice-202.json), [source manifest](complex-corpus.manifest.json)                                                                                  | Both NVRTC entries compile/assemble; both direct modes for both entries now reject `LoadFromUninitializedMemory`. Compile-only, not runtime proof.                       |
| Slice 201                          | [Manifest](runtime-validation.slice-201.json), [report](report.slice-201-wave-prefix-count.md)                                                                                                                    | Accepted wave-prefix support; its 61 wave identities are included in the full slice-202 replay.                                                                          |
| Historical preservation references | [Slice 195 frozen](census.slice-195.tsv), [slice 195 discovery](discovery-census.slice-195.tsv), [slice 200](runtime-validation.slice-200.json)                                                                   | Retained historical records; slice 202 compared the slice-195 outcomes plus the accepted slice-201 wave overlay and then completed a full fresh before/after comparison. |

Frozen correct counts are **449 / 438 / 438** for NVRTC O3 / NVVM O0 / NVVM O3.
Discovery correct counts are **73 / 73 / 73**, including the one added identity. The original
1,602 runtime cells retain exactly the same classifications and diagnostics; all 82 original
discovery contracts are unchanged. The 61 existing failing cells remain explicit, not passes.

Frozen selection remains 452 identities with historical healthy MVP denominator 427. Discovery
now selects 83 identities, preserving historical healthy denominator 72 and reporting the one
runnable addition separately. Historical manifests and denominators were not rewritten.

## Unresolved failures and next candidates

The slice-202 manifest's `unresolved_failures` is the fresh failure ledger: exact IDs/modes,
classifications, diagnostic/shape, historical references, current-host before-change evidence,
reproductions, and log hashes. Its `complex_corpus` separately records compile-only failures.

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
- **Performance:** this acceptance uses a Debug compiler and concurrent correctness suites.
  Timing samples establish no speedup. Use an optimized build and controlled measurements for a
  future performance slice after the relevant runtime contract is available.

Rolling accepted history: 200 was correctness/validation, 201 was runnable wave support, and
202 is a complex-driven feature with real runtime coverage. The 202-204 window therefore already
contains one complex-driven feature. Re-rank from current evidence after an explicit resume.

## Current working environment

- Repository: `/home/skallweit/codex/agent-sandbox/slang`, branch `nvvm-backend`, native Ubuntu 24.04.
- NVIDIA L4, SM89; driver 580.126.09; tests target SM80. Post-acceptance query was healthy
  (41 C, 4 MiB used).
- CUDA root `/usr/local/cuda-12.9`, vendor version 12.9.2; NVCC/NVRTC 12.9.86.
- Debug tools: `build/Debug/bin`; test libraries: `build/Debug/lib`.
- Provider: `build/Debug/bin/libslang-llvm-nvvm.so`, isolated pinned LLVM14, ABI 35 unchanged.
- Source `build/nvvm-setup/env.sh` for local paths and four-job build limit. It is ignored local
  state; WORKFLOW records equivalent explicit runtime environment settings.
- Local build skill: `build/nvvm-setup/slang-skills/skills/slang-build/SKILL.md`. If absent, follow
  skill lookup and `docs/building.md` fallback. The configured build requires no new remote access.
- Git has no configured author identity. Local commits use the existing branch identity with
  `git -c user.name='Simon Kallweit' -c user.email='skallweit@x11-0090.cl1c1.colossus.nvidia.com'`.

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
newest accepted results, and update failures, candidates, environment changes, and cadence.
Keep acceptance distinct from implementation progress. Do not resume beyond the maintainer's
recorded stop without a new resume request.
