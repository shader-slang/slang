# Extend masked integer MIN/MAX to 64 bits

This ExecPlan follows `.agent/PLANS.md`. The maintainer requires completed NVVM plans and reports
in the slice commit. Generated logs, scripts, binaries and raw probes remain under ignored build/.

## Purpose and Observable Result

Signed and unsigned 64-bit masked MIN/MAX reductions and inclusive/exclusive prefixes execute
correctly through direct NVVM at O0 and O3, including scalar/vector2/vector4 and existing matrix
reduction leaves. One registered fixture and the unchanged research 230 mathematical oracle
make both signedness and empty exclusive-prefix identities observable.

## Progress

- [x] 2026-09-25 Read instructions, accepted research 230, full checkpoint 229 and build skill.
- [x] 2026-09-25 Select bounded identity admission/materialization implementation on base
      51adf2c8c6ab9c61de4e87dfcc187ffca865d126; establish this plan before implementation.
- [x] 2026-09-25 Final readable fixture passes NVRTC; direct O0/O3 reject on unchanged accepted library.
      Captured 92 excluded-operation and 14 ordinary aggregate shuffle rejection cells before edits.
- [x] 2026-09-25 Two-function emitter patch formatted and built; focused fixture 3/3, smoke 4/4,
      all 12,768 oracle launches and 80 minimal direct family compiles pass.
- [x] 2026-09-25 All required gates pass; 639 fresh plus 1,035 inherited cells, 1,623 correct
      cumulative, 51 known failures and six resolved histories preserved. Exactly four diagnostic-only
      prefix transitions reach half; no old classification or other field changed.
- [x] 2026-09-25 Complete durable evidence, design, report and STATUS; checkout ownership returns
      to the parent with the worker handoff for independent acceptance and local commit.

- [x] 2026-09-25 Parent verified the diff, 525 unique references, all source/artifact/input hashes,
      exact corpus outcomes/inheritance, failure histories and 12,768 replay launches.

## Surprises and Discoveries

Research 230 already proves provider operations and canonical aggregate shapes. Identity admission
currently restricts MIN/MAX to 8/16/32-bit integers; widening alone exposes unsigned shift-by-64 and
signed constant-conversion hazards. No producer repair is indicated by accepted evidence.

Fixture-only setup attempts exposed comment reflow of line-oriented TEST_INPUT directives and generic
conversion restrictions. Preserve normal Slang formatting; use IInteger.toUInt64 and its declared
int64 constructor rather than assuming a generic scalar cast. Three failed fixture attempts remain separate
from the final before proof; none changed production code. A later audit initially expected the
canonical_shape field to change with the diagnostic; fresh evidence showed only diagnostic text
changes. The audit assertion was corrected without rerunning or altering corpus results.

## Decision Log

2026-09-25: Rank 64-bit integer MIN/MAX first because four frozen prefix failures reach exactly
this recipe, the source/oracle gate is accepted, and one algebra serves reductions and prefixes.
Material runtime ranks below this work because application bindings, textures/LUTs, inputs and
output oracle are missing. Matrix prefixes, arithmetic/bitwise/FP16, ordinary aggregate shuffle,
quad reconvergence and resource contexts require independent contracts and are excluded.

2026-09-25: Use targeted acceptance: selection.slice-208-frozen.tsv (107 identities/321 cells)
and all 105 old discovery identities plus one fixture (106/318), giving 639 fresh cells. Inherit
1,035 other frozen cells explicitly from full 229, cumulative 1,674. Full checkpoint 229 remains
latest; cadence advances zero to one only on parent acceptance. Expand to full if scope broadens.

## Outcomes and Retrospective

Implementation and independent parent acceptance are complete. The final fixture passes all three modes;
12,768 replay launches compare 28,600,320 exact words and all 12 runtime PTX artifacts assemble.
All 80 minimal direct family probes compile. Final gates: smoke 4, units 479 plus one existing skip,
toolkit 18, contracts 6, targeted frozen 321, discovery 318 and material compile/assembly 6.
No provider/ABI, frontend, aggregate classifier or general literal change was needed.

The four original prefix diagnostics advance to canonical half exclusive MIN/MAX E52017. This
separate FP16 boundary is the handoff; no next implementation starts. The cumulative inventory is
1,674 cells with 1,623 correct and 51 known failures. All six resolved histories and first-known
records remain. Accepted cadence is one implementation since full checkpoint 229.
Accepted research 230 artifacts and all 553 old runtime input hashes remain unchanged.

## Context and Current Pipeline

For `int64_t result = WaveMultiPrefixExclusiveMin(value, uint4(mask, 0, 0, 0));`, with dynamic
low/high-word input, hlsl.meta.slang produces canonical GenericAsm
`_wavePrefixExclusiveMin(($1).x, $0)`. Existing recipe admission calls
`_getNVVMMaskedWaveScalarIdentity`, stores raw identity bits, and
`_emitNVVMMaskedWaveScalarValue` materializes a provider constant. Existing scalar lane scans and
aggregate leaf traversal consume the same recipe. Matrices are canonical arrays of vectors;
no checked semantic data is reconstructed into syntax. Signed minimum and unsigned maximum must
be passed as signed-width constant arguments while preserving all 64 raw bits.

## Scope and Non-Goals

Only source/slang/slang-emit-nvvm.cpp production changes, one new runtime fixture and discovery
registration. Existing scalar/vector/matrix reduction recipe, mapping and typed operations remain
authoritative. Do not broaden operation families, aggregate shuffle policy or matrix-prefix
capabilities. No commit/push, driver/system change, reboot or material runtime/performance claim.

## Architecture and Invariants

MIN/MAX selects representable operands; member-set integer extrema need no overflow algebra.
Admit integer width 64 only for these operations. Compute unsigned all-ones without shifting by 64.
Use existing Slang::bitCast for signed-width-64 constant arguments; retain narrow sign extension.
Preserve all 8/16/32-bit and FP32/FP64 identity bits. No new helper, fallback or representation.
Singleton exclusive probes distinguish identities; negative probes preserve excluded signatures.

## Interfaces and Dependencies

Native Ubuntu, L4 SM89 target SM80, CUDA 12.9.2/NVRTC 12.9.86, LLVM 14, ABI 36. Source
build/nvvm-loop/slice-203-env.sh for matching RelWithDebInfo bins/libs and formatter tools. Build:
`CMAKE_BUILD_PARALLEL_LEVEL=1 cmake --build --preset releaseWithDebugInfo --parallel 4 --target slangc slang-test render-test test-server`.
GPU suites sequential, at most four CPU workers total, two unit servers.

## Milestones

1. Format fixture before proof; capture fixture, emitter and library hashes with each run.
2. Extend the two identity boundaries, then build and run the focused fixture all three modes.
3. Mirror accepted research script into slice-231-after with only paths/modes changed. Execute
   6,384 family and 6,384 typed-control launches (12,768 total), assemble every runtime PTX,
   preserve all inherited source/input/expectation/output hashes. Compile 80 minimal direct family
   probes; retain 24 separate matrix-prefix E36100/E36107 capability rejections.
4. Run required final gates and exact per-cell preservation; append old failures/transitions/fresh
   proofs for actual fixes, preserving all first-known and diagnostic histories.

## Validation and Acceptance

Focused fixture 3, GPU smoke 4, NVVM/routing/reporter units 479 plus existing Windows skip,
toolkit 18, discovery contracts 6, targeted frozen 321, all discovery 318, complex compile/assembly 6. Compare classification, return_code, complete execution_counts, diagnostic and canonical_shape
for every old identity. Count runtime-correct outcomes only as resolutions. Retain 51 open and six
resolved histories unless fresh proof actually resolves one. Historical denominators 427/72 stay
fixed. Preserve 553 old input hashes and record new fixture as 554; capture all 28 source/12 artifact
hashes. Both source_commit and source_revision name actual tested base 51adf2c8, plus final patch.

## Failure and Recovery

Each attempt uses distinct logs. Stop GPU dispatch on device loss. If final fixture changes after
before proof, reverse exact emitter patch, rebuild and repeat proof before restoring/rebuilding.
Do not edit accepted 230 artifacts. Next independent blocker gets exact diagnostic handoff; do not
expand scope. Use reverse patches for unrelated formatting hunks, never reconstruct tracked files.

## Artifacts and Hand-Off

Raw evidence lives in build/nvvm-loop/slice-231-before and slice-231-after. Commit candidates are
this completed plan, five-part report, design update, runtime-validation.slice-231.json, both census
TSVs, fixture/registration and STATUS. Worker returns ownership explicitly; parent accepts/commits.
