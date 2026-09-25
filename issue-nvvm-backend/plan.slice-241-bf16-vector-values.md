# Admit BF16 vector values and internal helpers

This ExecPlan follows `.agent/PLANS.md`. The NVVM maintainer requires completed plans and reports committed with each accepted slice.

## Purpose and Observable Result

Widths 2, 3 and 4 of canonical BFloat16 vectors compile and run through direct NVVM as register values and by-value internal helper arguments/results. Construction, splat, extraction, helper branch/phi, Float32 component conversion and existing lowered bit transport preserve every lane. ABI39 expands the semantic provider descriptor contract.

## Progress

- [x] 2026-09-25: Read workflow, status and accepted research240. Selected bounded value implementation on base 3e218a92a6c3b3c2cdc02fdf854f03b9712e6dda.
- [x] 2026-09-25: Freeze readable discovery fixture; accepted binaries pass NVRTC and reject direct O0/O3 at helper BF2 result. Parent review requested mixed operands, multi-lane swizzle and runtime index coverage; extended fixture and repeated the same before-proof before any build.
- [x] 2026-09-25: Implement explicit value roles and per-lane conversion; parent reviewed full diff and every admission. Formatted explicit C++ paths and restored unrelated historical whitespace.
- [x] 2026-09-25: Smoke4, fixture3, exports3, exhaustive public9/raw6, units483+oldskip, toolkit18, contracts6, frozen1356/discovery333 and material6 completed.
- [x] 2026-09-25: Exact preservation audit, completed report/design/status and parent handoff prepared. Independent parent acceptance passed after checkout release; local commit is authorized.

## Surprises and Discoveries

Research240 establishes physical i16 vectors for values, but BF3 LLVM allocation8/alignment8 differs from CUDA6/2, and BF4 LLVM alignment8 differs from CUDA2. Existing BF4 AST/IR layout producers also incorrectly model8. Storage is deliberately deferred.

## Decision Log

2026-09-25 worker: rank BF16 vector values first because frozen scalar-bf16 stops on a vector helper signature. Dot and integer construction remain separate research238 contracts. Material6 compile/assembly already passes; absent bindings/textures/LUT/input/oracle justify support/correctness cadence override. Keep canonical vector Select rejected; accepted research qualifies branch/phi only. No frontend, standard-library or runner edits.

## Outcomes and Retrospective

Implementation and final full checkpoint are accepted after independent parent review. All1,643 old correct cells survive; the new fixture adds3 correct cells. Final totals1,689fresh/1,646correct/43unresolved/14resolved histories. The two motivating direct cells remain preflight failures at `_slang_vector_dot`; vector success does not resolve them. Full241 is now the accepted checkpoint; cadence resets to zero. The scope stayed value-only and every qualified operation passed independent full-buffer checks. Export review caught and closed an otherwise unintended ABI admission before final validation.

## Context and Current Pipeline

Consider `vector<BFloat16,3> v = vector<BFloat16,3>(b0,b1,b2); float3 f = float3(v);` and a noinline helper returning one of two such inputs via an if branch. Core-library BFloat16Type remains semantic truth. Canonical MakeVector/MakeVectorFromScalar, typed Swizzle, vector FloatCast and helper signatures reach direct NVVM preflight unchanged. `_getNVVMVectorConstruction`, `_getNVVMSequentialElement`, `_getNVVMSemanticType` and helper signature validation own admission. `NVVMTypeLoweringContext::lowerType` maps explicit Value/HelperParameter/HelperResult roles to `<N x i16>`. Provider BFloat16Convert applies the scalar SM80 recipe per lane. `BitCastLoweringContext::processBitCast/readObject` already reduces uint32/BF2, uint64/BF4 and ushort3/BF3 to scalar transport and construction: no producer repair is needed.

## Scope and Non-Goals

Only widths2/3/4 register and internal by-value helper roles. No new local pointers/storage, external ABI interoperability, recursive aggregates/resources/globals/parameter groups, arithmetic/comparison/dot, integer/Half/double casts, or canonical vector Select. Scalar239 unchanged.

## Architecture and Invariants

BF16 remains distinct from IEEE Half and generic numeric/copyable/recursive helper classifiers. A named exact register-vector qualifier may combine established ordinary vectors with qualified BF16 widths for construction/extraction/semantic description only. Type role admission must reject BF16 vector storage before physical lowering. Conversion requires exact matching lanes1..4 and width16BF/width32Float; each lane uses existing scalar narrowing RN and high-word widening. Narrowing NaNs classification-only; widening payload exact for SM80.

## Interfaces and Dependencies

ABI38 becomes39; compiler-core semantic catalog/API, direct emitter/type lowering and real LLVM provider change together. LLVM14/libNVVM12.9 SM80 on L4SM89; Ubuntu24.04 driver580.126.09, CUDA12.9.2/NVRTC12.9.86. Inspected slice-203-env.sh chooses matching RelWithDebInfo. Four CPU workers total, sequential GPU suites, 30-minute bounds.

## Milestones

1. Add tests/cuda/nvvm-bf16-vector-values.slang and discovery manifest addition only; baseline proof under slice-241-before.
2. Explicit BF vector role qualifier, operation admission and semantic descriptor; per-lane provider conversion; real-provider positive and malformed descriptor negatives, source storage negatives. Inventory every branch before final build.
3. Format explicit changed C++ paths using extras/formatting.sh; parent early review. Build `CMAKE_BUILD_PARALLEL_LEVEL=1 cmake --build --preset releaseWithDebugInfo --parallel 4 --target slangc slang-test render-test test-server`.
4. Run final gates and record immutable evidence under slice-241-after.

## Validation and Acceptance

Shared type/provider ABI change mandates full checkpoint. Capture all baseline37 source/12 artifact/558 input hashes, extend provenance for touched/new files; capture before every gate and after. First smoke4, then fixture3 and exhaustive9 public projections (remove local helper/copy and writes29..29+N-1; use original input sentinels there). Preserve original240 artifacts and independently check every output word against the projected oracle; optionally replay6 raw controls with original oracle. Run all NVVM/routing/reporter units and doubleSourceLiteralsRoundTrip, toolkit18, runnercontracts6, frozen explicit census.slice-195.tsv452/1356, discovery110old/330 plus new3, material6. Compare classification, return_code, complete execution_counts, diagnostic and canonical_shape; preserve1643 old correct,43 unresolved histories,14 resolved histories and558 old inputs. Expected1689 total/1646correct if no old fix, actual evidence owns truth.

## Failure and Recovery

Keep attempts immutable. Stop GPU dispatch on device loss; no drivers/reboot/system changes. Fix ordinary regressions at responsible layer, never weaken oracle or reset baseline. Any final source change invalidates affected evidence and requires final checkpoint. No worker commit/push.

## Artifacts and Hand-Off

Completed plan, five-part report, design update, compact runtime-validation.slice-241.json, census outputs and STATUS accompany parent review. Raw sources/IR/PTX/buffers/scripts/hashes/logs stay ignored build/. Parent accepts and commits after checkout release.

## Prebuild Helper and Admission Inventory

All entries survive; no fallback is added. `asNVVMBFloat16VectorType` checks the producer-owned BFloat16Type and exact literal widths2..4; `asNVVMRegisterVectorType` combines that with existing ordinary vectors without modifying recursive classifiers. The frozen fixture's make/splat/mixed BF2+scalar and BF2+BF2 constructors, scalar and multi-lane swizzles and runtime index exercise its construction/extraction consumers. Helper signatures, explicit phi preflight, value availability and role-cache admission are required by dynamic choose2/3/4 and exhaustive public projections. `NVVMTypeInfo::supports` admits Value/HelperValue/HelperParameter/HelperResult only; source inout BF2, nested BF3 aggregate and BF4 resource negatives keep storage closed. `_getNVVMSemanticType` retains format identity; the semantic catalog validates width/lane matching; `_getSemanticLLVMType` creates i16 vectors; `_emitBFloat16ConvertLane` extracts the existing scalar recipe for lane reuse. `nvvmIRBuilderBFloat16VectorContract` and fixture Float32 conversion fail without these changes. Provider descriptor negatives independently exclude Select, arithmetic/comparison, integer/Half/double conversion, malformed widths/lanes/arity and mismatched physical operands. ABI39 makes the expanded descriptor contract explicit.

The input-shape audit finds canonical intentional producer output throughout. Core semantic BF16 and existing lowered BitCast are the sources of truth; no syntax reconstruction, graph rediscovery, alternative equivalence or producer repair is introduced. Storage predicates and alignments are unchanged. The ordinary vector constructor's SwizzleSet branch keeps its existing classifier and rejects BF vectors. Each preserved scalar recipe remains byte-for-byte equivalent apart from helper extraction. Parent early review corrected coverage before the expensive build; final linked IR verifies that the relevant source shapes survive simplification.

2026-09-25: Parent ABI review identified reachable CudaDeviceExport helpers using the same by-value path. Provisional result/parameter probes emitted BF3 `.visible .func` with eight-byte/alignment-eight payloads. Added explicit result and parameter exclusions in `_validateNVVMHelperTarget`, with two frontend-valid negatives and existing noinline.slang neighboring export checks. Canonical export input is valid, but external CUDA ABI is unqualified; preflight owns this rejection. Final incremental build succeeded. Initial provisional identity capture failed on an accidentally tuple-valued JSON key; corrected before gates. No provisional compiler hash is available, so that PTX is investigation evidence only. Final gate captures are complete.

The original fixture retains mixed BF3/BF4 MakeVector operands, multi-lane Swizzle and dynamic GetElement in the final preflight IR (provisional-trace/fixture.log lines102828-102860); no additional fixture mutation was necessary.

2026-09-25: Final smoke4, fixture3, exported-int neighboring3, exhaustive9 public +6 raw controls, units483 with one old Windows skip, toolkit18 and runnercontracts6 pass. Independent worker/parent integer oracles agree on52,696,815 words:20,712,770 active and31,984,045 preserved. All132 accepted240 artifacts and558 prior input hashes remain unchanged. Final linked IR preserves mixed constructor/swizzle/dynamic extraction; all three exhaustive helpers retain BF vector phi merge parameters. At this intermediate milestone, the frozen checkpoint was running and O0 reached `_slang_vector_dot`. The completed exact comparison below confirms the same two expected direct-mode transitions and no other deltas.

Final acceptance review: every requested identity/mode is present exactly once; all five outcome fields match239 except the two explained BF16 diagnostic/canonical-shape transitions. Complete prior failure records and all14 resolved histories remain. All39 final source,12 artifact and559 input hashes are stable across gates and afterward; all558 old inputs are exact. Independent parent review verified the final diff,1,303 evidence references, all identities/histories and52,696,815 replay words. Completed implementation/report/design/status are accepted for local commit. No GPU loss, system/driver change, reboot, worker commit or push occurred.

- [x] 2026-09-25: Parent independently accepted full241 after exact preservation, complete-buffer and artifact audits; see the six parent evidence references in validation241.
