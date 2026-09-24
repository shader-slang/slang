# Slice 210: preserve small double source literals

This ExecPlan follows `.agent/PLANS.md`. Completed plans and reports are committed under the
NVVM loop exception; raw logs and binaries remain ignored under `build/`.

## Purpose and Observable Result

Source-emitted finite double constants must parse back to identical binary64 bits. The motivating
slice 208 value `(1 + 2^-30) / 65536` became `0.00001525878907671` in CUDA, corrupting a trusted
NVRTC differential reference. Correctness overrides complex cadence: six materials compile but
have no application runtime contract. No material changes are justified.

## Progress

- [x] 2026-09-24: Read repository/workflow/build instructions and accepted 209 evidence.
- [x] 2026-09-24: Create bounded plan before implementation.
- [x] 2026-09-24: Old host API test fails 92 bit comparisons; GPU NVRTC fails, NVVM O0/O3 pass.
- [x] 2026-09-24: Corrected formatting policy; focused 4/4 pass (host 2127 patterns x 4 targets and GPU 3).
- [x] 2026-09-24: Final focused 4, smoke 4, units 476 (1 skip), shared-target 16 (3 skips), toolkit 18 pass.
- [x] 2026-09-24: Full frozen 1356/discovery 291 and complex 6 complete; all 1591 prior passes preserved, 53 known failures unchanged, 3 additions pass.
- [x] 2026-09-24: Exact old-cell fields match; report/manifest/censuses/design/STATUS finalized for parent review. Write ownership returned with handoff.

- [x] 2026-09-24: Parent independently reviewed and accepted the full checkpoint for local commit.

## Surprises and Discoveries

The existing fixed format precision is fractional digits, whereas roundtrip requires significant
digits. Existing mantissa trimming assumes a decimal point; an unqualified defaultfloat switch
would corrupt integer spellings ending in zero. No direct SourceWriter unit previously existed; public API emission now covers four text targets.
Initial host/GPU failures establish the shared writer boundary. A final comment-only clarification
was followed by a rebuild and complete gate restart; no full corpus work had begun.

## Decision Log

- 2026-09-24, worker: preserve existing locale and trimming machinery; prefer a minimal change to
  fixed/scientific selection. Revisit if focused range tests show another defect.
- 2026-09-24, worker: shared source emission has broad impact, requiring full 452 x 3 frozen and
  96 x 3 discovery plus additions checkpoint, representative source targets and complex 6.

## Outcomes and Retrospective

Shared formatter correction, focused/regression gates and full 1647-cell checkpoint pass their
criteria. All 1591 prior correct cells are preserved; 3 new cells pass and 53 known failures remain
unchanged. Complex 6 compile only. Parent accepted the full checkpoint for local commit.

## Context and Current Pipeline

A canonical double-valued IR float literal reaches C-like target emission and
`SourceWriter::emit(double)`. It uses classic-locale iostream formatting and max_digits10
precision, choosing fixed for absolute binary exponents below 17. A small finite literal is a
valid canonical input; changing its semantic producer or adding a CUDA workaround would be wrong.
`visitFloatingPointLiteralExpr` calls `IRBuilder::getFloatValue`, which preserves the double
payload. `CLikeSourceEmitter::emitSimpleValueImpl` sends that payload to this shared writer.

## Scope and Non-Goals

Fix finite-double roundtrip only. Preserve signed zero, locale independence, syntax and trimming.
Do not change NaN/Inf policy, add numerical features, alter old fixture oracles, expand frozen v1,
push, commit, modify driver, reboot, or start an independent feature. At most one discovery source
is needed (96 current, hard maximum 100).

## Architecture and Invariants

Existing IR is semantic source of truth. Decimal spelling must preserve it. Standard scientific
format with max_digits10 fractional digits is sufficient across the finite double exponent range;
fixed precision is retained only where it supplies sufficient significant digits. No new semantic
representation, equivalence relation, fallback, or syntax reconstruction is introduced.

## Interfaces and Dependencies

No API/ABI changes. Native Ubuntu, optimized tools, CUDA 12.9.2/NVRTC 86, LLVM 14 provider ABI 36,
L4 SM89 driver 580.126.09 target 80. Source `build/nvvm-loop/slice-203-env.sh`. Base revision
`1f161256b4bc8f57aa96bc0dd250f8e2a537ef1c`; accepted 209 is the preservation baseline.

## Milestones

1. Add boundary/locale/sign/normal/subnormal host emission coverage and GPU data-driven oracle;
   run against unchanged compiler, retain emitted source and failure logs under slice-210-before.
2. Update `source/slang/slang-emit-source-writer.cpp` formatting policy, build optimized tools,
   and demonstrate focused tests passing. Record exact input-shape audit in report.
3. Add eligible fixture to discovery manifest. Run full gates and exact structured comparison.
4. Distill durable behavior into design, manifest, census, five-part report and STATUS.

## Validation and Acceptance

Use `CMAKE_BUILD_PARALLEL_LEVEL=1 cmake --build --preset releaseWithDebugInfo --parallel 4
--target slangc slang-test render-test test-server`. Four total CPU workers, two unit servers,
sequential suites and four corpus workers. All suites use bounded timeout. Focused host tests
cover exponent boundaries, tiny normal/subnormal, sign/zero, and precision beyond float32.
GPU oracle uses runtime input plus independent integer-bit expectations at NVRTC O3/NVVM O0/O3.
Smoke 4 precedes expensive GPU work; relevant unit domain 475 plus new tests, toolkit 18, full
frozen 1356 and discovery 288+additions and complex 6. Existing failures return 2 in full runners.
Compare exact id/mode/classification/return_code/full execution_counts/diagnostic/canonical_shape.
Baseline 1644 fresh cells: 1591 correct 53 known failures. Preserve resolved 208 histories. No inherited
runtime evidence in final checkpoint. Source/binary hashes recorded after final relevant edits.

## Failure and Recovery

Save matching old artifacts before rebuild when useful. Do not dispatch more GPU work after device
loss. A new failure requires isolation and correction/revert; prior failure requires baseline proof.
Restore only this slice's changes if approach fails; preserve accepted 209 and all old oracles.
Raw reruns use distinct before/after directories and explicit output paths.

## Artifacts and Hand-Off

Raw: `build/nvvm-loop/slice-210-{before,after}`. Durable: this plan, five-part report,
`runtime-validation.slice-210.json`, census/discovery-census 210, manifest addition, STATUS and design.
Return <=500-word handoff with exact deltas, identities, limits and explicit ownership return.

Final evidence is in `runtime-validation.slice-210.json` and both 210 census tables. The public API
unit exposes the actual shared writer without adding a production test hook or exporting internals.
The unchanged IR/direct-NVVM results and failing old emitted references proved the writer owned
this defect. One sufficient binary magnitude bound repaired the precision contract without a
custom decimal algorithm or target-specific workaround. Four-source and three-PTX structural
probes confirm corrected spelling and retained runtime data dependence. All final hashes match.

Parent acceptance: slice 210 is the latest accepted implementation and full checkpoint; cadence is 0.
Independent source, artifact, structural and exact outcome reviews found no outstanding concern.
