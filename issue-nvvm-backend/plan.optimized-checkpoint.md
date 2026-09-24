# Establish the optimized configuration checkpoint

This ExecPlan follows `.agent/PLANS.md`. The NVVM workflow explicitly requires completed plans
and reports to be committed; the parent agent owns review and the local commit.

## Purpose and Observable Result

Validate the unchanged accepted slice-202 compiler in RelWithDebInfo before selecting another
feature. Every one of the 1,605 runtime cells must be freshly measured and compared with the
Debug preservation obligation: 1,544 correct cells and 61 explicit failures. This adds validated
configuration evidence, not compiler functionality or material runtime claims.

## Progress

- [x] 2026-09-24: Read repository, planning, workflow, status, build skill, and provider instructions.
- [x] 2026-09-24: Verified clean starting revision `ecfacff50002bd9b60f5250b56a2023a399581e5`.
- [x] 2026-09-24 15:19 UTC: Built matching RelWithDebInfo compiler, provider, test tools and libraries.
- [x] 2026-09-24 15:19 UTC: Early GPU runtime gate passed all four fixtures; runtime report captured compiler/provider/toolkit/device identity.
- [x] 2026-09-24 15:20 UTC: Selected units passed 473/473 with one Windows-only skip; toolkit passed 18/18.
- [x] 2026-09-24 15:36 UTC: Full frozen replay completed, 1,356 cells and unchanged 449/438/438 correct counts.
- [x] 2026-09-24 15:40 UTC: Discovery completed 249 cells, unchanged 73/73/73 correct; all six complex cells preserved.
- [x] 2026-09-24: Exact inventories, all 1,544 correct cells and 61 failures preserved; one known multisample diagnostic change classified.
- [x] 2026-09-24: Completed compact evidence, five-part report, self-review, and draft STATUS; parent owns acceptance and commit.

## Surprises and Discoveries

The existing helper explicitly selects Debug. Override every configuration path. Root CMake uses
Ninja Multi-Config, embeds core source but not core binary, and enables the isolated provider with
its pinned LLVM14 package. No source changes are needed to select RelWithDebInfo.

The initial root build used four outer jobs and inherited `CMAKE_BUILD_PARALLEL_LEVEL=4` into
the provider ExternalProject. Its two-object child may briefly have overlapped three root jobs;
actual overlap was not measured. Future incremental builds must set inherited level 1 and explicitly
select four outer jobs. This scheduling limitation does not support any build-speed claim.

The multisample diagnostic changed because the optimized configuration proceeded beyond the
Debug-only `_getTypeName` null-handle assertion and emitted missing texture type declarations.
NVRTC rejected those declarations. The runner returned the same infrastructure classification;
the manifest retains its empty parsed diagnostic and the actual raw NVRTC errors separately.

## Decision Log

- 2026-09-24, worker: Reuse the configured native Linux multi-config build and isolated LLVM14
  package; build with four workers. Run suites sequentially with at most four CPU workers total.
- 2026-09-24, worker: Keep Debug slice-202 outcomes as immutable obligations. Different diagnostics
  remain explicit differences and cannot silently replace passes or reset the baseline.

## Outcomes and Retrospective

Validation is complete and ready for parent review. All 1,605 runtime cells are fresh; all 1,544
previous correct cells and 61 failures retain their classifications and execution outcomes. No
missing or duplicate cells exist. The known multisample NVRTC failure changes from a Debug
null-handle assertion to malformed CUDA type-declaration errors; it remains failed. Both complex
NVRTC entries assemble, while all four direct cells still reject `LoadFromUninitializedMemory`.
Runtime 4/4, units 473/473 plus one Windows-only skip, and toolkit 18/18 passed. No compiler
feature changes were made.

A controlled two-fixture compile experiment used one warmup and three alternating sequential
samples per configuration after all suites finished. Debug/optimized median ratios were 1.228
and 1.270 with identical PTX. This is limited host compilation evidence, not GPU performance.

## Context and Current Pipeline

`slangc` emits CUDA for NVRTC O3 or direct NVVM via the matching provider at shader O0/O3. Corpus
runners compile and execute selected compute contracts through matching render/test tools, then
check their existing output oracles. The host compiler configuration is independent of these three
shader modes. Complex material entries compile and assemble only; their direct modes currently
stop at `LoadFromUninitializedMemory`, and runtime bindings/oracles are still absent.

## Scope and Non-Goals

Only configuration build and evidence/documentation. No compiler, runner, manifests selecting
workloads, provider ABI, drivers, reboot, push, or publication changes. Parent reviews and commits.

## Architecture and Invariants

Preserve 452 frozen and 83 discovery identities, exactly three mode rows each, without duplicates
or omissions. All 1,544 previously correct rows must stay correct. Retain exact classifications,
diagnostics, source contracts and existing failure provenance. Provider ABI stays 35.

## Interfaces and Dependencies

Native Linux CMake preset `releaseWithDebugInfo`; all tools under `build/RelWithDebInfo`.
CUDA `/usr/local/cuda-12.9`, architecture 80; observed device expected L4/SM89. Matching provider
is `build/RelWithDebInfo/bin/libslang-llvm-nvvm.so`; LLVM14 is pinned and statically isolated.
Baseline: `census.slice-202.tsv`, `discovery-census.slice-202.tsv`, and
`runtime-validation.slice-202.json` under this directory.

## Milestones

1. Build with `cmake --build --preset releaseWithDebugInfo --parallel 4 --target slangc slang-test
render-test test-server`, logging under unique ignored `build/nvvm-loop/optimized-checkpoint-*`.
2. Set optimized compiler/provider PATH and CUDA environment; run
   `extras/validate-nvvm-runtime.py --config RelWithDebInfo --cuda-path /usr/local/cuda-12.9
--architecture 80 --output <run>/runtime` before expensive GPU suites.
3. Run exact WORKFLOW full-checkpoint commands with optimized paths: selected unit tests (two
   servers), toolkit (SM80), census (slice-195 frozen selection, four workers), discovery (83-entry
   manifest, four workers), and complex (zero warmups, one sample). Each suite has a bounded timeout.
4. Produce checked-in per-cell TSVs and JSON with inventories, outcome transitions, 61-failure
   ledger/evidence references, provenance, gates and raw evidence hashes; report and STATUS handoff.

## Validation and Acceptance

Require 1,356 frozen + 249 discovery cells and all six registered complex backend/entry cells.
Expected runtime correct by mode: frozen 449/438/438 and discovery 73/73/73. Units and runtime/toolkit
must pass their actual executed inventories. Record skips, known failures, crashes and diagnostics
as observed. Compare classifications and diagnostics rather than relying on process exit codes.
No speedup assertion is made without controlled measurement.

## Failure and Recovery

Stop GPU dispatch on device loss; no driver replacement or reboot. Investigate any optimized loss
with its smallest reproduction and Debug comparison. Do not accept a loss or reset the baseline.
Bound suite time and retain partial evidence on timeout. Safe reruns use new output directories.

## Artifacts and Hand-Off

Raw logs, binaries, dumps and scripts remain in ignored `build/`. Durable outputs:
`runtime-validation.optimized-checkpoint.json`, `census.optimized-checkpoint.tsv`,
`discovery-census.optimized-checkpoint.tsv`, this completed plan,
`report.optimized-checkpoint.md`, and the accepted `STATUS.md` update.

Parent integration review accepted the checkpoint: independent exact-cell comparisons matched both
corpora; the one diagnostic transition was verified against source and raw evidence.
