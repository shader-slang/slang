# NVVM development handoff

Updated 2026-09-24. Read [WORKFLOW.md](WORKFLOW.md) before starting or resuming the loop.

## Current state and next action

**Ready to start in a new session; no feature loop is running.** There is no unfinished feature
acceptance. Slice 201 is accepted. This session prepared the loop as requested and stops here.
The next feature slice number is **202**; its implementation has not been selected or started.

On an explicit request to start the loop:

1. Check the checkout and environment, rebuild if needed, and run the four-fixture runtime gate.
2. Establish a full current-host baseline for the frozen, discovery, and complex corpora using
   WORKFLOW's command reference, with output under `build/nvvm-loop/slice-202-before`. Compare
   historical preservation obligations and the accepted wave snapshot; investigate differences.
   The 391 other frozen and 82 discovery identities have not yet been freshly run on this L4 host.
3. Rank a small candidate set from those results. Prefer the bounded CUDA integer-to-texture-
   descriptor feature if the refreshed evidence supports it, with runnable companion coverage.
   Write `plan.slice-202-<topic>.md`, then implement and follow the acceptance/commit loop.

Suggested new-session instruction:

> Read issue-nvvm-backend/WORKFLOW.md and STATUS.md, then start the NVVM development loop.
> Continue through accepted, committed slices until a recorded stopping condition needs my input.

## Accepted checkpoints and evidence

| Area                                           | Authoritative record                                                                                                                                                                                      | Interpretation                                                                                                                                                                 |
| ---------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| Complex corpus, committed as `63c118b8d`       | [Assessment](assessment.tiled-brass.json), [report](report.tiled-brass-assessment.md), [manifest](complex-corpus.manifest.json)                                                                           | Two material entries; NVRTC O3 compiles/assembles both; NVVM O0/O3 reject `CastUInt64ToDescriptorHandle`. Compile-only, not runtime proof.                                     |
| Slice 201, acceptance committed as `187d87148` | [Runtime manifest](runtime-validation.slice-201.json), [wave outcomes](census.slice-201-wave.tsv), [report](report.slice-201-wave-prefix-count.md), [completed plan](plan.slice-201-wave-prefix-count.md) | Fresh 61 identities / 183 cells. NVRTC 61 correct; NVVM O0/O3 each 53 correct and 8 preflight gaps. All 165 old-correct cells preserved; prefix count gains both direct modes. |
| Historical native Linux baseline               | [Slice 200 manifest](runtime-validation.slice-200.json), [report](report.slice-200-gpu-correctness.md)                                                                                                    | A6000/CUDA 13.4 results, including known failures and inherited rows. Its raw per-row files are absent on this host.                                                           |
| Historical per-identity outcomes               | [Frozen slice 195](census.slice-195.tsv), [discovery slice 195](discovery-census.slice-195.tsv)                                                                                                           | Durable comparison inputs, not fresh L4 results. Overlay the accepted wave outcomes for those 61 IDs and consult slice 200 for documented environment differences.             |

Frozen selection remains 452 identities; its historical healthy MVP denominator is 427. Discovery
currently selects 82 identities with a historical healthy denominator of 72. Those denominators
and the historical manifests were not rewritten during reconciliation. Record future runnable
additions separately; maintain each old identity's preservation obligation.

## Unresolved failures and next candidates

The linked manifests/reports are the failure ledger. Keep exact IDs/modes and reproductions there;
this list provides priorities without duplicating every diagnostic.

- **Complex material:** `CastUInt64ToDescriptorHandle` is the first observed blocker for both
  entries at both direct optimization levels. The [assessment trace](report.tiled-brass-assessment.md)
  identifies the valid standard-library constructor and NVVM preflight boundary. Find executable
  tests using real CUDA texture objects and the same typed conversion. Audit texture versus buffer
  descriptor representations; do not assume every descriptor is a scalar integer. Later material
  blockers are unknown until this one is resolved.
- **Wave/quad coverage:** eight identities still reject in both direct modes: quad-control
  functionality, five wave-multi workloads, and two wave-rotation workloads. Exact diagnostics and
  log hashes are in `runtime-validation.slice-201.json` under `known_gaps`. Shared shuffle transport
  was previously identified as a candidate prerequisite for rotation, but has not been implemented.
- **Other historical gaps:** slice 200 records frozen FP8/prelude/BF16 limitations and discovery
  infrastructure/output failures. These are unresolved historical observations, not fresh L4
  diagnoses. Reassess them during the full baseline; do not inherit the A6000's lack of SM89 support
  as a limitation of this L4 or count previously skipped cases as passes without executing them.
- **Material runtime:** host bindings, texture-object mapping, LUT/material/input fixtures and output
  oracles are still missing. They block claims about this shader's execution and generated-kernel
  performance, but do not block independently testable backend support slices.
- **Performance:** current material timings use a Debug compiler. No NVVM material speedup or code-
  quality win is established. Use an optimized build and controlled measurement when that becomes
  a slice's objective; inspect actual kernel runtime only after correctness is established.

Cadence at handoff: slice 199 was toolkit infrastructure, 200 was correctness/validation, and 201
was runnable wave support. The complex assessment is a corpus checkpoint, not a feature slice.
Begin the new three-feature window with 202 and ensure at least one complex-driven feature in
202-204 unless a documented correctness/infrastructure priority overrides it.

## Current working environment

- Repository: `/home/skallweit/codex/agent-sandbox/slang`, branch `nvvm-backend`, native Ubuntu 24.04.
- NVIDIA L4, SM89; driver 580.126.09. Tests target SM80. Post-acceptance GPU query was healthy.
- CUDA root `/usr/local/cuda-12.9`, vendor version 12.9.2; NVCC/NVRTC 12.9.86.
- Debug compiler/test tools: `build/Debug/bin`; test libraries: `build/Debug/lib`.
- Provider: `build/Debug/bin/libslang-llvm-nvvm.so`, built against isolated pinned LLVM14.
  Provider ABI remains 35. Required submodules and dependency downloads are locally present.
- Source `build/nvvm-setup/env.sh` for this host's paths and four-job build limit. It is ignored local
  state. WORKFLOW documents equivalent explicit runtime environment settings if it is absent.
- The local build skill is `build/nvvm-setup/slang-skills/skills/slang-build/SKILL.md`. If missing,
  follow repository skill lookup and `docs/building.md` fallback; do not assume temporary network
  access is still available. No remote access is required for the existing configured build.
- Git has no configured author identity on this machine. Recent commits use the existing branch
  identity via `git -c user.name='Simon Kallweit'` and
  `-c user.email='skallweit@x11-0090.cl1c1.colossus.nvidia.com'`, without global configuration changes.

Fresh slice-201 checks passed 6/6 focused CUDA lanes, 4/4 runtime fixtures, 473/473 selected unit
tests (one Windows-only skip), and 8/8 compile/assembly commands. Raw evidence is under
`build/nvvm-slice201-reconcile`; initial complex measurements are under `build/nvvm-tiled-brass`.
The old A6000 GPU-loss incident is retained in the slice-201 manifest's historical attempt; it is
not an active blocker on this host, and no cause was established or reboot performed here.

## Updating this handoff

At each checkpoint, replace the current-state/next-action section, link the active or completed
plan and newest accepted results, and update the candidate list, unresolved failures, environment
changes, and rolling three-feature cadence. Keep acceptance distinct from implementation progress.
Do not leave obsolete machine failures as active instructions or claim stale evidence was rerun.
