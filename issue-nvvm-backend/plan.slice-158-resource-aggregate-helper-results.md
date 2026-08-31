# Transport canonical resource aggregates through helper results

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires the completed slice plan to be committed with this experimental backend.

## Purpose and Observable Result

After this slice, an ordinary direct-NVVM helper may return a finite non-empty struct accepted by
`asNVVMSupportedResourceStructType`. The result keeps the same first-class LLVM struct
representation already used for that exact type as a helper parameter and an ordinary SSA value.
The shortest end-to-end observations are the frozen-v1 workload
`language-feature/dynamic-dispatch/optional-single-concrete-layout.slang`, whose `makeValue`
helper returns `{UInt, FooImpl}` and whose `FooImpl` contains a `StructuredBuffer<int>`, and the
discovery workload `language-feature/types/opaque/return-opaque-type-in-struct.slang`, whose
`getThings` helper returns `{int, RWStructuredBuffer<int>}`.

## Progress

- [x] (2026-08-31) Selected the shared cross-corpus first blocker from the Slice 156 census and
  captured final linked IR for both motivating workloads.
- [x] (2026-08-31) Audited the canonical producer and existing classifier/type-lowering
  contracts; helper-result admission is the only role asymmetry before generic provider use.
- [x] (2026-08-31) Added exact compiler-side admission and focused fake-provider coverage without
  changing the provider ABI.
- [x] (2026-08-31) Built and validated both motivating workloads at direct O0/O3, promoting only
  stable correct lanes and recording any newly exposed independent blocker.
- [x] (2026-08-31) Ran the selected regression prefix, complete frozen-v1 and discovery corpora,
  representative measurement gates, artifact integrity checks, formatting attempt, and
  self-review.
- [x] (2026-08-31) Finalized artifact integrity, formatting evidence, the completed plan, and the
  Slice 158 commit candidate.

## Surprises and Discoveries

- The final linked `Optional<IFoo>` payload is not an opaque interface value. Type-flow lowering
  produces `%Tuple = { UInt tag, %FooImpl payload }`, and `%FooImpl` contains the selected
  `StructuredBuffer<int>` handle. `makeValue` constructs and returns that exact tuple and both
  caller paths extract its exact fields.
- `getThings` returns the source struct unchanged after parameter-group lowering:
  `%Things = { Int first, RWStructuredBuffer<Int> rest }`. The constant-buffer load, helper
  return, call result, field extraction, and structured-buffer access all use that exact type.
- `asNVVMSupportedResourceStructType` is already the recursive, finite classifier for these
  structs. Helper parameters and ordinary values already accept it, and the fake provider already
  validates its `ScalarStruct` handle as a generic function result, call result, aggregate
  construction, extraction, load, and value return. No new representation or provider operation
  is needed.
- The first frozen probe advanced beyond helper-result admission to `makeStruct(%Tuple)`. The
  canonical producer is type-flow lowering's ordinary tagged-union construction, and
  `_getNVVMAggregateConstruction` already owns explicit ordered `makeStruct` values but omitted
  the same resource-struct classifier. This is the construction half of the exact representation
  admitted by the slice, not a new IR shape or adjacent resource family.
- The promoted source's two direct CUDA lanes pass, while a whole-file run also executes a
  synthesized WebGPU lane that fails on this machine with invalid bind-group-layout diagnostics.
  The failure is unrelated to the direct CUDA path and is retained as infrastructure evidence.
- `extras/formatting.sh --modified` cannot run on this machine because `gersemi`, `clang-format`,
  `prettier`, and `shfmt` are absent from `PATH`. Manual style review and `git diff --check` are the
  available formatting evidence.

## Decision Log

- Decision: define Slice 158 as exact resource-struct transport through helper results, using
  `asNVVMSupportedResourceStructType` as the sole classifier.
  Rationale: the two selected failures have different source features but the same canonical
  producer/type/operation shape. Reusing the existing classifier keeps one source of truth.
  Date/author: 2026-08-31, Codex.
- Decision: change only compiler-side role admission and focused test coverage; retain provider
  ABI revision 30.
  Rationale: the existing generic struct, function, call, return, construction, and extraction
  operations already express the exact representation.
  Date/author: 2026-08-31, Codex.
- Decision: do not include nested parameter blocks, borrowed aggregate parameters,
  append/consume buffers, double-indirect pointers, BFloat16/FP8, or arbitrary opaque results.
  Rationale: those are distinct first canonical shapes and producers. Their failures do not prove
  that helper-result resource-struct admission owns them.
  Date/author: 2026-08-31, Codex.
- Decision: include canonical `makeStruct` construction when its exact result is accepted by
  `asNVVMSupportedResourceStructType`.
  Rationale: the frozen motivating helper constructs its return value from an explicit ordered
  field list. The generic aggregate provider operation already owns this shape; retaining a
  construction-only rejection would make the newly admitted result representation internally
  inconsistent.
  Date/author: 2026-08-31, Codex.

## Outcomes and Retrospective

Discovery `return-opaque-type-in-struct` is correct at direct O0 and O3 and gains two permanent
lanes. Discovery remains exactly 82 workloads/72 healthy references and improves from 59/59/59 to
60/60/60, with zero old-correct loss. Its one-row healthy helper-aggregate-result cluster is
eliminated.

Frozen corpus v1 remains exactly 452 workloads/427 healthy MVP references and unchanged at
384/388/384 O0/O3/both correctness, with zero old-correct loss. Its selected optional-resource row
advances first through helper-result admission and then through canonical `makeStruct` admission to
`defaultConstruct<StructuredBuffer<int>>`, produced by
`TypeFlowSpecializationContext::specializeMakeOptionalNone` and rejected by
`_validateNVVMFunction`. That independent null-resource operation remains for a later slice and is
not counted as correct.

The selected unit prefix passes 427/427. All fourteen measurement gates assemble with CUDA 12.9
at direct O3 for SM70, SM80, and SM90. The new gate measures 252.1 ms/875-byte PTX at direct O3
SM70 versus 387.0 ms/8841 bytes through NVRTC O3; direct O0 is 247.9 ms/2807 bytes. Provider ABI
revision 30 remains unchanged.

## Context and Current Pipeline

After specialization, type-flow lowering, parameter-group lowering, and linking, each motivating
source has an ordinary `IRFunc` returning an `IRStructType`. In
`optional-single-concrete-layout.slang`, the producer creates `%Tuple` with a UInt tag and a
`%FooImpl` resource-bearing payload; `makeValue` uses `makeStruct`, returns it with `return_val`,
and the entry point calls the helper twice. In `return-opaque-type-in-struct.slang`, parameter-group
lowering preserves `%Things = {Int, RWStructuredBuffer<Int>}`; `getThings` loads it from the
constant buffer and returns it, and `test` consumes both fields after its call.

`_validateNVVMHelperTarget` first calls `_isSupportedNVVMHelperResultType` on the exact linked
signature. That function accepts ordinary helper values, selected pointers, raw buffers, and
sampled textures, but not the resource-struct classifier already accepted by helper parameters.
After preflight, `emitEntryPointDirectlyViaNVVM` asks
`NVVMTypeLoweringContext::lowerType` for `NVVMTypeUse::HelperResult`; its role legality repeats the
same omission even though the same exact type is legal for `HelperParameter` and `Value`.

Once admitted, resource-struct lowering recursively lowers the source fields into one provider
struct handle. Function declaration passes it to `NVVMIRBuilder::getFunctionType`; calls and
returns use the existing generic `emitCall` and `emitValueReturn`; construction and extraction use
the existing aggregate operations. No later consumer needs source syntax or a fixture identity.

## Scope and Non-Goals

In scope are exact resource-bearing structs selected by
`asNVVMSupportedResourceStructType`, synchronized helper-result preflight and type legality,
focused fake-provider coverage, real O0/O3 differential validation, stable test promotion, full
cross-corpus reporting, and representative measurement updates where useful.

Out of scope are arbitrary aggregates, resource arrays rejected by the classifier, parameter-block
or constant-buffer objects as helper results, surface/sampler result widening beyond fields already
owned by the resource-struct classifier, new provider callbacks, provider ABI changes,
source-syntax reconstruction, fixture-name checks, compatibility fallbacks, downstream text
patching, diagnostic weakening, frozen-corpus identity changes, and corpus-v2 activation.

## Architecture and Invariants

- A newly legal helper result must be the exact non-empty finite `IRStructType` accepted by
  `asNVVMSupportedResourceStructType`; no second recursive matcher is introduced.
- Preflight and `NVVMTypeUse::HelperResult` legality must admit the same exact classifier.
- A resource struct has one first-class provider representation for helper parameters, helper
  results, and ordinary SSA values. Function/call/return types remain identical.
- Generic construction and extraction preserve field order and exact recursively lowered field
  types. No ABI packing, source reconstruction, or type equivalence is added.
- Frozen corpus v1 remains exactly 452 workloads/427 healthy references, and discovery remains
  exactly 82/72. Their denominators and metrics remain separate.

## Interfaces and Dependencies

Production changes are limited to `source/slang/slang-emit-nvvm.cpp` and
`source/slang/slang-emit-nvvm-type-lowering.cpp`. Focused coverage reuses
`kDirectNVVMResourceStructHelperSource` in
`tools/slang-unit-test/unit-test-nvvm-support.h` and
`nvvmSlangResourceStructsCrossLocalAndHelperBoundaries` in
`tools/slang-unit-test/unit-test-nvvm-emitter.cpp`. Correct repository workloads may gain direct
O0/O3 directives. The isolated LLVM 14 provider remains ABI revision 30.

## Milestones

1. Extend the existing resource-struct fake fixture with an identity helper that returns its exact
   resource-bearing parameter, then assert its function result and call result are the same
   `ScalarStruct` provider type.
2. Add `asNVVMSupportedResourceStructType` to `_isSupportedNVVMHelperResultType` and the
   `NVVMTypeUse::HelperResult` legality branch, without adding another helper or fallback.
3. Build and run the focused unit test, then measure both motivating workloads through direct O0
   and O3. Promote only rows that are differentially correct and record exact cascades otherwise.
4. Run all promoted file directives, the selected 427-test NVVM prefix, full frozen-v1 and
   discovery censuses, and generate separate TSV/JSON/Pareto artifacts.
5. Run representative compile-time/PTX-size/PTX-assembly measurements where practical; update the
   report, design, capability ledger, and plan; perform the input-shape/unprincipled-change audit;
   then commit Slice 158.

## Validation and Acceptance

All CMake builds and tests run outside the sandbox with Windows-native tools as required by
`AGENTS.md`. Acceptance requires:

- focused fake-provider coverage proves the exact resource struct appears as a helper function
  result and generic call result, and is consumed through ordinary aggregate extraction;
- every promoted workload compares correctly with native CUDA at direct O0 and O3;
- any advanced-but-not-correct workload records its new exact first canonical IR shape, producer,
  and diagnostic without broadening this slice;
- the selected NVVM regression prefix passes with zero old-correct regression;
- frozen-v1 remains exactly 452/427 and discovery remains exactly 82/72, with separate O0/O3/both,
  classifications, Pareto, and newly-unlocked reporting;
- representative direct-O3 PTX assembles for SM70, SM80, and SM90 where the established harness
  permits;
- provider ABI revision remains 30; and
- formatting is attempted, `git diff --check` passes, JSON/TSV identities are intact, and
  `external/slang-binaries/` is not staged.

## Failure and Recovery

If either workload advances to an independent blocker, preserve the narrow admission, record the
new producer/type/operation/diagnostic, and do not widen the slice. If LLVM verification or
libNVVM compilation rejects the existing first-class struct representation, revert the admission
for that class and retain the evidence rather than patching emitted text. Generated files under
`build/` are reproducible and may be replaced by rerunning the established census scripts.

## Artifacts and Hand-Off

Commit this completed plan because the user explicitly requires plan and implementation together
for the experiment. Also commit the implementation, focused coverage, stable promoted directives,
Slice 158 frozen/discovery snapshots and Pareto JSON, any refreshed representative measurement
manifest, a five-part report, and durable design/capability-ledger updates. Generated raw logs and
IR dumps remain under `build/`.
