# NVVM development handoff

Updated 2026-09-24. Read [WORKFLOW.md](WORKFLOW.md) before starting or resuming the loop.

## Current state and next action

**Slice 204 is accepted with a full checkpoint. The authorized loop continues.** The
[completed plan](plan.slice-204-texture-dimensions.md),
[five-part report](report.slice-204-texture-dimensions.md), and
[result manifest](runtime-validation.slice-204.json) contain the evidence. No next feature has started.

Slice 204 supports the existing exact non-mip texture dimension helpers for supported numeric texel
families and signed/unsigned i32 outputs. Real textures verify all nine Float32/Int32/UInt32
scalar/two-/four-lane families, the material's descriptor helper, one-texel boundaries and multiple
shapes. CUDA array-count zero is explicitly checked. Floating outputs, mip/MS queries and unrelated
fetch/sample exclusions remain rejected. Shared type lowering and provider ABI 35 are unchanged.

Next, rank the newly exposed helper-argument boundary against remaining wave transport. All four
direct complex cells now reject `call argument type: OutParam<mtlx.BSDF> ->
BorrowInOutParam<mtlx.BSDF>`. A minimal matching source path is `mx_layer_bsdf(..., out BSDF result)`
calling mutating `result.set_layer(...)`; the callee's `this` is BorrowInOutParam. The report retains
fresh diagnostics and a clearly identified inherited IR excerpt. Do not infer full material runtime
correctness or expand this slice into that independent call boundary.

## Checkpoints and evidence

Latest accepted implementation slice and last full checkpoint are **204 (RelWithDebInfo)**.
Implementation slices since that checkpoint: **0**. Slice 204 has no inherited
runtime outcomes: all preservation obligations and its addition were freshly replayed.

| Area                                           | Authoritative record                                                                                                                                                                           | Interpretation                                                                                                                    |
| ---------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------- |
| Slice 204 full checkpoint, accepted            | [Manifest](runtime-validation.slice-204.json), [frozen outcomes](census.slice-204.tsv), [discovery outcomes](discovery-census.slice-204.tsv), [report](report.slice-204-texture-dimensions.md) | 1,611 fresh runtime cells: 1,550 correct and 61 known failures. All 1,547 previous correct cells preserved, plus three additions. |
| Slice 203 accepted targeted validation         | [Manifest](runtime-validation.slice-203.json), [report](report.slice-203-undefined-sampler.md)                                                                                                 | 72 fresh cells, including three additions; preserved optimized checkpoint by exact subset plus inheritance.                       |
| Optimized transition, accepted full checkpoint | [Manifest](runtime-validation.optimized-checkpoint.json), [report](report.optimized-checkpoint.md)                                                                                             | Historical full baseline: 1,605 cells, 1,544 correct, 61 failures.                                                                |
| Slice 202, accepted                            | [Manifest](runtime-validation.slice-202.json), [report](report.slice-202-texture-descriptor-conversion.md)                                                                                     | Full Debug checkpoint; UInt64 texture descriptors, real texture oracle, native CUDA discovery normalization.                      |
| Complex support                                | `complex_corpus` in [slice-204 manifest](runtime-validation.slice-204.json), [source manifest](complex-corpus.manifest.json)                                                                   | Both NVRTC entries compile/assemble with unchanged PTX. Four direct cells reject the next helper-argument boundary. Compile-only. |

Slice-204 frozen counts are **449 / 438 / 438** correct over 452 identities; discovery counts are
**75 / 75 / 75** correct over 85 identities, for NVRTC O3 / NVVM O0 / NVVM O3. There are no missing,
duplicate or extra keys. All 1,608 previous cells preserve classifications, return codes, execution
counts and diagnostics against the optimized full checkpoint plus the slice-203 overlay/addition.
The three new dimension-fixture cells pass. No runtime outcome is inherited at this full checkpoint.
Historical healthy denominators remain 427 frozen and 72 discovery; no frozen source overlap or
previous manifest contract changed.

## Unresolved failures and next candidates

The slice-204 manifest retains all 61 unresolved runtime failures, exact modes/diagnostics/counts,
log hashes, reproduction commands and links to earlier failure history. No known failure is hidden
or reclassified by the query feature. The previous optimized ledger remains linked as provenance.

- **Complex material:** dimension queries are now independently executable; the next rejected
  OutParam-to-BorrowInOutParam helper call is recorded above. No implementation of that feature
  is included here.
- **Wave/quad coverage:** eight identities still reject in both direct modes: quad-control, five
  wave-multi workloads and two wave-rotation workloads. Shared shuffle transport remains a possible
  prerequisite for rotation.
- **Other gaps:** frozen FP8/prelude/BF16 and reference/harness limitations, plus discovery
  infrastructure/output failures remain measured. The optimized checkpoint's explanation of the
  multisample NVRTC Debug assertion versus optimized malformed declarations remains historical.
- **Material runtime/performance:** bindings, texture-object mapping, texture/LUT/material/input
  fixtures and output oracles remain absent. They still prevent full material execution and
  kernel-performance claims.

Rolling accepted history is 202 complex-driven descriptor support, 203
complex-driven undefined-sampler support, and 204 complex-driven dimension queries. Each feature
has an independent runnable fixture.

## Current working environment

- Repository: `/home/skallweit/codex/agent-sandbox/slang`, branch `nvvm-backend`, native Ubuntu 24.04.
- NVIDIA L4, SM89; driver 580.126.09; tests target SM80. Final post-validation GPU query is recorded
  in the slice-204 manifest; no device loss occurred.
- CUDA root `/usr/local/cuda-12.9`, vendor version 12.9.2; NVCC/NVRTC 12.9.86.
- Matching tools: `build/RelWithDebInfo/bin`; test libraries: `build/RelWithDebInfo/lib`.
- Provider: `build/RelWithDebInfo/bin/libslang-llvm-nvvm.so`, pinned LLVM14, ABI 35 unchanged.
- Debug artifacts remain available for assertion checks and historical comparison.
- Local `build/nvvm-setup/env.sh` selects Debug; use the optimized overrides in WORKFLOW or inspect
  `build/nvvm-loop/slice-203-env.sh` before reuse. Set `CMAKE_BUILD_PARALLEL_LEVEL=1` with explicit
  outer `--parallel 4`. This slice used four corpus workers, two unit servers and sequential suites.
- Build skill: `build/nvvm-setup/slang-skills/skills/slang-build/SKILL.md`; otherwise use the skill
  lookup and `docs/building.md` fallback. No new remote access was required.
- Local commits use the established identity with `git -c user.name='Simon Kallweit'
-c user.email='skallweit@x11-0090.cl1c1.colossus.nvidia.com'`; no push is authorized.

Slice-204 gates pass: focused 14/14, runtime 4/4, units 473/473 with one Windows-only skip and
18/18 toolkit cells. The tested base is `ff3c679909428fd963787de74abc5cfb72faa6f3` plus exact source
hashes in the manifest. Compiler SHA256 is
`cb187104da2ae3699c93d7189cd893a142b4bdc2bea52fcc9a1eb08000dedb09`; provider SHA256 is
`1f3ef9bd03de64838dc039a97ec30f4fe00cd33682d0d95b06abac895446124f`.
The byte-identical positive fixture passes NVRTC before the change, rejects in both direct modes
before, and passes all modes after. Raw evidence is under `build/nvvm-loop/slice-204-before` and
`build/nvvm-loop/slice-204-after`. The manifest rechecks final source/binary hashes after all gates.

The old A6000 GPU-loss incident remains historical in slice201. No cause was established, and no
driver was changed or reboot performed during this slice.

## Updating this handoff

The parent reviews and accepts the full checkpoint, updates the acceptance/cadence statements,
and commits the completed slice. Preserve historical evidence and unresolved failures; do not
resume beyond a maintainer stop without a new resume request.
