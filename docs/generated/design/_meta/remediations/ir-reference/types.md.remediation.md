---
remediation_report: true
remediator_model: claude-sonnet-5-5
remediated_at: 2026-11-03T08:36:54Z
target_doc: ir-reference/types.md
review_report: ../../reviews/ir-reference/types.md.review.md
target_doc_source_commit_before: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
actions:
  fixed: 6
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ir-reference/types.md

## Summary

All six findings were verified against the current source and fixed in the document. The 52 `(synthesized)` cells now name a producer (builder call, pass, or intrinsic declaration) or state `no producer at HEAD` (`RTTIPointerType`, `GLSLImageType`, `Rate` kind, after source-wide greps). The document grew from 68.6 KB to about 73 KB, under the 98304-byte cap.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `source/slang/slang-ir.h` lines 1595-1607 confirm `dataLayout` is an `IRType*` and `hasAddressSpace()` requires a non-Generic value. | Ptr callout: data layout is a type operand; `hasAddressSpace()` tests non-Generic, `getOperandCount() > 2` tests presence. |
| F-002 | fixed | Producers confirmed by grep in `source/slang` (builders, passes, `__intrinsic_type` declarations). | Replaced all 52 `(synthesized)` cells with producers; three opcodes marked `no producer at HEAD`. |
| F-003 | fixed | `slang-ir-lower-dynamic-dispatch-insts.cpp` line 1798 uses `getWitnessTableIDType`. | Witness-table prose now distinguishes `getWitnessTableType` and `getWitnessTableIDType`. |
| F-004 | fixed | `slang-ir-insts.h` lines 3101-3105 read two tagged-union operands; `OptionalNoneType` is nullary. | Single-operand claim limited to the three set wrappers; tagged union and generated optional-none distinguished. |
| F-005 | fixed | `slang-check-shader.cpp` `resolveStageOfProfileWithEntryPoint` fills an unknown profile stage from `EntryPointAttribute`. | `-stage node` described as optional. |
| F-006 | fixed | `_common.md` line 275 forbids markup in summaries. | Removed `*named*` and `**first**` emphasis. |
