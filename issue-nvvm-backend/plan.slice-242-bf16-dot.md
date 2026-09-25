# Preserve source-ordered BF16 dot on SM80

This ExecPlan follows `.agent/PLANS.md`. The NVVM maintainer exception requires committing completed plans/reports with accepted slices; only the parent accepts and commits this slice.

## Purpose and Observable Result

Support `dot(vector<BFloat16,N>, vector<BFloat16,N>)` for N=2,3,4 through direct NVVM O0/O3, preserving CUDA's positive-zero initial accumulator, lane order, and separate BF16 rounding of every multiplication and addition. The accepted241 baseline stopped at this exact canonical GenericAsm boundary; candidate242 now passes the original workload in all three modes. Acceptance requires correct outputs, not merely a later diagnostic.

## Progress

- [x] 2026-09-25: Read repository/loop instructions, accepted241 and research238/vector contract; accepted base d6c26eb4cf5960ac07feff8158d15163cc2757fc, ABI39.
- [x] 2026-09-25: Qualified prototype9 launches/9SM80 assemblies with independent full-buffer expectations and parent oracle.
- [x] 2026-09-25: Frozen discovery fixture; accepted binaries pass NVRTC and reject directO0/O3 at exact dot2 GenericAsm.
- [x] 2026-09-25: Implemented exact catalog/canonical mapping/provider, ABI40 and real-provider negatives; parent early review accepted before build.
- [x] 2026-09-25: Final source build302/302 passed under4CPU/30minute bound.
- [x] 2026-09-25: Final smoke4, fixture3, exports3 and production9replays passed; independent parent numeric audit agrees.
- [x] 2026-09-25: Six final direct linked IR/PTX controls retain canonical signatures and2N BF16 FMA instructions.
- [x] 2026-09-25: Final smoke/focused/exports/replay, units484 plus1oldskip, toolkit18 and contracts6 passed.
- [x] 2026-09-25: Full frozen452/1356, discovery112/336 and material6 gates complete; exact preservation audit passes.
- [x] 2026-09-25: Completed report/design/manifest/censuses/STATUS handoff and final immutable evidence audit.
- [x] 2026-09-25: Parent independently accepted source, checkpoint, histories and numeric evidence; authorized local commit includes this completed plan.

## Surprises and Discoveries

Prototype/source-generator typo and counterexamples are detailed below. Existing accepted238 inputs have679 records of16 uint32 words with packed four-lane operands in columns0..3 and result column4. Width2/3 projections can consume the firstN lanes without changing those inputs.

## Decision Log

2026-09-25 worker: rank source-ordered BF16 dot first because it is the exact two frozen direct-mode blockers and has independently established public semantics. Exact integer construction ranks next; vector storage requires distinct layout producer repairs and ABI evidence; arithmetic/comparison remains separate. Material runtime is reconsidered and deferred: all six compile/assembly cells pass, but bindings, textures/LUTs, inputs and output oracle are absent. This is an explicit support/correctness cadence override, not material runtime evidence.

## Outcomes and Retrospective

Implementation242/full checkpoint is independently accepted:1692freshcells/1651correct/41unresolved/16resolved histories. Both frozen dot failures resolve; all1646 oldcorrect and559 oldinputs survive, with only those two explained five-field deltas. Prototype9 and production9 check195858fullwords; smoke4/fixture3/exports3/units484+1oldskip/toolkit18/contracts6/material6 pass. Latest accepted full is242, targeted233, zero implementations since full.

## Context and Current Pipeline

Consider `dot(bit_cast<vector<BFloat16,4>>(uint64_t(0x4080404040003f80)), vector<BFloat16,4>(BFloat16(0.5), BFloat16(0.5), BFloat16(1.0), BFloat16(1.0)))`. Canonical `hlsl.meta.slang::dot` selects CUDA `_slang_vector_dot` GenericAsm, whose prelude rounds products and sums in lane order, producing BF16 8.5. In accepted241, this valid producer shape reached NVVM canonical operation planning without a qualified semantic operation. BF16 values already retain semantic identity with physical i16 vectors. Do not repair the frontend/library or interpret arbitrary emitter source text.

## Scope and Non-Goals

Only exact BF16 scalar result and matching BF16 vector operands widths2/3/4. No generic arithmetic/comparison, integer/Half/double conversions, explicit vector Select, storage/pointers/aggregates/resources/globals/parameter groups or external helper ABI. Preserve existing export-vector rejection and scalar239/vector241 behavior. Record any next independent blocker and stop its investigation.

## Architecture and Invariants

Use canonical operation planning/catalog qualification and one dedicated BF16 dot family. Provider emits per-lane BF16 FMA multiplication with negative-zero addend, then BF16 FMA addition with multiplicative one and previous accumulator, starting BF positive zero. Both instructions round individually; do not use Float32 accumulation, contraction, native SM90-only add/mul, or LLVM bfloat. Semantic BF16 format remains distinct from IEEE half and integer physical transport. Catalog validates exact formats, bit widths, lane counts and arity; real provider validates physical operands.

## Interfaces and Dependencies

Expanded descriptor contract requires ABI40. Native Ubuntu24.04, L4SM89/driver580.126.09 target80, CUDA12.9.2/NVRTC12.9.86 LLVM14, matching RelWithDebInfo. Read local slang-build skill and inspected slice-203-env.sh. Four CPU workers maximum total and sequential GPU suites. Accepted artifacts238/240/241 are immutable.

## Milestones

1. Prototype under slice-242-before/prototype: generate width projections retaining all679 original records; independent rational/integer expectations for each width and full returned buffers, NaN classification only. Verify explicit fused/Float32 counterexamples for each width, append bounded cases only if needed. Nine launches and nine SM80 assemblies: sourceNVRTC each width plus rawLLVM O0/O3 each width. Promote only exact full-buffer agreement; discard failed recipes, preserving attempts.
2. Add readable tests/cuda/nvvm-bf16-dot.slang with dynamic nonuniform operands and independently expected finite/zero/subnormal/inf/NaN/cancellation results, register one discovery source (112 total). Run accepted binaries before production edits.
3. Implement provider/catalog/canonical mapping and tests. Inventory every new helper/branch/admission with named failing tests and input-shape audit. Format changed explicit paths, never Slang fixtures, send parent early diff.
4. Build using `CMAKE_BUILD_PARALLEL_LEVEL=1 cmake --build --preset releaseWithDebugInfo --parallel 4 --target slangc slang-test render-test test-server` then final validation.

## Validation and Acceptance

Provider ABI change mandates full checkpoint. Capture final source paths (extend accepted39),12 artifacts and all559 old inputs plus fixture before every gate and after. Final smoke4 FIRST, fixture3, production public widths2/3/4 replay NVRTC/O0/O3 with independent expectations, relevant NVVM/routing/reporter units plus doubleSourceLiteralsRoundTrip (baseline483+one old Windows skip), noinline export3, toolkit18, runnercontracts6, explicit census.slice-195.tsv frozen452/1356, discovery111old/333+new3, material6. Use30-minute suite bounds, unit2servers. Compare exactly classification,return_code,complete execution_counts,diagnostic,canonical_shape, with no duplicates/omissions. Preserve1646 oldcorrect/559oldinputs,43unresolved histories and14resolved histories; move entire histories only when truly resolved. Expected1692 total; if dot resolves both original failures then1651correct/41unresolved/16resolved. Actual evidence governs.

## Failure and Recovery

Keep failures/incomplete runs distinctly. Never edit executing scripts. Fix/revert ordinary regressions at responsible layer; no baseline reset. Stop GPU dispatch on device loss; no reboot/driver/system changes. Failed prototype stays evidence-only. Source changes invalidate affected final evidence. Worker does not commit/push.

## Artifacts and Hand-Off

Raw scripts/logs/sources/IR/PTX/cubins/full buffers/oracles/identities stay ignored build/nvvm-loop/slice-242-before and slice-242-after. Durable plan, five-part report, design, census/addition and runtime-validation.slice-242.json/STATUS record accepted full242. Parent review is complete after explicit checkout release; the authorized local commit includes these records.

## Prebuild Helper, Branch and Admission Inventory

2026-09-25: Prototype9 launches/9SM80 assemblies pass97,929 full words (6,120 active/91,809 preserved). Same680-record input for every width preserves all679 original record payloads, appending reversed case679. Record2 gives source0 versusFP32 0x3880; record679 gives source0 versus both lane-fused/FP32 0x3880 for widths2/3/4. Record1 demonstrates accumulation order for widths3/4. First attempt's missing constructor angle bracket rejected in parsing; generator typo and failure are retained under prototype/attempts. No semantic recipe failed or changed. Parent independently confirms exact-rational full-buffer oracle.

Frozen fixture baseline: NVRTC passes, directO0/O3 reject exact GenericAsm dot2 signature (1/3pass). Fixture hash captured before production edits and may not change.

Every production admission survives pending final tests: (1) ABI40/new BFLOAT16_DOT ID names the expanded contract; (2) one exact canonical GenericAsm spelling-table row reuses `_resolveNVVMSemanticValueOperation` and operation planning; (3) catalog `BFloat16Dot` branch requires scalarBF16 result, two matching BF16 operands widths2..4; (4) `_emitBFloat16Dot` owns the prototype-qualified sequential FMA recipe; (5) provider family dispatch calls that helper. No generic scalar arithmetic, classifier, type-role, frontend, library, ABI-export or storage admission is added. Fixture `nvvm-bf16-dot.slang` fails without mapping/catalog/provider; real `nvvmIRBuilderBFloat16DotContract` fails without descriptor/emission/both-dialect boundaries. Existing vector/scalar contract negatives retain all excluded operations. New test checks every operand role, wrong format/width/lanes/result/arity/null-descriptor operands/physicalHalf mismatch.

Input-shape audit: core BFloat16Type and hlsl.meta's target-selected dot produce the intentional one-block GenericAsm helper; `_isCanonicalNVVMIntrinsicValueHelper` validates that producer shape, `_getNVVMSemanticType` supplies canonical BF16 descriptors, the shared catalog is the single qualified signature source, provider's existing physical validation proves `<N x i16>`. No syntax reconstruction/equivalence/operand graph search or malformed-producer accommodation. Exact BF rounding belongs at semantic provider emission because valid source shape requires target-specific arithmetic. Existing ABI239/241 transport stays unchanged.

2026-09-25 final worker review: exact inventory contains1,692 cells with no missing/duplicates and no unexpected field changes. Both frozen BF16 direct failures move with their complete nested histories; all41 other failures and14 old resolved records remain. All40 final sources/12 artifacts/560 inputs are stable across gates and afterward. Original indexed research238117/research240132/accepted24188 artifacts remain immutable. No compiler source changed after the reviewed successful build. Parent acceptance subsequently passed, with seven independent evidence artifacts retained; the parent owns the authorized local commit. The worker made no commit or push.
