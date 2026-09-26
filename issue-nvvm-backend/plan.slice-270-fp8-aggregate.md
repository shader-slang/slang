# Qualify FP8 aggregates through dynamic dispatch

This bounded ExecPlan follows `.agent/PLANS.md`; completed NVVM plans are committed under the
explicit AGENTS exception. On2026-09-26 the user resumed the normal development loop and authorized
prioritizing FP8 aggregates, language breadth and material-driven work. This plan covers only270.
The lead owns acceptance/commits/navigation; one fresh worker owns implementation and execution.
Slack notifications remain skipped following the user's instruction. No push or system change.

## Purpose and Observable Result

Start from `tests/compute/dynamic-dispatch-substandard-float.slang`: NVRTC passes, NVVM O0/O3 reject
helper result `A`, whose fields are FloatE4M3/FloatE5M2. Its second dynamic type contains BF16vector2.
Determine the canonical aggregate/AnyValue path and qualify the smallest principled support slice.
A retained implementation must execute independently checked results, including the original case
when in scope; merely advancing a diagnostic is insufficient. Preserve every existing oracle.

## Progress

- [x] 2026-09-26: Resume authorized; clean HEAD06a26a3f7, accepted269 selected. Read WORKFLOW,
      STATUS, HANDOFF, RESULTS, PLANS and local slang-build skill. Baseline has1,740cells/39gaps.
- [x] 2026-09-26: Verified37 runtime provenance entries (31 installed artifacts, two runner scripts,
      four CUDA toolkit artifacts),576inputs and 22pins. Preserved accepted269-layout including
      numerics modules under the raw270 root.
- [x] 2026-09-26: Original reproduced and eight reduced A/B value/AnyValue direct cells reject
      helper result; canonical producer traced. Natural layout metadata is keyed separately from
      CUDA local layout; independent review confirms existing SSA excludes partial field stores.
- [x] 2026-09-26: Lead selects the explicit combined internal-value/local-record gate below after
      writer/reviewer traces. Implementation must qualify original dynamic dispatch end to end.
- [x] 2026-09-26: Formatted final focused gate and independent pre-full review passed: original3,
      focused9, neighbours33, units4, raw6 exact buffers/assembly6. Final identity frozen; provider
      ABI42 unchanged, all 576inputs/22pins retained.
- [x] 2026-09-26: Full checkpoint completed in1,355.2seconds under its1,800second bound.
      Frozen1,354exact+2resolved; discovery384exact; all 576input hashes unchanged. Units1,099old
      identities preserved+1newpass; semantics1,248exact; runtime4/material6/toolkit18/contracts pass.
- [x] 2026-09-26: Final fixture single-return refinement proves Payload block parameters with
      distinct branch arguments in both direct modes/cache orders. Fixture3/raw6/assembly6 pass;
      pre-phi/full and final-focused identities retained, with production and corpus inputs unchanged.
- [x] 2026-09-26: Lead and independent source/evidence/ledger reviews passed. Compact accepted
      record retains original comparison and failure histories; scoped local commit closes270.
      Continue with the separately bounded four-source breadth probe; Slack stays skipped.

## Surprises and Discoveries

Fresh O0/O3 final IR retains local Ptr<A>/Ptr<B> variables, keyed subword field/component stores and
whole-record load/return in generated unpackAnyValue helpers. A has size2/alignment1 metadata; B
has size4/alignment2 metadata with ScalarLayout field pointers. Therefore value-only admission
cannot resolve the original workload, and local allocation/layout needs explicit qualification. Layout decorations are keyed by ruleName:
Natural/Scalar enum0 is distinct from recomputed CUDA local4/4 for B. Existing SSA intentionally
rejects partial field-address stores, so a promotion pass would be an independent larger change.
No producer repair is indicated. Prior243/249/260 qualified scalar FP8 transport and widening, not aggregate
transport;259 qualified local BF16 references, not arbitrary BF16 record values. Do not infer that
admitting `A` alone completes dynamic object support or that every layout role shares representation.

## Decision Log

- 2026-09-26, lead: Choose existing direct-only semantic capability boundary first;269 provides a
  fresh full correctness baseline, and267 supplied recent material-driven optimization. Review material
  priorities within the next three accepted implementation slices.
- 2026-09-26, lead/reviewer: Exclude newly admitted local-record pointer helper results; the gate
  qualifies internal record values and explicit local/reference parameters. Preserve existing259
  internal BF pointer transport, while exported local-record pointer results remain forbidden.
  The initial candidate exposed this signature route during review before qualification.
- 2026-09-26, lead: AnyValue's BF2 component pointers need an exact local-record field provenance
  path. Admit only that proven BF2 role; retain existing bare/BF3/BF4 pointer boundaries.
- Keep frozen inventory and discovery128 unchanged. New regression fixtures remain focused until a
  separately reviewed inventory decision; no capacity increase or eviction is part of270.
- Use existing classifiers/builders and preserve canonical aggregate/field keys. Audit every new
  helper or special case against its responsible producer and valid input shape.

- 2026-09-26, lead: Accept the newly excluded synthetic Generic local-record pointer-result role
  through independent code audit of helper preflight and TypeInfo before cache lookup. Public source
  pointers lower as UserPointer and do not exercise this synthetic result. No new test-only API or
  IR harness is warranted for a conservative closed role; explicitly retain this evidence limit.
  Actual supported Value/Storage cache visitation orders have six independently checked GPU buffers.

## Outcomes and Retrospective

Build2 executes the unchanged original in all three modes with independent outputs -1.0/0.75.
Mixed-record transport and value-first controls pass six focused cells. Candidate1's original stop
at BF2 sequential addressing is retained; build2 adds that explicitly reviewed local field route.
All focused and full gates passed, including the supplemental actual aggregate phi. The full
comparison preserves 1,738 old outcomes and resolves only the original two direct cells, yielding
1,703correct/37unresolved with 20resolved histories. Independent consolidated review accepted the exact transitions and final identity lineage.
Slice 270 closes with its scoped local commit; the authorized development loop continues.

## Context and Current Pipeline

`createDynamicObject<IFoo>` reads a payload word and dynamic tag; AnyValue lowering generates pack/
unpack helpers returning concrete `A` or `B`. NVVM helper type validation currently stops at `A`.
Inspect `slang-ir-any-value-marshalling.cpp`, `slang-emit-nvvm-type-lowering.{cpp,h}` and
`slang-emit-nvvm.cpp`, and prior FP8/BF16 design contracts. Confirm this trace in fresh final IR.
Use dynamic byte inputs and raw bit oracles to separate exact transport from floating conversion.
Record source/IR shape and code trace before changing the consumer. Do not reconstruct syntax or
introduce a parallel type representation.

## Scope and Invariants

The reviewed270 implementation gate is flat canonical records containing integer scalar fields and
FP8 E4M3/E5M2, scalar BF16 or BF16vector2, with at least one substandard leaf. Admit internal SSA,
construction/extraction/call/return/phi and exact Generic local Ptr/OutParam/BorrowInOut record
storage. Reuse259 keyed local root and CUDA layout proof. Preserve BF3/BF4 local-only support and
all existing helper/copyable classifiers; do not widen267 decomposition. Readonly, device, resource,
shared, nested/array aggregates and exported signatures remain outside the new domain. Whole-value
load/store must use actual proven allocation alignment, preserving separate Natural/CUDA metadata
and role caches. Test guarded BF2 offsets and both cache visitation orders. A's Natural2/1 and B's
Natural4/2 decorations do not replace independently computed CUDA allocation layout.

Bound aggregate internal transport and the necessary canonical AnyValue path. External helper ABI,
arbitrary resource/device layouts, general FP8 arithmetic/narrowing and provider ABI expansion need
separate evidence and scope review. Canonical FloatE4M3/FloatE5M2 format identity must remain distinct.
Local/reference/storage roles require their own layout proof. Existing unsupported forms must retain
clear diagnostics. No source simplification, skipped execution, relaxed oracle or hidden retry.

## Milestones and Execution

1. Save actual source/binary/provider/core/standard-module identities and submodule pins to ignored
   `build/nvvm-fp8-aggregate270`. Preserve the accepted layout before rebuilding. Run the original
   frozen selection with the maintained census runner and all three modes, jobs1, unique output.
2. Reduce aggregate by-value, return, field construction/extraction and AnyValue cases. Derive raw-bit
   expectations independently. Identify the smallest successful end-to-end gate and adjacent rejected
   contracts. Send the lead the input-shape audit and proposed boundary before implementation.
3. Implement only the accepted boundary, with fixtures proving every retained change. Build using
   `build/nvvm-setup/slang-skills/skills/slang-build/SKILL.md`, native RelWithDebInfo, <=4jobs. Refresh
   cached source version as RESULTS describes; include numerics modules from the same compiler/core.
4. Run focused GPU/assembly/negative/neighbour checks at NVRTC O3/NVVM O0/O3. Shared type or lowering
   changes trigger full `nvvm-results.py checkpoint --baseline issue-nvvm-backend/runtime-validation.slice-269.json`
   plus all RESULTS side gates. Run one serialized executor, two unit servers and <=4CPU workers;
   bound long gates to1800seconds. Preserve failures and original review-required comparisons.
5. Compare all 1,740 corpus rows,576 source hashes, exact units/semantic IDs,39unresolved and18resolved
   histories. Any intentionally resolved old failure retains old evidence in resolved history. New
   focused inputs and compiler/provider/module identity must be explicit. Do not claim inherited
   performance/AST measurements as fresh. Material6 is compile/assembly, not GPU execution.
6. Independent source/evidence review, helper inventory and revert drill where practical; lead creates
   one accepted record, compact five-part report and completed plan. Format, scoped local commit,
   record notification skip, then re-rank the next bounded slice under the resumed loop.

## Validation and Acceptance

The original independent values are -1.0 and0.75. Reduced exact transport must include both FP8
formats, differing bytes, signed zero/nonfinite payloads where no numerical interpretation occurs,
field neighbors and call/return boundaries. Numerical assertions use the established format contract,
including NaN classification where payload preservation is not promised by conversion. Preserve
BF16 controls and unsupported external/storage negatives. Declare exact focused neighbours after
tracing the changed domain. Full gate covers runtime4/material6/toolkit18, units/semantics and four
runner contract suites. A research-only closeout keeps compiler/inputs unchanged and qualifies its
own execution evidence without pretending to reset full-checkpoint cadence.

## Failure, Recovery and Artifacts

Retain failed attempts, snapshots, generated artifacts and exhaustive hashes under ignored build.
An introduced regression blocks subsequent implementation until resolved or the candidate is reverted.
If the proof reveals an independent unsupported contract, qualify the current bounded capability or
close research with the next concrete gate; do not extend indefinitely. Human input is required for
an external API/layout decision the current evidence cannot resolve, unavailable required resources,
or destructive/system changes. Routine local implementation/review/commits are authorized.
Keep one structured outcome/comparison/provenance record with all per-cell obligations and histories;
raw artifact indexes stay in build. Current accepted baseline269 is never overwritten.
