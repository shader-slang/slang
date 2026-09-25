# Preserve BF16 identity in CUDA layout producers

This ExecPlan follows `.agent/PLANS.md`. The NVVM maintainer requires completed plans/reports in
local slice commits; raw experiments and logs remain ignored under `build/`.

## Purpose and Observable Result

Make CUDA reflection and direct CUDA layout queries agree with the actual prelude for BF16 vectors.
For `struct W { uint16_t prefix; vector<BFloat16,4> value; uint16_t suffix; }`, return size12,
alignment2 and offsets2/10 instead of24,8 and8/16. Three-record holders become38/2 with tail36.
The unchanged253 query's five wrong direct results must become correct in both optimization modes.

## Progress

- [x] 2026-09-25: Accepted253 research committed as
      `11afb27a72a931ee0393205ca32df6384d276e7a`; clean checkout, compiler identity remains252.
- [x] 2026-09-25: Inspect all AST vector/matrix query implementations/callers and IR CUDA rule.
      Only CUDA needs element semantics; other vector rules ignore the current BaseType argument.
- [x] 2026-09-25: Declare implementation, adjacent qualification and full acceptance before edits.
- [x] Capture before identity and unchanged failing fixture: NVRTC passes96 values; each direct mode
      has15 measured mismatches (BF4 and row-major2x4/3x4 wrapped/array queries). Actual CUDA device
      probe matches224 layout words plus32 sentinels. Retain the initial FileCheck header failure and
      unanchored-value attempt; final numeric expectations are anchored before production validation.
- [x] Preserve canonical Type* through AST vector/matrix queries; repair AST/IR BF3/BF4 CUDA rules.
      Add48 public-reflection cases and the96-value executable fixture; optimized build passed.
- [x] Add public-reflection and executable CUDA-query regressions; format and build matching targets.
- [x] Pass runtime4, focused5, reflection48, IR CUDA36/Natural36 and253 control preservation.
      Replay21 GPU controls/5376 words, including2 immutable bad-input controls;12 preflight shapes
      and7 NVRTC PTX outputs are unchanged. All96 new fixture values match at each mode.
- [x] Full units1048pass/13skip and semantics1052pass/77skip preserve all1060/1129 prior
      identities, adding only cudaSpecialScalarLayout. Toolkit18 and contracts6 pass.
- [x] Complete full frozen1356/discovery348/material6 checkpoint. Preserve all1701 old outcomes
      and all39 unresolved/18 resolved histories; add3 correct discovery cells, giving1704/1665.
- [x] Complete self-review, evidence, five-part report, design and STATUS; exact local acceptance
      succeeds. Include these completed artifacts in the local slice commit.

## Surprises and Discoveries

The qualified row-major2x4 and3x4 BF matrices inherit the same BF4 alignment defect;4x2 and4x3
are unchanged neighbors. CUDA-device qualification succeeded before production edits. A guarded
edit script expected10 interface implementations but found11; it stopped after the header edit.
The corrected script applied the implementation changes once, and the failed script is retained.

Fresh delegation is unavailable: the253 spawn reached `agent thread limit reached`, and no tool
exists to close those agent contexts. WORKFLOW's local fallback continues with one parent writer
and separate mechanical/oracle acceptance; no fresh independent reviewer is claimed.

## Decision Log

- 2026-09-25, parent: Replace the internal vector/matrix query's lossy BaseType argument with the
  existing canonical Type*. This is simpler than another scalar-kind mapping or BF16 AST type.
  The only synthetic uint2 descriptor query can obtain the existing canonical UInt type from
  TypeLayoutContext::astBuilder. Scalar-size queries retain their existing BaseType interface.
- 2026-09-25, parent: Mirror the actual prelude component-struct layout for BF3/BF4 in both CUDA
  producers. Preserve native BF2 and existing Half padding. Natural rules remain untouched.
- 2026-09-25, parent: CUDA matrices store arrays of row-vector types, and the query interface
  forwards element identity through matrices. Qualify narrow row-major BF matrix neighbors before
  retaining the fix; do not investigate or repair the separate recorded column-major input contract.

## Outcomes and Retrospective

The CUDA BF4 producer defect is repaired. The exact original253 queries and the expanded96-value
fixture are correct at both direct optimization levels and NVRTC. Public reflection48 agrees with
actual CUDA layouts; canonical IR CUDA36 agrees and Natural36 is unchanged. Corrected reflection
packing reproduces immutable correct CUDA input bytes. The2 old bad-input controls remain wrong;
all12 direct storage preflight shapes remain rejected.

Full254 preserves all1701 old corpus outcomes and adds3 correct cells:1704total/1665correct/
39unresolved, retaining18 resolved histories. Units1048pass/13skip and semantics1052pass/77skip
preserve every old identity; runtime4, toolkit18, contracts6 and material6 pass. Targeted233 remains
latest targeted-only acceptance; full cadence resets to0. Rolling implementations are250 BF16
literal correctness,252 material compile-time and254 CUDA layout correctness. Separate oracle and
mechanical acceptance is recorded without claiming a fresh independent-agent review.

Storage representation/admission and matrix orientation remain separate contracts. The next bounded
candidate is physical BF16 storage qualification using the now-correct CUDA metadata; do not infer
storage support from this layout fix.

## Context and Current Pipeline

Research253 qualifies36 CUDA/reflection rows and72 explicit IR rows. Only BF4's vector, wrapper
and array-holder disagree with the prelude. The direct `__sizeOf`/`__alignOf` path is already admitted
as compile-time metadata; its five wrong values are observable without admitting BF runtime storage.
The accepted250/pre252 compiler emits identical PTX, proving the bug predates252.

`_createTypeLayout` receives canonical BFloat16Type but extracts BaseType only for BasicExpressionType,
passing Void to `CUDALayoutRulesImpl::GetVectorLayout`. Preserve Type* through LayoutRulesImpl and
SimpleLayoutRulesImpl vector/matrix interfaces and callers; remove the now-unused ordinary-layout
BaseType extraction blocks. Varying paths still need BaseType for scalar-slot rules, but pass Type*
to vector/matrix queries. CUDA can inspect canonical BasicExpressionType for Half and BFloat16Type
for the component structs. IR `CUDALayoutRules::calcSizeAndAlignment` owns the corresponding rule.
Default matrices call vector layout, so element identity must survive that forwarding as well.

## Scope and Non-Goals

AST/IR CUDA layout producers, meaningful regression tests, one discovery source addition, docs and
acceptance only. No provider ABI change, emitter/query-consumer patch, alternate type representation,
new storage admission, FP8 conversion, natural-layout change, matrix orientation repair or material
runtime claim. Keep all pre-existing fixture/input/oracle identities and histories intact.

## Architecture and Invariants

The existing Type* is the semantic source of truth. No new equivalence relation, AST reconstruction,
scalar-kind mapping or permissive fallback is needed. BF2 remains4/4, BF3 remains6/2 and BF4 becomes
8/2 in CUDA producers. Ordinary ushort4 remains8/8; Half3/Half4 remain padded/aligned8/4. Other target
vector rules ignore the semantic argument exactly as before. Only valid prelude BF3/BF4 widths are
handled by the new CUDA branch; unsupported-width behavior must not be broadened accidentally.
Matrices use their existing vector/array construction and major-order policy. Natural rules remain
distinct, including BF2 alignment2. Downstream aggregate validation and role caches remain strict.

## Interfaces and Dependencies

Expected production files: source/slang/slang-type-layout.h, slang-type-layout.cpp and
slang-ir-layout.cpp. Public reflection regression belongs beside
tools/slang-unit-test/unit-test-special-scalar-reflection.cpp. A new CUDA query fixture near
tests/cuda/cuda-layout.slang will use exact independent integer expectations and all3 modes.
Append one source to discovery (115 becomes116); frozen452 is immutable.

Native Ubuntu24.04/L4SM89, driver580.126.09, CUDA12.9.2/NVRTC12.9.86, targetSM80, LLVM14/providerABI41.
Use the already-read local slang-build skill and inspected203 environment. Baseline optimized
compiler10ffeb3246d56c9a835b1cd606e35a1a2cd6c8f9fcb3f6bfef36fed26413ea7b and provider
5fe0b977e22b80acc5ee39147c69510a01c09563354a1a67bd9573d1cda1aeab match252. At most4 CPUs total,
unit servers2, corpus workers4, sequential GPU suites,30minute suite bounds.

## Milestones

1. Preserve accepted252/253 identities and research controls in new before/after roots. Qualify
   row-major BF matrix neighbors with actual prelude queries, then declare exact fixture inventory.
   Run the unchanged new fixture before implementation: NVRTC correct, direct BF4 query failures.
2. Apply the canonical Type* interface change and the two CUDA layout branches. Add complete-sentence
   comments with a concrete wrapped-record example and the prelude invariant. Audit every changed
   helper/branch and remove obsolete extraction code rather than retaining dead alternatives.
3. Add reflection tests for vector/wrapper/array/neighbor layouts, plus runnable query fixture and
   discovery addition. Format explicit changed files before final builds. Build optimized targets:
   `CMAKE_BUILD_PARALLEL_LEVEL=1 cmake --build --preset releaseWithDebugInfo --parallel 4
--target slangc slang-test render-test test-server`. Use matching Debug assertion checks when
   needed for the changed canonical-type boundary; do not mix configurations.
4. Run runtime4 first, new/focused source controls, public reflection and standalone253 IR proof.
   Require all36 CUDA rows to match actual ABI and all36 Natural rows unchanged. New host packing
   derived from corrected reflection must equal253's immutable correct CUDA-packed input. The old
   reflection-packed bytes remain recorded wrong-input controls, not silently rewritten.
5. Full compiler units and semantic regressions, toolkit18, contracts6, frozen1356 and discovery348
   cells, all6 material compile/assembly checks. Compare all1701 old outcomes exactly; additions
   are reported separately. Full checkpoint required because layout is broadly shared.
6. Complete final source/binary/input hashes, self-review, report/design/STATUS and local acceptance.

## Validation and Acceptance

The new fixture must fail before and pass after at both NVVM optimization levels with the same
source/input/oracle. Reflection must report BF4 W12/2 with offsets2/10 and holder38/2/tail36. Preserve
all33 other253 CUDA rows, all36 Natural rows and public scalar-kind reflection. Require narrow
row-major matrix neighbors to agree with the prelude if their BF4 vector-derived layout changes.
Retain all12 existing253 direct storage-control rejection shapes; query correctness does not admit
runtime storage. Compare old focused PTX/runtime controls where source/options are unchanged.

Full baseline:1060 compiler-unit identities1047pass/13ignore,1129 semantic identities1052pass/77ignore,
frozen1356/1347correct, discovery345/315correct, combined1701/1662correct/39unresolved/18resolved histories.
New expected discovery348 cells gives1704 total,1665 correct if all3 new cells pass. Require exact
missing/extra/duplicate accounting, five-field preservation of all old cells, every old unit identity,
new unit outcomes, final6 material support cells and final tested-source identity. Timings are not
measured speedup claims. No full reset until all preservation checks pass.

## Failure and Recovery

Retain failed probes/builds with exact commands and source states. Diagnose regressions at their
producer; do not weaken expectations or admit unrelated runtime types. Revert or repair within this
bounded layout scope before acceptance. Stop on unresolved regression or GPU loss; no system/driver
change, reboot or push. If matrix qualification reveals an independent issue, preserve its old
outcomes and reassess scope explicitly rather than bundling an orientation/ABI redesign.

## Artifacts and Hand-Off

New raw roots `build/nvvm-loop/slice-254-before` and `slice-254-after`. Completed plan, five-part
report, runtime evidence, census/discovery results, test addition, durable design and STATUS are
committed together. Preserve baseline identities, failed unchanged fixture, exact outputs, standalone
proofs, new/source snapshots, commands and raw artifact index. Record the local-delegation fallback
and do not claim a fresh independent review.
