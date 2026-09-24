# NVVM development handoff

Updated 2026-09-24. Read [WORKFLOW.md](WORKFLOW.md) before starting or resuming the loop.

## Current state and next action

**Slice 203 is accepted with targeted validation. The authorized loop continues.** The [completed plan](plan.slice-203-sampler-undefined.md),
[five-part report](report.slice-203-undefined-sampler.md), and
[result manifest](runtime-validation.slice-203.json) contain the evidence. No next feature has started.

Slice 203 supports canonical undefined ordinary CUDA sampler placeholders through the existing
chosen-value path. Real gradient and zero textures verify the material's descriptor, branch-join
and noinline sampling-helper boundary. Undefined real resources/resource aggregates and comparison
sampler helper values remain rejected. Shared type lowering and provider ABI 35 are unchanged.

Next, rank the newly exposed integer-texture GetDimensions
helper against remaining wave transport. All four direct complex cells now reject
`GenericAsm ... txq.width.b32 ... txq.height.b32 ...`, with signature
`Void(Texture2D, OutParam<int>, OutParam<int>)`. The minimal trace is
`render.TextureHandle.resolve_udim` constructing a `Texture2D<uint>` descriptor and passing
`dim.x`/`dim.y` to `GetDimensions`. The report retains exact diagnostics. Do not infer full material
runtime correctness or expand this slice into the independent query blocker.

## Accepted checkpoints and evidence

Latest accepted implementation slice is **203 (RelWithDebInfo, targeted)**. Last accepted full checkpoint is the
**RelWithDebInfo configuration transition**. Implementation slices since that checkpoint: **1**. No full replay is claimed for this bounded compiler-local
admission change; two further implementation slices may be accepted before the cadence checkpoint.

| Area                                | Authoritative record                                                                                                                                                                                                    | Interpretation                                                                                                                    |
| ----------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------- |
| Slice 203 targeted acceptance       | [Manifest](runtime-validation.slice-203.json), [frozen subset](census.slice-203-targeted.tsv), [discovery subset](discovery-census.slice-203-targeted.tsv), [report](report.slice-203-undefined-sampler.md)             | 72 fresh cells: 69 old cells preserve all outcomes/diagnostics and three additions pass. Seven selected known failures retained.  |
| Optimized full checkpoint, accepted | [Manifest](runtime-validation.optimized-checkpoint.json), [frozen outcomes](census.optimized-checkpoint.tsv), [discovery outcomes](discovery-census.optimized-checkpoint.tsv), [report](report.optimized-checkpoint.md) | 1,605 runtime cells: 1,544 correct, 61 known failures. Slice 203 inherits 1,536 of these cells without replay.                    |
| Slice 202, accepted                 | [Manifest](runtime-validation.slice-202.json), [report](report.slice-202-texture-descriptor-conversion.md)                                                                                                              | Full Debug checkpoint; exact UInt64 texture-descriptor conversions, real texture oracle, native CUDA discovery normalization.     |
| Current complex support evidence    | `complex_corpus` in [slice-203 manifest](runtime-validation.slice-203.json), [source manifest](complex-corpus.manifest.json)                                                                                            | Both NVRTC entries compile/assemble; four direct entries/modes now reject integer-texture GetDimensions GenericAsm. Compile-only. |
| Historical references               | [Slice 201](runtime-validation.slice-201.json), [slice 200](runtime-validation.slice-200.json), [slice 195 frozen](census.slice-195.tsv), [slice 195 discovery](discovery-census.slice-195.tsv)                         | Preserved earlier history and first-known failure evidence; not replaced by a smaller targeted baseline.                          |

Fresh slice-203 frozen subset correct counts are **15 / 16 / 16** across 16 identities; discovery
subset counts are **6 / 6 / 6** across eight identities including the addition, for NVRTC O3 / NVVM
O0 / NVVM O3. Exact keys have no omissions or duplicates. The 69 old cells include 62 correct and
seven failures, all unchanged. The remaining **1,536 old cells** (1,482 correct, 54 failures) are
explicitly inherited. Combined preservation obligations therefore remain all **1,544 old correct
cells and 61 failures**, with **three new correct cells** tracked separately.

The last full checkpoint's frozen counts remain 449/438/438 over 452 identities, and discovery
counts 73/73/73 over 83 identities. The addition brings the authoritative discovery selection to
84 identities. Historical healthy denominators remain 427 and 72; no frozen source overlap or old
manifest contract changes occurred. These full counts are historical, not a fresh slice-203 replay.

## Unresolved failures and next candidates

The optimized checkpoint's `unresolved_failures` remains the full failure ledger. Slice 203
references it and additionally records all seven freshly replayed failures with exact IDs/modes,
classifications, execution counts, diagnostics and log hashes: frozen texture-subscript NVRTC
infrastructure; discovery texture-get-dimensions NVRTC mismatch and direct preflight failures;
and discovery multisample texture NVRTC infrastructure and direct preflight failures. No failure
or historical preservation obligation was removed.

- **Complex material:** ordinary sampler undefined values are now accepted. Both direct modes for
  both entries stop at the exact integer Texture2D GetDimensions helper described above. Retained
  undefined constructor aggregates contain only numeric and Boolean fields and already belong to
  the copyable domain; resource-containing undefined aggregates remain unsupported separately.
- **Wave/quad coverage:** eight identities still reject in both direct modes: quad-control, five
  wave-multi workloads and two wave-rotation workloads. Shared shuffle transport remains a possible
  prerequisite for rotation, deferred from this material-driven slice.
- **Other gaps:** frozen FP8/prelude/BF16 and reference/harness limitations, plus discovery
  infrastructure/output failures remain in the full optimized ledger. The known multisample NVRTC
  Debug assertion becomes rejected malformed CUDA declarations in RelWithDebInfo; the optimized
  checkpoint's diagnostic review preserves that prior difference.
- **Material runtime/performance:** bindings, texture-object mapping, texture/LUT/material/input
  fixtures and output oracles remain absent. This still prevents full material execution and kernel
  performance claims. Slice 203 makes no new performance claim.

Rolling accepted history is 201 wave support, 202 complex-driven texture descriptor support, and
203 complex-driven undefined sampler support. Both 202 and 203 are independently runnable.

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
  the default command examples. The optimized configuration has a full checkpoint; slice 203 has targeted acceptance. Suites ran sequentially
  with four corpus workers and two unit servers. The first root build may briefly have exceeded
  four compile processes through its two-object provider child; actual overlap was not measured.
  Future builds must set `CMAKE_BUILD_PARALLEL_LEVEL=1` with explicit outer `--parallel 4` to prevent
  nested oversubscription.
- Local build skill: `build/nvvm-setup/slang-skills/skills/slang-build/SKILL.md`. If absent, follow
  skill lookup and `docs/building.md` fallback. The configured build requires no new remote access.
- Git has no configured author identity. Local commits use the existing branch identity with
  `git -c user.name='Simon Kallweit' -c user.email='skallweit@x11-0090.cl1c1.colossus.nvidia.com'`.

Slice-203 final gates passed: 17/17 focused checks, 4/4 runtime fixtures, 473/473 selected units
with one Windows-only skip and 18/18 toolkit cells. The final tested base is
`60e2277f1522fd64a062960b43471d8a4ef33423` plus hashes in the slice-203 manifest. Compiler library
SHA256 is `fe80f943831841eb419cf2317037d7d83f57339545fb854581ca0175b513bb57`; provider hash is unchanged.
The literal revert drill reproduced the prior compiler hashes and failed only the two direct
positive lanes against byte-identical final fixtures. Raw evidence is under
`build/nvvm-loop/slice-203-before` and `build/nvvm-loop/slice-203-after`; GPU remained healthy.

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
