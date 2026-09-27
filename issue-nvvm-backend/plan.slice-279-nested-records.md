# Qualify nested internal FP8 and BF16 records

This ExecPlan follows `.agent/PLANS.md` and the NVVM committed-plan exception. Authorized development
loop active; no Slack/push/install/system changes. Root owns acceptance, records, builds/gates and
local commit. Fresh279 worker again unavailable at thread limit; reused worker initially owns only
read-only source tracing. Assign one source writer explicitly after the prototype gate.

## Purpose and Observable Result

Allow a qualified internal FP8/BF16 record to compose inside another local record while preserving its
bit encodings, integer guard fields, by-value transport and mutable local storage. Slice270 deliberately
stopped at flat records. An existing nested negative unit confirms the current boundary, but a real
GPU oracle and canonical IR trace must establish the exact extension before production edits.

## Progress

- [x] 2026-09-27: read WORKFLOW/STATUS; accepted277 remains installed after research278 commit0043e8d17.
- [x] Locate existing nested negative and classifier/layout/field-access consumers; request read-only trace.
- [x] Freeze two prototypes, expected outputs/layouts, source/runtime identities and six mode cells.
- [x] NVRTC2passes with full outputs; NVVM4preflight failures retained. Four direct IR dumps prove
      canonical nested value/return and OutParam/local-field shapes. Dedicated classifier plus parent
      local-storage permission propagation selected; no upstream representation rewrite needed.
- [x] Save and verify100-entry accepted277 installed layout with symlinks/modes/configuration/pins.
- [x] Final depth-three fixture99d38 frozen; all65536 encodings, observable integer-only child copy,
      final before evidence: NVRTC passes and NVVM O0/O3 reject OutParam. Guard omission control fails.
- [x] Implement dedicated nested admission/local-role propagation plus reproduced provider store correction.
- [x] Final focused native14/14 and GPU18/18 pass; six full independent regression buffers verified.
- [x] Full277 comparison, native maps, smoke/material/toolkit/contracts and final source/artifact audit.
- [x] Complete compact plan/report/validation and contract/navigation; format/review and prepare accepted local commit.

## Context and Current Pipeline

`nvvmSlangFloat8UnsupportedRolesStopBeforeEmission` includes `struct Outer { Payload value; }` where Payload
contains FP8. A noinline function returning Outer stops at `helper function result type`.
`asNVVMSupportedSubstandardRecordType` admits only integer scalar fields plus FP8/scalarBF16/BF2 leaves,
with at least one substandard field. It is separate from recursive general helper types because those
also authorize device/resource roles. Record values flow through existing struct construction/extract,
call/return and phi operations. Local Ptr/OutParam/BorrowInOut roots have their own exact qualification.
`_getNVVMAggregateStorageLayout` already walks canonical fields recursively and proves CUDA layout.
Value and Storage caches remain distinct; do not widen an unrelated classification to bypass admission.

## Scope and Invariants

Selected candidate domain is internal record values and mutable Generic local storage with integer
scalar guards and existing FP8/scalarBF16/BF2 leaves, composed through canonical nested struct fields.
Whole-value and local physical representations must agree recursively. Natural AnyValue packing and
CUDA local layout remain separate, with offsets/alignment proved at every nested boundary. Field lookup
must use canonical keys and exact result types, never positional semantic assumptions.

Do not admit record arrays, readonly references, resources/device/shared storage, exported signatures,
record-pointer helper results, BF3/BF4 whole-record values, pointers as payload fields or new arithmetic/
conversion semantics. Existing neighboring domains must remain exact. The prototype can narrow this
candidate domain if evidence finds another independent blocker; do not expand the slice indefinitely.
No provider ABI change or performance claim. Provider implementation may change only for the
reproduced nested-store padding defect described below. Keep main frozen/discovery manifests unchanged.

## Interfaces and Dependencies

Likely owners are `source/slang/slang-emit-nvvm-type-lowering.{h,cpp}` and existing canonical field/local
admission in `slang-emit-nvvm.cpp`. Reuse existing layout/lowering builders. Tests belong near
`tests/cuda/nvvm-substandard-records.slang` and native emitter units. Every new helper/special case must
have an input-shape audit, failing proof and responsible-layer reason; moving an unsupported diagnostic
alone is not acceptance. Actual baseline compiler aa1fe42e/version293-g9210ef5a1, providerABI42/fbef1a9e,
37 runtime artifacts,22pins,576main sources, L4SM89/SM80 target, CUDA12.9.2/NVRTC12.9.86.

## Milestones and Prototype Gate

1. Use ignored `build/nvvm-nested-records279` with a small source containing an Inner FP8/BF16 payload
   and Outer integer guards. Preserve all scalar encodings via bit_cast; use no FP arithmetic. Noinline
   initialize/choose/replace functions must force whole values and nested local field accesses; both
   branch outcomes and BF2 component indices must execute. Output a failure mask plus fixed sentinels
   and runtime seed. Independently derive expected outputs and CUDA/Natural layouts before execution.
   Include a one-field nested source matching the current negative unit as the minimal reduction.
2. Freeze hashes/commands/expected three modes; use accepted277 binaries with no rebuild. Run bounded
   native CUDA comparison once per mode and direct `slangc ... -dump-ir` for failed NVVM paths. Require
   positive NVRTC execution/output, actual canonical nested IR/calls/local storage and deterministic
   before failure. Inspect producer shape: it must be intentional canonical struct composition, not
   a semantic-representation accident. If that proof fails, correct the probe or producer first.
3. Select the smallest responsible-layer extension supported by evidence. Keep recursive substandard
   classification separate from device-capable helper classification, preserve roles before cache hits,
   reuse canonical field/layout traversal and document the supported contract. Explicitly inventory
   new helpers/fallbacks and justify their tests. Worker may become sole source writer only after root
   approves this gate; root owns concurrent docs and read-only audits.
4. Before a real build, snapshot accepted277 installed layout/config/pins under this new root. Follow
   already-read local slang-build skill; native RelWithDebInfo, max4workers, preserve numerics modules.
   Refresh cached version metadata consistently and record actual compiler/provider/source-patch bytes.
   Serialize build/test/GPU work. Retain every failed candidate and command; never overwrite attempts.
5. Focused positives must exercise nested values/returns/phi, whole local load/store, nested integer/FP8/
   scalarBF16/BF2 fields and dynamic BF2 component mutation. Include multiple nested depths and both
   Value/Storage visitation orders when practical. Existing arrays/readonly/exported/BF3/BF4/device/
   shared/pointer-result boundaries remain rejected at their exact responsible boundary. Preserve every
   old unit identity; move only intentionally supported nested cases to meaningful positive coverage.
6. Run focused real GPU outputs at NVRTC O3/NVVM O0/O3; qualify complete output and bit patterns against
   the independent integer oracle. Add exact layout/unit checks where they prove the new admission
   boundary. A focused revert drill should recover the minimal preflight failure when practical.
7. Require full accepted277 checkpoint because type/admission impact needs broad qualification:
   runtime4, frozen1356/discovery384/material6, full units and semantic suites with2servers/no retries,
   toolkit18 and four runner contracts. Each gate bounded1800s, max4jobs, no competing workload.
   Preserve1740 exact outcomes/576hashes,37unresolved/20resolved histories and old native identities;
   additions reported separately. No regression becomes a baseline. Material outputs/resources compared
   when practical; no runtime/performance claim.
8. Final helper/input-shape audit and exact source/artifact checks, compact five-part report/one full
   validation record/completed plan, durable record-contract update and concise navigation. Raw logs,
   binaries, source snapshots and exhaustive indexes remain ignored. Format/diff check, accepted local
   commit, skip Slack and continue the authorized loop.

## Validation, Failure and Recovery

Exact next action: format/review completed records and make the accepted local commit, then open a
separate bounded array-store qualification slice.
Unaccepted candidate3 is installed (same compiler as candidate1, corrected provider and tests); accepted277 recovery is verified under `build/nvvm-nested-records279/accepted277-layout`. Known-correct
regressions block subsequent feature work; resolve within this domain or restore accepted277 artifacts
and record the blocker. Independent new shapes are deferred explicitly. User stop instructions take
precedence; no host/toolkit/driver changes. If the candidate cannot preserve canonical role/layout
invariants, close a research-only result without pretending an implementation succeeded.

## Decision Log

- 2026-09-27, lead: choose nested record composition after278's six existing language probes all pass.
  It is a documented270 gap with a concrete existing negative, and can extend useful aggregate
  composition without changing FP8 numerical semantics or broad pointer/resource capabilities.

## Surprises and Discoveries

Fresh delegation unavailable at thread limit. Separate root audits/reused-worker review must not be
reported as fresh independent-agent review. The prototype and subsequent correctness cascade are recorded below.
The initial promoted fixture passes NVRTC and reproduces OutParam<Outer> rejection at both NVVM modes.
Review found its integer-only Guards assignment copied equal values, so the oracle did not observe
that operation specifically. Refine guards to depend on runtime bits and retain the initial attempt;
also normalize native mode directives to the existing270 `-Xslang` convention before final freeze.

## Outcomes and Retrospective

Accepted full279: all1740 main cells,576inputs,37unresolved and20resolved histories preserved.
Units1096pass/13skip preserve1103old identities plus6new; semantics1170pass/78skip preserve1248.
Focused14native/18GPU, runtime4/toolkit18/material6 and four contracts pass. New provider ABI42
bytesa861b242 correct nested-store padding; O3/NVRTC material artifacts exact, O0 artifacts reviewed
with resources equal. Reused independent source and material review found no blockers.

## Candidate1 qualification (2026-09-27)

Production classifier/local-address extension built successfully. Focused GPU: NVRTC O3 and NVVM O0
pass; NVVM O3 returns `[1,123,0,456]`. Native focused9:6pass/3fail; two positives exceed the singleton
fake builder's struct domain, one negative source missed a struct delimiter. Worker repaired test-only
provider selection with a real-builder/fake-libNVVM loader and the delimiter; repair not yet rebuilt.
No broad gates started and no acceptance claimed. Original failed source/output/build identities retained.

Field/stage probes locate bits0, mask400 (FP8a/b and Guards.first), only destination after whole Outer
replacement. Original emitted LLVM O0/O3 is byte-identical (b9331a799550c21d846976434c4265683c3387fa9c109161a4a77b944cc076f0).
Whole aggregate LLVM load/store uses canonical nested type and align8. Optimized PTX replace stores
values from offsets4,8,9,10 contiguously at4/5/6/7, and values from20,24 contiguously at20/22, losing
nested padding. This precisely explains observed fields. Root and reused worker independently read
these instructions; direct vendor reproduction pending. No production workaround authorized.

## Correctness cascade and authorized provider repair

Direct libNVVM12.9 replay of the original identical LLVM fails only O3; a reduced single replacement
also fails. Accepted277 integer-only control (FP8->uint8, BF16->uint16, BF2->uint16x2) executes three
cells: NVRTC/O0 correct, O3 wrong1. Initial absolute-path runner attempt executed zero tests and is
retained as invalid evidence; corrected relative-path attempt owns these outcomes. This defect predates
279 and affects valid nested physical integer types, not FP8 numerical semantics.

Diagnostic LLVM counterfactuals replace only the whole Outer store: explicit leaf stores and the
narrower nested-boundary split retaining flat Inner/Guards stores both make the original exhaustive
oracle pass at O0/O3. The valid producer is unchanged. Provider `_emitStore` owns translation to the
installed target compiler, so use a documented vendor workaround there: after existing validation,
recursively split a struct only when it has an immediate struct child. Use canonical LLVM indices,
CreateExtractValue/CreateStructGEP and DataLayout offsets; derive alignment with commonAlignment from
the actual parent guarantee. Flat structs/scalars/vectors and array subtrees retain existing stores.
Do not invent stronger child alignment, duplicate Slang type/layout semantics or unroll large arrays.
Array-nested layouts remain outside this bounded mitigation, with no new correctness claim.

Worker remains sole source/test writer and may implement this helper plus direct provider shape/
root-alignment tests and an integer GPU regression. Root owns builds and acceptance. Qualify original
FP8 fixture and integer regression at three modes, provider unit controls including unchanged arrays/
flat/scalar/vector stores and align1, and existing invalid-operation contracts. Preserve candidate1
failure and direct counterfactual evidence as the revert proof. Provider ABI stays42; provider bytes
change, requiring full277 checkpoint and material PTX/cubin/resource comparison. This is a correctness
cascade required by the proposed domain, not an independent feature expansion.

Candidate2 provider compiled/linked, but the unit build failed because a brace-less `if` around
SLANG_CHECK left an unmatched else after macro expansion. Source identity/build failure retained;
test-only braces correction authorized. No focused/full qualification claimed for this build.

Candidate3 builds successfully after the test-only brace fix. Source patch691dbe42; compiler9e013b2c
is unchanged from candidate1; provider a861b242 (ABI42) contains the store correction. Focused native
and GPU qualification is running serially. Raw build-3-identity owns all37 runtime and eight source
hashes; no accepted identity changed until the full comparison succeeds.

Candidate3 focused native13/14 passed; only groupshared negative expected a later field-address
diagnostic while existing module preflight rejects `global_var`. Source trace confirms intentional
earlier rejection; correct exact expected construct without changing E52017/zero-provider obligations.
The original GPU source is unchanged and18/18 native cells pass. Six new exhaustive buffers are
independently checked as `[0,123,0,456]`. Candidate4 is a test-only rebuild; preserve GPU identity3
and verify all compiler/provider/cache bytes before inheriting its focused GPU evidence.

## Queued follow-up (not part of279 acceptance)

After279 is accepted, qualify whole stores of arrays containing padded integer records. The current
provider correction intentionally leaves arrays opaque; source review shows existing general copyable
values already admit such types. A bounded independent-oracle research gate can determine whether
that remaining physical boundary shares the reproduced vendor defect before selecting any further
implementation. Do not widen substandard array admission or preselect unrolling as a solution.

A reused independent reviewer (`/root/semantic_writer273`, not this slice's author) completed a
read-only diff/evidence review with no blockers. All eight final source hashes and selected causal/
GPU/native evidence hashes match. This improves on the earlier root/author-only review state while
remaining explicitly not fresh-context review. Full-gate acceptance is still required.

Final acceptance: compact runtime-validation279 generated only after all eight bounded gates and
exact source/runtime/configuration/input/pin audits pass. Frozen1356/discovery384 outcomes are exact.
Original material artifact comparison remains review-required for twoO0 cells, resolved by a separate
review record without asserting material GPU equivalence. Full279/targeted233/cadence0; preserve
verified277 recovery. Next array-nested integer-store research remains outside279 scope.
