# Implement CUDA wave rotation through typed indexed shuffles

This ExecPlan follows `.agent/PLANS.md` and `issue-nvvm-backend/WORKFLOW.md`. The maintainer's
NVVM exception requires committing completed plans and reports with each accepted slice. The
worker does not commit; the parent owns final acceptance and the local commit.

## Purpose and Observable Result

Make the two unchanged frozen WaveRotate/WaveClusteredRotate workloads execute correctly at
NVVM O0/O3. Add race-free, per-lane CUDA fixtures with independent expected output that check
scalar families, every vector lane, cluster boundaries and rotation deltas. Preserve NVRTC O3.

## Progress

- [x] 2026-09-24: Read repository, workflow, accepted 206 evidence and native build skill.
- [x] 2026-09-24: Audit rotation producers and existing scalar shuffle operation.
- [x] 2026-09-24: Accepted206 binary/toolkit hashes match; small GPU gate4/4; both final fixture sources pass NVRTC and fail NVVM O0/O3 (2/6).
- [x] 2026-09-24: Implemented bounded library composition and scalar shuffle transport extension.
- [x] 2026-09-24: Final build passed; focused 8/8, runtime 4/4, units 474/474+1 skip, toolkit 18/18 and complex 6/6 passed. Full corpus running.
- [x] 2026-09-24: Full checkpoint complete: 1629 fresh cells, 1572 correct, 57 retained failures; exactly four old rotation fixes and six correct additions.
- [x] 2026-09-24: Completed input-shape self-review, report, design/status updates and review handoff; parent acceptance/commit remains separate.

## Surprises and Discoveries

The existing provider indexed shuffle only admits int32/uint32/float32. The old frozen rotation
sources instantiate int8/16/64, uint8/16/64, half and bool as well as vectors. Simply spelling
rotation as a vector WaveMaskReadLaneAt would move the diagnostic: its selected-vector consumer
only admits 32-bit numeric lanes. Scalar CUDA rotation already defines the required lane formula.

Further measured discoveries: the ordinary NVRTC 12.9 adapter requests at least compute75;
`-Xnvrtc -arch=compute_50/60` is needed for actual lower-target PTX probes. The broader frozen
SM50 source enables unchanged CUDA half-math wrappers unavailable at that architecture. A separate
integer-only legacy fixture validates the branch contract without expanding into a prelude fix.
Raw failed probes remain under `slice-207-after/legacy-*`; the manifest separates them from the
four successful source tests and four actual lower-target compile/assembly probes.

## Decision Log

- 2026-09-24, worker: Rank the two rotations ahead of quad reconvergence, five partitioned
  wave-multi operations, and FP8/BF16/prelude/harness gaps. They have runnable frozen contracts,
  simple indexed transport semantics and bounded width legalization. All six complex cells already
  compile/assemble after 206; material runtime lacks bindings/oracles, so no speculative material
  change is justified. Accepted 204/205/206 were complex-driven, satisfying the rolling cadence.
- 2026-09-24, worker: Prefer library composition over new rotation semantic IDs or source-text
  parsing. CUDA scalar rotation computes a lane then calls typed WaveMaskReadLaneAt with the same
  full mask. Vector rotation applies the scalar rotation componentwise. Extend the existing scalar
  indexed-shuffle semantic's admitted widths at its provider boundary, preserving exact bits through
  32-bit shuffle words. No new operation ID or ABI is necessary. Revisit if before/focused tests
  show an independent blocker; do not retain a patch justified solely by diagnostic advancement.

- 2026-09-24, worker: Initial core-module build proved `subgroup_rotate` advertises CUDA SM5,
  while the public indexed shuffle and lane-index APIs advertise SM7. Preserve the old CUDA
  source/prelude branch and compose the existing APIs only under `_cuda_sm_7_0`. Target-switch
  cases require capability atoms (the `cuda_sm_7_0` alias was rejected); no capability definition
  changes. This is a public lower-target preservation boundary, not an unsupported-input fallback.
  Existing lower-level lane query is an untagged builtin, and introducing a second shuffle helper
  would duplicate canonical semantic admission. The historical CUDA prelude arithmetic remains
  unchanged and is not newly copied. Compile/assembly probes will retain SM5/6 coverage.

## Outcomes and Retrospective

Implementation and all gates are complete; parent acceptance is complete, and this plan is included in the accepted local commit.
The two frozen rotation identities now pass both direct modes. All prior correct cells and every
unaffected exact outcome/diagnostic/count are preserved. Legacy integer scalar/vector source
selection and actual SM50/SM60 compile/assembly are separately proven. The initial lower-target
probe exposed the unchanged NVRTC adapter's SM75 clamp; explicit downstream override resolves the
probe architecture. A broader frozen SM50 half-math prelude failure is retained as an exploratory
limitation and was not repaired or counted as a pass. All six complex cells still compile/assemble;
material runtime semantics are still absent. No next independent feature was investigated.

Base is accepted commit
`1214f6b4d969ee0a5395dce748a5d460899eac92`, branch `nvvm-backend`, clean at start.

## Context and Current Pipeline

For `WaveClusteredRotate(uint64_t(lane) << 40, 3, 8)`, the CUDA standard library currently emits
an untagged `_slang_waveClusteredRotate` GenericAsm. Its prelude computes clusterStart and source
lane, then CUDA shuffles the value. Direct NVVM correctly rejects that unowned assembly.
`WaveMaskReadLaneAt` already emits producer-owned `nvvmWaveReadLaneAt`; its typed catalog and
provider own indexed shuffle. Canonical small-vector components need no new IR representation.
The planned producer computes `clusterStart + ((lane - clusterStart + delta) % clusterSize)`
and retains full-mask participation. WaveRotate computes `(lane + delta) % 32`.

## Scope and Non-Goals

Only rotation and the necessary scalar indexed transport widths. Preserve frozen source contracts,
resource ABI and provider ABI 35. Do not implement partitioned reductions, divergent reconvergence,
quad control, material runtime setup or another independent feature. Invalid intrinsic signatures
remain rejected before provider mutation. Valid cluster sizes are powers of two 1 through 32;
this slice does not redefine invalid cluster sizes or inactive source-lane behavior.

## Architecture and Invariants

The standard library owns rotation lane arithmetic and generic componentwise composition. The
semantic catalog owns admitted signatures. The provider owns bit-preserving transport through its
native 32-bit indexed shuffle: narrow values extend/truncate, 64-bit values shuffle two words and
reassemble; floating-point values preserve bit patterns. No semantic value is converted to syntax,
no source spelling recognition is introduced, and the existing full mask is preserved.

## Interfaces and Dependencies

Native Ubuntu, optimized build `build/RelWithDebInfo`, CUDA 12.9.2/NVRTC 12.9.86, LLVM14 provider
ABI35, NVIDIA L4 SM89 driver580.126.09, target SM80. Read `build/nvvm-loop/slice-203-env.sh` before
sourcing it; it overrides Debug paths in setup env. Four total CPU workers, sequential suites,
two unit servers. All raw evidence lives in `build/nvvm-loop/slice-207-{before,after}`.

## Milestones

1. Create per-lane rotation fixture and exact independent oracle, register two disjoint discovery sources,
   run it before compiler changes and retain source hashes and the failing NVVM diagnostics for both fixtures.
2. Change CUDA rotation branches in `source/slang/hlsl.meta.slang`, extend typed scalar shuffle
   admission and provider emission only as required. Add focused rejection coverage for malformed
   signatures at the existing preflight/provider boundary.
3. Format changed code, rebuild using `CMAKE_BUILD_PARALLEL_LEVEL=1 cmake --build --preset
releaseWithDebugInfo --parallel 4 --target slangc slang-test render-test test-server` and required
   provider target; inspect focused real-GPU outputs and final IR.
4. Run full checkpoint and machine-check exact keys/classification/return codes/full execution_counts/
   diagnostic/canonical_shape. Record four expected old-cell fixes separately from new cells.
5. Complete five-part report, manifest, portable census TSVs, design and STATUS; return ownership.

## Validation and Acceptance

Reuse exact accepted 206 before evidence for 452 frozen +89 discovery identities (1623 cells,
1562 correct,61 known failures), after verifying source/binaries/toolchain/runner identities.
The new final fixture must fail before and pass after at both NVVM optimizations with an independently
expected NVRTC reference. If its source changes after the before run, repeat an actual revert drill.
The library/provider contract changes require a full checkpoint, not targeted-only acceptance.

Run the small gate first: `python3 extras/validate-nvvm-runtime.py --config RelWithDebInfo
--cuda-path "$CUDA_PATH" --architecture 80 --output "$NVVM_RUN/runtime"`.
Run focused test files, all NVVM/routing/reporter units (baseline473 pass+1 skip), toolkit18,
all six complex compile/assembly cells, full frozen with `--workload-ids-from
issue-nvvm-backend/census.slice-195.tsv --jobs 4`, and full discovery manifest `--jobs 4`.
Use the commands in WORKFLOW.md and a retained per-gate script; bound each suite with timeout.
Discovery loader requires50–100 entries before filters, so focused discovery uses full manifest
plus disjoint `--match`. Keep full execution counts and exits, even expected corpus exit2.
All previous correct cells must remain correct; retain unresolved failures and history exactly.
Require final formatted source hashes and binary hashes before/after final gates.

## Failure and Recovery

Stop GPU dispatches immediately on device loss and notify parent. Do not reboot/change drivers.
A timeout/interrupted run is incomplete. New regressions block acceptance: isolate/fix or revert,
then repeat affected checks and complete checkpoint. Save prototype code before any revert drill;
restore only this worker's edits. Do not modify frozen contracts to hide gaps. Stop after documenting
any next independent blocker with exact diagnostics and a minimal trace.

## Artifacts and Hand-Off

Durable: this completed plan, `report.slice-207-wave-rotation.md`,
`runtime-validation.slice-207.json`, `census.slice-207.tsv`, `discovery-census.slice-207.tsv`,
discovery manifest addition, design facts and STATUS. Local: before/after logs, IR, hashes,
commands, inventories and comparison script. Parent alone accepts and commits; no push.

## Parent acceptance

Accepted 2026-09-24 after production and boundary-test review. Independent raw206/207 comparisons found exactly four repaired rotation cells, six correct additions, and no other changes in classifications, return codes, execution counts, diagnostics or canonical shapes. All requested inventories are exact. All 12 artifact and 35 tested-source hashes match; four lower-target compile/assembly probes confirm actual SM50/SM60 targets and legacy call sites. Full-checkpoint cadence resets to zero.
