# Transport canonical resource values through helper results

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires the completed slice plan to be committed with this experimental backend.

## Purpose and Observable Result

After this slice, a selected direct-NVVM helper may return either an admitted raw-buffer view or
an admitted read-only texture handle by value. These are not new provider representations: raw
buffers retain their existing `{global data pointer, i64 count}` representation and sampled
textures retain their existing `i64` CUDA handle. The same exact canonical type must be accepted
by helper preflight, helper-result type lowering, generic call emission, and generic return
emission.

The slice targets the frozen-v1 workload `compute/reinterpret-structured-buffer.slang` and the
discovery workloads `optimization/func-resource-result/func-resource-result-simple.slang` and
`optimization/func-resource-result/func-resource-result-complex.slang`. It does not widen helper
results to writable surfaces, samplers, resource-bearing user aggregates, or arbitrary opaque
types without a concrete canonical producer and workload.

## Progress

- [x] (2026-08-31) Reconciled the Slice 150 Pareto and selected the one three-workload
  cross-corpus root cause: raw-buffer and sampled-texture values rejected at helper results.
- [x] (2026-08-31) Traced the linked helper signature through `_validateNVVMHelperTarget`,
  `NVVMTypeLoweringContext::lowerType`, generic function declaration, `emitCall`, and
  `_emitNVVMFunctionValueReturn`.
- [x] (2026-08-31) Added exact helper-result classification and focused fake-provider coverage without changing
  provider ABI revision 30.
- [x] (2026-08-31) Proved the simple raw-buffer result workload correct through the real provider
  at O0/O3 and promoted it; captured exact later blockers for the complex texture-array and frozen
  descriptor-handle workloads without widening the slice.
- [x] (2026-08-31) Ran the selected regression, complete frozen-v1 and discovery corpora, representative
  SM70/80/90 measurements, artifact generation, documentation, and self-review.
- [x] (2026-08-31) Finalized the completed implementation, plan, report, and evidence for the
  Slice 151 commit.

## Surprises and Discoveries

- The provider already has every required generic operation. `_lowerRawBufferType` creates the
  same first-class struct used for raw-buffer parameters and body values; sampled textures lower
  to the same 64-bit handle used for parameters and body values. Generic function types, calls,
  and value returns accept arbitrary existing provider type handles.
- Admission is deliberately role-sensitive, and helper results are independently rejected in two
  places: `_isSupportedNVVMHelperResultType` during linked-IR preflight and the `HelperResult`
  branch of `NVVMTypeLoweringContext::lowerType`. Both must express the same exact invariant.
- The fake provider models raw-buffer resource views as a distinct fake struct handle but has not
  admitted that handle as a function result. Its generic call and return validation therefore
  need test-harness support to verify the production path without weakening type checks.
- `func-resource-result-complex` advances from helper-result rejection to an
  `IRLoad<Array<Texture2D, 2>>` produced by its synthesized conventional-global block. This is a
  first-class resource-array value contract, not a helper-result contract.
- `reinterpret-structured-buffer` advances from helper-result rejection to a typed field address
  of `DescriptorHandle` inside the input handle wrapper. That representation belongs to aggregate
  pointer/layout transport and remains outside this slice.
- The generic load diagnostic initially omitted its canonical type. The strict discovery
  summarizer rejected that unaudited shape, so preflight now reports
  `load result type: Array<Texture2D, 2>` and the census owns it at the exact
  `collectEntryPointUniforms -> GlobalParams field -> IRLoad` producer chain.
- The installed `clang-format` is version 21.1.8, outside the repository's accepted 17/18 range.
  The slice therefore uses manual style review and `git diff --check` rather than an unsupported
  formatter rewrite.

## Decision Log

- Decision: define Slice 151 by first-class canonical resource-value transport across helper
  results, limited to raw buffers and read-only textures.
  Rationale: all three motivating failures share the same producer and existing physical value
  representation. Adjacent resource categories have no selected failing workload in this slice.
  Date/author: 2026-08-31, Codex.
- Decision: change compiler-side role classification and the fake provider only; retain provider
  ABI revision 30.
  Rationale: generic function, call, return, struct, pointer, and integer operations already
  express the required LLVM IR exactly.
  Date/author: 2026-08-31, Codex.
- Decision: reuse `getNVVMSupportedRawBufferType` and
  `getNVVMSupportedReadOnlyTextureType` as the sole canonical classifiers.
  Rationale: these functions already prove the source type, element/layout contract, access, and
  executable representation. Adding a second resource matcher would create competing truth.
  Date/author: 2026-08-31, Codex.
- Decision: treat the two newly exposed blockers as measured cascades rather than broadening Slice
  151.
  Rationale: neither the conventional resource-array load nor the descriptor-handle field pointer
  is part of helper result transport; admitting either here would merge independent invariants.
  Date/author: 2026-08-31, Codex.

## Outcomes and Retrospective

Frozen corpus v1 remains exactly 452 workloads and 427 healthy MVP references with O0/O3/both
correctness 372/376/372 and zero old-correct regressions. The frozen helper-result row advances to
its independent `DescriptorHandle` field-address blocker, so no frozen numerator changes.

Discovery remains 82 workloads and 72 healthy native references. O0/O3/both correctness reaches
52/52/52, up one with zero old-correct regression. The newly correct identity is
`optimization/func-resource-result/func-resource-result-simple.slang#discovery-1`; the complex row
advances to the exact `Array<Texture2D, 2>` load blocker. Each direct mode now has 52 correct, 21
preflight, one provider, seven infrastructure, and one runtime-mismatch result. The selected NVVM
prefix passes 427/427.

The new resource-result gate measures 357.8 ms/8705-byte PTX for NVRTC O3, 238.9 ms/2541 bytes for
direct O0 SM70, and 243.5 ms/732 bytes for direct O3 SM70. CUDA 12.9 `ptxas` accepts all ten
discovery measurement gates at direct O3 for SM70, SM80, and SM90. Provider ABI revision 30 remains
unchanged.

## Context and Current Pipeline

After specialization and linking, each motivating source contains an ordinary `IRFunc` whose
result type is either `RWStructuredBuffer<...>` or `Texture2D`. `_validateNVVMHelperTarget`
inspects that exact linked signature before any provider mutation. It currently calls
`_isSupportedNVVMHelperResultType`, which accepts helper values and selected pointers but not the
resource classifiers already accepted for helper parameters and ordinary values.

For each accepted function, `emitEntryPointDirectlyViaNVVM` asks
`NVVMTypeLoweringContext::lowerType` for `NVVMTypeUse::HelperResult`. That role repeats the same
rejection even though later lowering already knows how to produce the value type. Function
declaration passes the resulting handle to `NVVMIRBuilder::getFunctionType`. Body returns flow
through `_getLoweredNVVMValue` and `_emitNVVMFunctionValueReturn`; callers flow through
`_getLoweredNVVMHelperValue` and the generic `emitCall`. None reconstruct source syntax or inspect
fixture names.

For raw buffers, `_lowerRawBufferType` is the canonical producer of the provider struct containing
a global element pointer and 64-bit count. For read-only textures,
`getNVVMSupportedReadOnlyTextureType` proves the canonical source type and type lowering produces
the existing 64-bit CUDA texture handle. Helper transport must preserve those representations
unchanged.

## Scope and Non-Goals

In scope are exact raw-buffer and read-only-texture helper results, synchronized role
classification, focused fake-provider type/call/return coverage, real O0/O3 differential
validation, selective permanent promotion, complete cross-corpus measurement, and durable
documentation.

Out of scope are surface and sampler results, resource aggregates, parameter-block results,
opaque handles not recognized by the two classifiers, new provider callbacks, provider ABI
changes, source-syntax reconstruction, fixture-name checks, compatibility fallbacks, diagnostic
weakening, frozen-corpus identity changes, speculative feature implementation, and corpus v2.

## Architecture and Invariants

- Only an exact type accepted by `getNVVMSupportedRawBufferType` or
  `getNVVMSupportedReadOnlyTextureType` is newly legal in a helper result.
- Preflight and type lowering use the same canonical classifiers and admit the same set.
- The helper result representation is identical to the existing ordinary-value and
  helper-parameter representation for that source type.
- Generic function declaration, call, and return operations remain type-exact. The fake provider
  must reject mismatched result values rather than treating all resource handles as interchangeable.
- No provider callback or ABI revision is justified.
- Frozen corpus v1 and discovery keep separate denominators and reports.

## Interfaces and Dependencies

Production changes are limited to `source/slang/slang-emit-nvvm.cpp` and
`source/slang/slang-emit-nvvm-type-lowering.cpp`. Focused test-fixture changes may touch
`tools/slang-unit-test/unit-test-nvvm-support.h` and
`tools/slang-unit-test/unit-test-nvvm-emitter.cpp`. Stable repository tests may gain direct O0/O3
directives only after correct differential results. The isolated LLVM 14 provider remains ABI
revision 30 and requires no rebuild unless validation proves otherwise.

## Milestones

1. Add a focused fixture in which a raw buffer and a read-only texture each cross an ordinary
   helper result and are consumed after the call. Extend only the fake result-kind bookkeeping
   needed to prove exact generic function/call/return transport.
2. Admit the two canonical classifiers in `_isSupportedNVVMHelperResultType` and the
   `NVVMTypeUse::HelperResult` legality branch, with no new structural matcher.
3. Build and run focused fake-provider coverage, then run all three corpus workloads at direct O0
   and O3. Promote workloads that become correct and record any independent newly exposed first
   blocker without broadening scope.
4. Promote only stable correct direct lanes, run the selected prefix and both complete corpora,
   and regenerate separate TSV/JSON/Pareto evidence.
5. Run representative PTX compile-time/size/assembly measurements where practical, update the
   plan/report/design/ledger, perform the input-shape and unprincipled-change audits, and commit
   Slice 151.

## Validation and Acceptance

All CMake builds and tests run outside the sandbox with Windows-native tools as required by
`AGENTS.md`. Acceptance requires:

- focused fake-provider coverage observes exact resource result types through function
  declaration, generic call result, and value return;
- every promoted workload is differentially correct through the real provider at O0 and O3, and
  every advanced-but-not-correct workload has its exact new canonical shape, producer, and
  diagnostic recorded;
- the selected direct-NVVM regression prefix passes with zero old-correct regression;
- frozen-v1 remains exactly 452/427 and reports O0, O3, both, classifications, and regressions;
- discovery remains exactly 82/72 and reports O0, O3, both, classifications, Pareto, and newly
  unlocked workloads;
- representative SM70, SM80, and SM90 PTX assembles where the established measurement harness
  permits;
- provider ABI remains revision 30;
- `git diff --check`, JSON/TSV integrity, and self-review pass, with no staged content from
  `external/slang-binaries/`.

## Failure and Recovery

If one motivating workload reaches a new first blocker, record it as an advanced row and do not
admit another type or operation unless it is the same canonical invariant. If LLVM verification
or libNVVM compilation rejects one existing representation as a helper result, retain the
preflight gate for that type, revert its role widening, and record the provider evidence; do not
patch generated text or add a fallback. Generated outputs under `build/` are reproducible.

## Artifacts and Hand-Off

Commit the completed plan because the user explicitly requires plans and implementation together
for this experiment. Also commit the implementation, focused tests, promoted stable directives,
Slice 151 frozen-v1 and discovery snapshots/reports, and durable design/ledger updates. The final
report must distinguish newly correct rows from workloads that merely advanced to a later blocker
and must record the exact canonical producer for every retained widening.
