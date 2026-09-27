# Qualify stores of arrays containing padded integer records

This bounded ExecPlan follows `.agent/PLANS.md` and the NVVM committed-plan exception. The user
resumed the loop; skip Slack, no push/system changes. Root owns acceptance and records. Fresh worker
`array_probe280` owns ignored prototypes and experimental evidence only; no production writer.

## Purpose and Observable Result

Determine whether the accepted279 compiler preserves every integer field when whole arrays containing
padded records are copied. Slice279 corrects direct nested struct stores but intentionally leaves array
subtrees opaque. A small independent-output experiment will establish this boundary before a fix is
selected. Passing these examples is bounded evidence, not blanket array qualification.

## Progress

- [x] 2026-09-27: Accepted279 committed locally as46e58db06; working tree clean at start.
- [x] 2026-09-27: Read WORKFLOW/STATUS and select bounded array research; fresh worker available.
- [x] 2026-09-27: All37 runtime artifacts,8 source files,2 configs,576 main inputs and22pins match279.
- [x] 2026-09-27: Freeze three shapes and nine cells, with source-derived full-buffer oracle.
      Independent review strengthened wrapper guards before execution; warning-only fixture correction
      explicitly compares checker results with zero and preserves the first attempt.
- [x] 2026-09-27: Clean nine-cell run7pass/2mismatch; all three shapes retain expected LLVM stores.
      Omission control predicts mask8; separate out-copy fails both optimized modes with mask2.
      Isolated277 replay proves preexisting; both PTX paths lose nested field padding.
- [x] 2026-09-27: Separate reused-context review verifies79 evidence references, all outcomes and traces;
      root verifies final identities and strict record generation. Completed report/record/navigation
      ready for formatting and accepted research local commit; no production change.

## Context and Current Pipeline

General copyable values already include integer records and fixed arrays recursively. Slang constructs
canonical IRArrayType/IRStructType; whole assignment flows through emitter store serialization to the
provider `_emitStore`. Its279 `_emitStorePreservingNestedStructLayout` splits only immediate struct
children. Arrays are opaque LLVM aggregates. libNVVM12.9 previously changed valid nested struct stores
into contiguous PTX stores that lost field padding; no array behavior is inferred from that result.

## Scope and Non-Goals

Research only: root arrays of padded integer records, a containing record with an array field, and
nested array/record combinations. Include noinline whole-value return/out/inout copy boundaries with
independent expected field values and sentinels. Worker proposes at most four concrete sources before
execution. No compiler/provider/harness/main manifest changes, substandard-array admission, blanket
unrolling, build or performance claims. A failure is preserved and traced, then supplies a bounded
corrective slice. Frozen195/discovery128 selections stay unchanged.

## Architecture and Invariants

Observe canonical field values and array element strides, never undefined padding bytes. Distinct
source/destination patterns and snapshot checks must detect missing assignment, element permutation and
wrong offsets. Use dynamic input to prevent constant-only evaluation and inspect generated LLVM for
the aggregate stores the experiment claims to exercise. NVRTC agreement supplements the independent
integer oracle. Returned previous values and updated destination must both be observed.

## Interfaces and Dependencies

Accepted279 compiler9e013b2c/version295-g0043e8d17, provider a861b242/ABI42, source0043e8d17 plus
patch62ae6473; commit46e58db06 records those bytes but is not a rebuilt version. Native Ubuntu24.04,
L4SM89/driver580.126.09, targetSM80, CUDA12.9.2/NVRTC12.9.86, LLVM14. All37 runtime artifacts,
22pins and576 main input hashes must match validation279. Reuse existing gate/test helpers; all raw
sources, snapshots, output buffers and LLVM/PTX under `build/nvvm-array-record-stores280`.

## Milestones

1. Root verifies accepted identities. Worker proposes exact matrix/oracle, then freezes bytes before
   execution. Root reviews it; separate reused-context reviewer audits evidence after execution.
2. Reuse279 gate environment and run slang-test with relative test prefixes, retries disabled, serialized
   modes NVRTC O3/NVVM O0/NVVM O3. Runner path/commands are recorded in ignored run records. Require
   exactly one execution per directive and full output-buffer verification, not exit status alone.
3. Emit LLVM/PTX with the same source/options and locate relevant stores. If failure occurs, retain
   original files/results and isolate producer-to-consumer offsets without changing production.
4. Verify identities after research; complete compact five-part report, structured research record and
   this plan. Update STATUS/HISTORY/HANDOFF, format/check diff and locally commit; continue the loop.

## Validation and Acceptance

Every frozen cell retains return code, executed/passed/ignored counts, classification, diagnostic and
canonical shape. Independent output expectations are fixed before execution; failed attempts cannot
be overwritten. Source-derived integer oracle and full-buffer checks are required for every passing
cell. Independent reviewer confirms shape/oracle/identity and limits. No implementation means full279,
targeted233/cadence0 and its1740 exact outcomes/37 unresolved/20 resolved histories remain inherited,
never relabeled fresh. Any production change requires a separate plan and broader gates.

## Failure and Recovery

Use1800-second owned process-group bounds, max4CPU workers, serialize all GPU/build/suites. Known-correct
regressions or artifact drift block feature work until resolved. Unsupported test syntax is retained as
an invalid experiment and corrected in a new attempt, never mislabeled backend failure. No accepted
artifacts are modified. A reproduced preexisting array bug is recorded honestly and prioritized next.

## Decision Log

- 2026-09-27, root: Test the opaque array boundary before broadening279. The65536-element array builder
  unit explicitly preserves bounded IR size; evidence must precede any choice of correction.

## Surprises and Discoveries

Fresh author delegation is available for280. A second fresh spawn and semantic273 followup hit the
thread limit, so existing pch_repro275 independently reviews280 (it did not author280).

## Outcomes and Retrospective

Clean nine-cell research execution yields seven correct buffers and two wrong outputs: nested-array
NVRTC O3 gives46,123,0,456; NVVM O3 gives14,123,0,456; NVVM O0 passes. All flat and wrapped-flat
cells pass. LLVM/CUDA/PTX trace, omission control and accepted277 alternate-layout comparison are complete.
The failures predate279. Separate out-copy proves destination child.first corruption on both optimized
paths. All accepted identities remain unchanged. No production change or new passing baseline.
Next: bounded correction prototype that preserves canonical layout and limits generated IR growth.

## Artifacts and Hand-Off

Raw root `build/nvvm-array-record-stores280`; durable `research-evidence.slice-280.json`,
`report.slice-280-array-record-stores.md`, this plan and navigation updates. Resume by checking current
worker progress and frozen evidence before executing any cell again.
