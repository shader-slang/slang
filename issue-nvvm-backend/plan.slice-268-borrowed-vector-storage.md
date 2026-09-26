# Preserve native vector storage for borrowed helper aggregates

This bounded ExecPlan follows `.agent/PLANS.md` and the NVVM exception requiring completed plans and
reports in the closing local commit. The user's 2026-09-26 request authorizes fixing the borrowed
float3 issue first, followed by enumeration repair and corpus expansion (a separate slice269).
Complete this correctness slice, commit and notify, then continue to that authorized second slice.
No general-loop resumption, autodiff pilot, push, publication or unrelated backend extension.

## Purpose and Observable Result

A valid helper reading a float3 field through an explicit constref aggregate currently fails direct
NVVM with E52018, compact parameter-group vector extraction. Float4 works. Fix the storage-role
classification so the unchanged valid helper compiles, assembles and produces independently expected
GPU output at NVVM O0/O3, with NVRTC O3 as a reference. Preserve genuinely compact parameter-group
storage, mutable/local native storage and existing ABI/provider contracts.

## Progress

- [x] Read WORKFLOW, STATUS, HANDOFF, PLANS and the native slang-build skill; clean start0aff56e26.
- [x] Verify exact accepted267 compiler/provider/modules/toolkit bytes and preserve its layout.
- [x] Reproduce float3 failure at O0/O3 and successful float4 controls; GPU fixture fails both NVVM
      modes on267 while NVRTC passes. Boundary fixtures are qualified.
- [x] Independent audit agrees canonical borrowed input is valid; approve explicit storage-role
      propagation through existing field/index resolver records, independent of readonly access.
- [x] Candidate2 built serially. New fixtures6/6 GPU and6/6 standalone assembly; physical
      reference controls2/2 assembly; neighbours42pass/9platformskip, including Graph6 and negative2.
      Baseline with final fixture bytes fails precisely both borrowed NVVM modes; all controls pass.
- [x] Full checkpoint passes:1356 frozen+357 discovery exact,567 unchanged hashes, runtime4/material6.
- [x] Units1086pass/13skip and semantics1170pass/78skip preserve exact maps; toolkit18 and
      all four runner contract suites pass.
- [x] Independent final acceptance and source/hash review; compact report/record formatted.
      Closing local commit and notification status are retained in the ignored closeout.
- [x] Prepared the bounded269 handoff; continue only that authorized follow-up after268 closeout.

## Surprises and Discoveries

Research267 retained the failure under `build/nvvm-receiver-snapshot267/constref-float3` and an explicit
copied Graph constref experiment. The float4 counterpart succeeds. The suspected boundary is
`_getNVVMStructFieldAddress` → `_getNVVMCompactParameterGroupVectorPointer`: a read-only borrowed
helper field is treated as compact storage although the helper uses a native vector layout. This is
confirmed by unchanged267 reproduction and independent producer/consumer audit. Native helper
references lower with `NVVMTypeUse::Value`; physical and parameter-group roots lower with
`ParameterGroupStorage`. No producer or provider ABI repair is appropriate.

The compact-resource control initially had an arithmetic oracle typo (1330 instead of1320);
the writer retained the failed source/log and corrected the independently calculated sum before
production editing. It is not a compiler failure or a retry-hidden regression.

## Decision Log

- 2026-09-26, lead: Execute two bounded slices sequentially. Compiler/storage correction is268;
  parser and corpus changes belong to269. Do not mix their validation identities.
- 2026-09-26, lead: One fresh-context implementation worker owns compiler/tests and all build/GPU
  execution. Lead owns plan, acceptance, reports and commits; independent read-only review can overlap.
  Maximum four CPU workers, two unit servers, serial builds/GPU suites, gates bounded to30minutes.
- 2026-09-26, lead: Shared storage/emission impact requires a full checkpoint against accepted267.
  No new performance claim or paired timing run is required for this correctness repair.
- 2026-09-26, lead/reviewer: Retain immutable conversion gates alongside new storage role. Removing
  them would expand mutable physical/direct-array conversion without matching store coverage.
- 2026-09-26, lead: Focused outputs, native/compact IR distinction and baseline revert proof pass;
  authorize full checkpoint and exact side gates using final candidate2 bytes.

## Outcomes and Retrospective

Final candidate2 is accepted after independent source, focused, full-corpus and ledger review.
All1713 outcomes,567 input hashes,39 unresolved failures and18 resolved histories are preserved;
exact unit/semantic maps and all side gates pass. The borrowed helper failure is resolved without
changing valid compact storage or expanding mutable conversion. No performance claim.
The closing commit/notification belong to the ignored closeout. Continue the separately authorized
enumeration/corpus slice269, then stop; its implementation is not part of this completed plan.

## Context and Current Pipeline

Accepted267 source is commit0aff56e26405e08b47971a6c6a46bfc81fca8fd5. Its installed compiler version
is2026.18.3-283-gd937c9f5a plus the recorded source patch; loaded compiler SHA256 is
1cba6a5119a4058b449f70ed79a242fc13ddccff382e77132e58bdcbad8d97ed. ProviderABI42 is unchanged,
SHA256fbef1a9e22f3ac0cd42d3ffbade22470f7143930608924fc39b5bc57e40eb913.
The launcher hash alone is not compiler identity. WORKFLOW/RESULTS define full provenance and commands.
Read the recorded source probe, follow borrowed parameter lowering and field/lane address production,
then inspect layout selection and vector extraction. Reuse existing storage classifications/helpers.

## Scope and Non-Goals

Fix valid borrowed aggregate vector-field access at its owning representation/classification boundary.
Cover nested fields, lane reads, native vector padding and actual compact resources as relevant to
that invariant. No new provider ABI, blanket inbounds/noalias flags, source-oracle changes, arbitrary
provenance walks, borrowed float3 special-case emitter or unrelated feature implementation.

## Architecture and Invariants

Readonly access does not imply compact storage. Canonical aggregate identity, storage role and
selected physical layout must remain consistent from producer through field address and vector-lane
consumer. Audit every new helper/fallback with AGENTS' input-shape checklist. A malformed producer
must be repaired there; intentionally valid shapes must be handled by the layer owning their layout.
Use assertions for impossible shapes and existing canonical type/field-key operations.

## Interfaces and Dependencies

Native Ubuntu24.04, L4 SM89/driver580.126.09, targetSM80, CUDA12.9.2/NVRTC12.9.86, LLVM14,
RelWithDebInfo. Preserve matching bin/lib/module/cache layout before rebuilding. Use
`build/nvvm-setup/slang-skills/skills/slang-build/SKILL.md`, native CMake and existing configuration,
refreshing SLANG_VERSION_FULL/NUMERIC. Raw artifacts live under `build/nvvm-borrowed-vector268`.
Dependency pins and environment remain unchanged.

## Milestones

1. Capture baseline identities and minimal failure/control with unchanged source and exact commands.
2. Add an output-checked fixture and boundary cases; trace producer/consumer storage contracts and
   obtain independent review before retaining a fix.
3. Build once serially per necessary source identity. Preserve all failed attempts and hashes.
   Require focused runtime success in all three modes, assembly and relevant contract regressions.
4. Run the full maintained checkpoint against validation267, then units/semantics/toolkit/contracts
   as in RESULTS. Compare every id/mode/five-field outcome and exact side-gate identities.
5. Commit compact plan/five-part report/one structured outcome record, update navigation, notify,
   and proceed to separately planned269 without further user confirmation.

## Validation and Acceptance

Revert proof must show the relevant float3 test fails on accepted267 while the candidate passes;
float4/native and real compact-storage controls must keep their intended outputs/layouts. Runtime
oracles are independent of NVRTC; compile success alone cannot establish correctness. A broader Graph
probe is useful if it exercises the same invariant, but must not extend the repair into another bug.

Full acceptance preserves1356 frozen +357 discovery outcomes,567 input hashes,39 known unresolved
cells and18 resolved histories. New focused tests are separate additions. Require runtime4,
material6, toolkit18, exact unit/semantic maps and four harness contracts. A new regression blocks
promotion; retain original comparisons. Known concurrency/PCH incidents are not erased by a pass.
Check material compile/assembly support, without inferring GPU performance from resources.

## Failure and Recovery

Bound long gates to30minutes and preserve failures/timeouts under unique paths. Never retry away
wrong output. Resolve introduced regressions within the same invariant; record independent blockers
rather than expanding scope. If the correction cannot be qualified, preserve its patch and restore
accepted source/layout together before dependent work. Reassess authorization only if a genuinely
independent requirement prevents the requested bounded sequence.

## Artifacts and Hand-Off

Keep raw logs, dumps, source snapshots, patches and binary inventories under ignored build. Retain
one structured validation record with per-cell outcomes/history, compact report268, completed plan
and necessary design/navigation updates. Root handles local commit and once-per-slice Slack delivery,
recording its permalink in the ignored closeout. Slice269 will repair enumeration and qualify the
five missing eligible files plus four breadth candidates within the128-source discovery limit.
