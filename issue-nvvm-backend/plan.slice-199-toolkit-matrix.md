# Slice 199: Automate required CUDA toolkit and architecture validation

This ExecPlan follows `.agent/PLANS.md` and remains a working log until the slice is complete.

## Purpose and Observable Result

Run one reproducible compilation and assembly gate against a selected CUDA Toolkit. Every selected
shader, architecture, and optimization level must produce PTX and an assembled cubin. Publish a JSON
record for every required cell, and fail missing tools, artifacts, or unsuccessful cells. A manual
GitHub Actions workflow runs CUDA 12.9 with SM70/80/90 and CUDA 13.4 with SM80/90 without a GPU.

## Progress

- [x] 2026-09-22: Read the prior validation evidence, workflow conventions, and ExecPlan contract.
- [x] 2026-09-22: Implemented the strict runner with complete cells, exact tools, and hashes.
- [x] 2026-09-22: Added the manual matrix with verified, digest-pinned NVIDIA container tags.
- [x] 2026-09-22: CUDA 13.4.2 passed 36/36; isolated CUDA 12.9.2 passed 54/54. Negative gates, Python/YAML syntax, actionlint 1.7.12, and formatting passed.
- [x] 2026-09-22: Completed the five-part report with commands, counts, and undispatched-CI limitation.

## Surprises and Discoveries

The historical measurement and census runners hardcode Windows Release paths and SM70 defaults.
The previous Linux compile/assembly evidence lives only under `build/`. An initial exact-file provider override exposed the loader issue addressed by slice 198; no directory workaround remains in this runner. Existing GPU infrastructure
is unnecessary for this gate: NVVM compilation and ptxas assembly work without a CUDA device.

## Decision Log

- 2026-09-22, Codex: Keep a small fixed shader set in the runner so matrix identity and expected
  cardinality are explicit. Revisit when adding a workload requires a new semantic family.
- 2026-09-22, Codex: Require explicitly selected architectures; never silently replace an unsupported
  target or turn an infrastructure failure into a skip. CUDA 13's matrix intentionally excludes SM70.
- 2026-09-22, Codex: Introduce workflow_dispatch first. A checked-in workflow is runnable automation,
  but its remote results are not claimed until actually dispatched.

- 2026-09-22, Codex: Verified public NVIDIA tag metadata; pin 12.9.1 and 13.4.1 images by digest because 13.4.2 development tags do not exist. Locally extracted 12.9.2 packages supplied additional direct matrix evidence.

## Outcomes and Retrospective

The final exact-file provider runner passed all 90 local compile/assembly cells across CUDA 12.9.2 and 13.4.2. Required missing-input and unsupported-architecture cases fail without skips. The manual workflow is syntax-validated but undispatched; its 12.9.1/13.4.1 container patches differ from the local packages.

## Context and Current Pipeline

For example, `tests/cuda/nvvm-core-execution.slang` contains compute control flow and synchronization.
`slangc -emit-cuda-via-nvvm` lowers checked Slang IR through the compiler-matched LLVM 14 provider,
then the selected toolkit's libNVVM emits PTX. The selected toolkit's `ptxas` consumes that PTX and
produces a cubin. Both boundaries must succeed; PTX file existence alone is insufficient.

## Scope and Non-Goals

Own `extras/validate-nvvm-toolkit.py`, `.github/workflows/nvvm-toolkit-validation.yml`, and this
slice's plan/report. Do not modify compiler semantics, frozen corpus identities, GPU dispatch,
performance measurement, global design documentation, or the slice197 provider builder.

## Architecture and Invariants

One fixed workload list owns source paths and entry points. Each requested architecture and O0/O3
combination produces an independent record. Delete only each named output before its producer runs,
so stale artifacts cannot satisfy validation. All required records must pass before the runner exits
successfully. A source/tool failure produces infrastructure records rather than reducing the count.

## Interfaces and Dependencies

Runner arguments identify `--slangc`, `--provider`, `--cuda-root`, `--architectures`, and `--output`.
Optional `--optimizations` selects O0/O3; defaults test both. Require CUDA toolkit version metadata,
libNVVM, libdevice, and ptxas from that root. Results include commands, exit codes, actual PTX target,
logs, output paths, and version metadata. The workflow uses `extras/build-nvvm-provider.py` to build
the compiler-matched provider against isolated LLVM 14 rather than using the main compiler LLVM.

## Milestones

1. Add the standalone runner with explicit inputs, artifact checks, and deterministic records.
2. Add two CUDA development-container CI jobs via a matrix and upload evidence even on failure.
3. Run local CUDA 13.4 SM80/90 O0/O3, missing-tool/unsupported-target negative gates, and syntax checks.

## Validation and Acceptance

Run `python3 extras/validate-nvvm-toolkit.py --slangc build/Debug/bin/slangc --provider <provider>
--cuda-root /usr/local/cuda-13.4 --architectures 80 90 --output build/nvvm-slice199`. Require all
expected records to pass. Missing provider/toolkit and CUDA 13 SM70 must exit nonzero with JSON
failures; no skip rows are allowed. Check Python syntax, workflow YAML/actionlint if available,
and formatting. The exact CUDA 12.9.1 and 13.4.1 CI containers remain undispatched; local CUDA 12.9.2 and 13.4.2 validation is complete.

## Failure and Recovery

Each run writes logs and a complete JSON result inventory. Rerunning replaces only artifacts owned
by each required cell and its summary. Missing dependencies require installing or selecting them;
compilation or assembly failures point to their per-cell log. Remove the manual workflow to disable
remote automation without affecting existing Slang build/test workflows.

## Artifacts and Hand-Off

Retain bulky PTX/cubin/logs under `build/nvvm-slice199`; record counts, exact commands, negative-gate
results, and unexecuted remote cells in `report.slice-199-toolkit-matrix.md`. Parent owns integration
and commits after the slice is complete.
