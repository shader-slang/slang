# Legalize canonical scalar numeric truthiness

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed experimental-backend slice plan to be committed with its implementation,
overriding the normal working-log policy for this branch.

## Purpose and Observable Result

After this slice, canonical checked casts from selected scalar integer and floating types to Bool
share one numeric-truthiness recipe. The existing integer behavior remains unchanged; Half,
Float32, and Float64 values compare unordered-not-equal to an exact typed zero, preserving false
for both signed zeros and true for nonzero, infinity, and NaN.

The primary frozen workload is
`tests/language-feature/conversions/conversion-to-bool.slang`, which exercises 65 results across
Bool, signed/unsigned integers, Half, Float32, and Float64. Native CUDA and direct NVVM O0/O3 must
agree before permanent direct lanes are added. Corpus v1 and discovery remain separate contracts.

## Progress

- [x] (2026-09-01) Selected ordinary floating truthiness from the updated Pareto: it is a common
  scalar semantic and the frozen fixture provides a broad deterministic differential oracle.
- [x] (2026-09-01) Traced the first stop to canonical `kIROp_CastFloatToInt` with a scalar Bool
  result; the current integer-truthiness recipe handles only `kIROp_IntCast`.
- [x] (2026-09-01) Generalized the existing recipe and added focused Half/Float32/Float64 topology
  coverage without revising provider ABI 33.
- [x] (2026-09-01) Passed all 65 native/O0/O3 results, promoted two stable lanes, replayed both
  corpora, measured SM70/80/90, passed selected/permanent regressions, formatted, and self-reviewed.

## Surprises and Discoveries

- The generic semantic catalog intentionally rejects `FLOAT_TO_INTEGER` when its result is Bool.
  This is correct: truthiness is comparison with zero, not an out-of-range floating conversion.
- Provider ABI 33 already supports selected floating comparisons and typed floating constants.
  Its `NOT_EQUAL` predicate is LLVM `fcmp une`, which makes NaN truthy as required.
- A focused unit source with raw Half/Float32/Float64 entry parameters mixed truthiness ownership
  with launch-ABI coverage. Deriving Half and Float64 locally from one established Float32 entry
  parameter isolates the intended semantic while still proving all three comparison descriptors.

## Decision Log

- Decision: generalize the existing integer-truthiness recipe rather than adding a parallel float
  conversion rule to the catalog.
  Rationale: both canonical casts mean numeric nonzero, and both require the same Bool-producing
  comparison invariant. `FLOAT_TO_INTEGER` must remain reserved for actual integer results.
  Date/author: 2026-09-01, Codex.
- Decision: admit only selected scalar values in this slice.
  Rationale: the motivating producer emits scalar casts and the existing integer contract is
  scalar. Vector widening without a canonical failing producer would be speculative.
  Date/author: 2026-09-01, Codex.

## Outcomes and Retrospective

The 65-result workload passes native CUDA and direct NVVM O0/O3 and now owns two permanent direct
lanes. Frozen corpus v1 remains exactly 452 rows/427 healthy references and advances from
415/415/415 to 416/416/416 O0/O3/both; `conversion-to-bool` is the only changed row and there are
no old-correct regressions. All-row frozen direct totals are 430 correct, three runtime mismatches,
and 19 preflight failures in each mode.

Discovery remains exactly 82 rows/72 healthy references at 72/72/72 with no changed row. The
selected prefix passes 435/435 and the permanent `nvvm` category passes 86/86. Native, direct O0
SM70, and direct O3 SM70/SM80/SM90 PTX all assemble through CUDA 12.9. At SM70 the exploratory
one-repetition measurements are 489.3 ms/38,917 bytes native, 278.3 ms/32,829 bytes direct O0,
and 287.6 ms/30,174 bytes direct O3.

## Context and Canonical Ownership

Checked conversion lowering represents `bool(value)` from an integer as `kIROp_IntCast` and from a
floating value as `kIROp_CastFloatToInt`, both with Bool result type. The latter name reflects the
broad numeric cast family; its complete source/result types determine that this instance is
truthiness. The shape is canonical and intentionally valid. The direct emitter owns decomposing
that multi-operation semantic into typed zero plus `NOT_EQUAL`, just as it already does for integer
truthiness.

The shared resolver must prove one operand, scalar Bool result, and a selected scalar integer or
float source. Preflight and emission consume the same recipe. The exact source type chooses an
integer or floating zero; the comparison descriptor remains the shared typed-provider operation.

## Scope and Non-Goals

In scope are scalar Int8/16/32/64, UInt8/16/32/64, Half, Float32, and Float64 truthiness; the
existing fake/provider operation path; focused negative adjacent-shape coverage; promotion of the
frozen fixture; both corpus replays; selected/permanent regressions; and SM70/80/90 measurement.

Out of scope are vector truthiness, BFloat16, FP8, arbitrary float-to-integer conversion changes,
source reconstruction, compatibility callbacks, fixture-name checks, and malformed-IR repair.

## Validation and Acceptance

Acceptance requires Release host/provider builds and tests outside the sandbox; focused fake
topology proving floating `NOT_EQUAL` against typed zero; all 65 fixture values correct in native,
direct O0, and direct O3; frozen v1 exactly 452/427 with no old-correct regression; discovery
exactly 82/72; selected-prefix and permanent-category passes; representative PTX assembly for
native, O0 SM70, and O3 SM70/80/90; changed-line clang-format 17; and `git diff --check`.

## Self-Review Inventory

Audit the generalized resolver, requirement collector, emitter, and any test-only source. For each
retained branch, record the concrete producer, why scalar numeric-to-Bool is canonical, which test
fails without it, and why the provider's existing comparison/constant API owns every primitive.

## Artifacts and Recovery

Keep transient IR, logs, PTX, cubins, and timings below ignored `build/nvvm-census/slice178-*`.
Commit the completed plan with implementation, tests, promoted directives, durable docs, census
TSV/JSON, measurement manifest, and five-part report. The changes are isolated to the direct NVVM
route and can be reverted without changing NVRTC.
