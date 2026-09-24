# Legalize local Boolean vector lane accesses

This ExecPlan follows `.agent/PLANS.md`. The NVVM maintainer exception requires the completed plan and report in the slice commit. Parent owns acceptance and commit.

## Purpose and Observable Result

Make local bool2/bool3/bool4 lane loads/stores executable through direct NVVM, including the vector isnan/isinf helper shape that currently stops tiled-brass eval_buffer. Preserve independently expected GPU output for dynamic NaN/infinity/finite classification and lane updates.

## Progress

- [x] 2026-09-24: Read workflow, accepted 205, build skill; clean native Linux base c17c9c92e63f55e959cf2b32504215f190b81e9e.
- [x] Audit existing scalar classification and Boolean value-vector tests; they do not exercise local Boolean element addresses.
- [x] 2026-09-24: Final formatted fixtures rerun on deliberately reverted compiler:2/6 pass (NVRTC), four matching Boolean pointer rejections. Fresh focused/material IR retained.
- [x] 2026-09-24: Target legalization and final focused 16/16, runtime 4/4, units 473/473+1skip, toolkit 18/18 pass. All6 complex cells compile/assemble.
- [x] 2026-09-24: Full final checkpoint audited: 1623 fresh cells, 1562 correct, 61 unchanged failures; all 1617 old keys/results/diagnostics/counts preserved and six correct additions.
- [x] 2026-09-24: Self-review, five-part report, portable outcomes, design and STATUS prepared for parent acceptance.

## Surprises and Discoveries

`_getNVVMSequentialElementPointer` intentionally uses numeric vector classifiers. Local Boolean values lower to packed LLVM `<N x i1>` with alignment1, while GEP of scalar i1 advances in bytes. Widening admission alone would be wrong. Existing physical structured-buffer Boolean representation uses i8, but local vectors use the semantic packed representation. The buffer-element lowering pass only discovers resources and UserPointer/Input/Output pointers, and its LLVM policy handles matrix storage, not generic local Boolean vectors. AddressInstElimination rewrites arbitrary address chains/calls and introduces UpdateElement; that broad autodiff pass is not applicable to the bounded target domain.

## Decision Log

2026-09-24 worker: select complex-driven local Boolean lane legalization over independent wave transport. Keep LLVM value/storage/provider ABI unchanged. Use existing IR builders for extract, compare, select and vector construction at the NVVM target legality boundary. Restrict to direct local Var roots and canonical same-element Generic ReadWrite scalar-layout lane pointers with only load/store users. Do not create a physical lane pointer. Unknown/escaping/address-space/layout shapes retain existing preflight rejection. Any need to change shared physical ABI expands beyond this slice.

Before implementation: require FULL checkpoint, because this is target lowering and preservation impact warrants complete verification. Reuse accepted 205 matching before evidence (1617 cells, 1556 correct, 61 failures); run new final fixtures before edits, no redundant old full before.

## Outcomes and Retrospective

Implementation works in all focused modes and all complex compile/assembly cells. Full corpus preservation is complete: no old key, classification, return-code, execution-count or diagnostic changes, no missing/duplicate/extra cells, and no inherited runtime cells. Parent acceptance is complete; this plan is included in the accepted local commit. Material remains unmodified; no material runtime claim or performance claim is authorized by compile/assembly support.

## Context and Current Pipeline

Material line2485 calls any(isnan(eval)) / any(isinf(eval)). Generic vector intrinsics construct bool vectors through scalar GetElementPtr stores. `fixBufferAccessPointerTypes`/IRBuilder emit canonical Ptr<bool,Generic,ReadWrite,ScalarLayout>; target preflight rejects it because packed LLVM vector lanes are not separate scalar storage. `legalizeIRForNVVM` is the proper target-owned boundary to turn these valid semantic accesses into operations on the canonical vector value before preflight. Exact fresh first-failure attribution remains to be recorded; inherited203 IR only corroborates this path.

## Scope and Non-Goals

Only nonescaping load/store accesses to a direct local Boolean vector Var, widths 2–4, i32 indices. Preserve all other pointer admission and physical storage. No buffer/shared/global lane rewriting, no helper-pointer ABI widening, no provider changes, no material edits, wave features or second independent blocker.

## Architecture and Invariants

A local Var stores the existing packed Boolean vector. Lane loads extract from the loaded vector. Lane stores load the vector, select the replacement for the indexed lane and preserve all other lanes, then store the new vector. This is valid only for private local storage with no externally observable concurrent lane writes. The IR semantic type remains source of truth. Canonical pointer equality, access/layout/address-space are checked before rewriting; unsupported shapes are left for diagnostics. No default initialization, pointer reinterpretation, custom equivalence or graph search.

## Interfaces and Dependencies

Reuse local-copyable/derived-pointer/value-vector classifiers and standard IRBuilder methods in source/slang/slang-ir-nvvm-legalize.cpp. CUDA12.9 SM80 on L4; matching RelWithDebInfo compiler/provider/tools; provider ABI 35 unchanged.

## Milestones

1. Add independent tests/cuda fixture(s), fail-before in all3 modes, source hashes and IR evidence.
2. Target legalization; rebuild matching optimized tools; pass-after same fixture bytes and meaningful negative boundaries.
3. runtime 4, focused, NVVM/routing/reporter units 473+1skip, toolkit 18, all6 complex probes, full frozen452 and discovery87+addition identities all3 modes. Sequential suites, shared four CPU workers.
4. Exact old key/classification/return/execution/diagnostic preservation; separate additions. Finish documents and ownership transfer.

## Validation and Acceptance

Build: `CMAKE_BUILD_PARALLEL_LEVEL=1 cmake --build --preset releaseWithDebugInfo --parallel 4 --target slangc slang-test render-test test-server`.
Source optimized environment `build/nvvm-loop/slice-203-env.sh`. Run final gates following WORKFLOW's full commands with four corpus workers/two unit servers, bounded timeouts. All6 complex cells compile/assembly only. New runnable tests require NVRTC O3 and NVVM O0/O3 expected output, dynamic indices and untouched lanes, 2/3/4 lanes, finite/NaN/signed infinity/zero. Negative boundaries retain element/layout/address-space rejection and provider untouched on invalid IR. Discovery registration uses full manifest validation, no frozen overlap or contract edits. Exact final source/binary hashes must match testing.

## Failure and Recovery

On a failing prototype, remove only this slice's changes; retain findings. A regression blocks acceptance and requires fix/revert plus full replay. Stop GPU work on device loss; no drivers/reboot/push. Record next independent complex diagnostic minimally and stop investigation.

## Artifacts and Hand-Off

Raw build/nvvm-loop/slice-206-before and slice-206-after. Completed plan, five-part report, runtime-validation.slice-206.json, census/discovery-census.slice-206.tsv, design facts and STATUS are durable. Parent accepts/commits; worker does not commit.

## Input-Shape and Helper Inventory Review

One production helper survives: `_legalizeNVVMLocalBooleanVectorAddresses`, called once before NVVM preflight. It scans existing GetElementPtr instructions, validates direct local Var ownership and all uses before changing any use, and delegates shape equality/classification to existing helpers. `asNVVMSupportedValueVectorType` guarantees widths 2–4, bounding lanes[4]. `asNVVMSupportedDerivedCopyableValuePointerType` requires Generic, canonical3operand or4operand ScalarLayout spelling; the rewrite further requires ReadWrite, exact Boolean element equality and i32 index. Existing numeric and memory admission functions are byte-unchanged. Nonescaping means every lane-address use is the address operand of Load/Store. Other uses leave the address for preflight rejection; a new O0/O3 diagnostic fixture covers a noinline inout helper escape.

`UpdateElement` was considered: direct NVVM has no consumer, while existing peephole expansion concerns constant aggregate updates, not dynamic vectors. Reuse of the canonical select/extract/make-vector pipeline avoids another provider API or broad new operation support. `AddressInstElimination` additionally walks arbitrary aggregate paths and copies call arguments, outside this slice.

The newly loaded uninitialized vector is not default-initialized. LLVM 14 alloca semantics yield undef on an uninitialized load; select depends on its condition and selected value. For defined in-range index k, lane k selects the replacement without depending on the old undef lane; other lanes select their existing values. Construction uses independent insertelement operations, so undef in a different lane does not taint a defined lane. Inductively every initialized lane stays defined, and after the source loop writes all lanes the whole vector is defined. The runtime rotated-initialization fixture corroborates that proof at O0/O3. No branch or address is computed from uninitialized data. Reference: https://releases.llvm.org/14.0.0/docs/LangRef.html#alloca-instruction and #poison-values.

Fresh material final IR contains vector isinf/isnan functions with local Ptr(Vec(Bool,3)) result Vars and scalar-layout GetElementPtr stores; the fresh focused source reproduces the same producer/consumer shape and diagnostic. This replaces inherited203 corroboration with fresh206 code-shape evidence, without claiming instrumentation pinpointed the first preflight instruction. All6 after probes pass, so there is no next complex compiler blocker; material runtime semantics remain absent.

## Validation Interruption

2026-09-24: Initial formatting invocation lacked setup PATH and failed tool discovery. A dry check with the installed formatter showed two wrapping-only hunks in the new helper. The worker stopped the partial replay safely (runtime/focused/units/toolkit/complex had passed; frozen was incomplete), archived all evidence under `build/nvvm-loop/slice-206-preformat`, applied only scoped new-code formatting and rebuilt. Those interrupted corpus cells are not acceptance evidence. The final suite restarts completely under `slice-206-after`, including standalone escaping-pointer diagnostics. No GPU loss or semantic regression motivated this avoidable replay. Final fixture bytes are unchanged; before proofs are not repeated.

The first standalone diagnostic fixture used a spaced, abbreviated CHECK annotation that the diagnostic harness did not recognize. Both modes produced the intended exact rejection. The annotation was corrected to the harness's exact `//CHECK:` diagnostic, the failed harness log was retained as `negative-annotation-failure.log`, and only that fixture and subsequent pending gates resumed. Compiler code/binaries and both positive source fixtures were unchanged; final negative source hash was refreshed before its successful rerun.

## Final Outcome

All final gates pass: focused 16/16, separate escaping diagnostics 2/2, runtime 4/4, units 473/473+one Windows-only skip, toolkit 18/18 and complex 6/6 compile/assembly. Frozen 452 identities preserve 449/438/438 correct; discovery 89 identities yield 79/79/79 correct. Full 1623-cell replay preserves all 1617 previous cells exactly and adds 6 independently correct cells. Manifest `runtime-validation.slice-206.json` retains exact identities, commands, raw evidence refs, final source/tool hashes and all 61 unresolved failures. Material has no next registered compiler blocker, but application runtime semantics remain absent. Worker implementation is complete; parent owns acceptance and commit; no next slice started.

## Parent acceptance

Accepted 2026-09-24 after reviewing the production diff and boundary tests. Independent raw205/206 comparisons preserved all old classifications, return codes, execution counts, diagnostics and canonical shapes with exact inventories; all six additions pass. All 12 artifact and 30 tested-source hashes match. Full-checkpoint cadence resets to zero.
