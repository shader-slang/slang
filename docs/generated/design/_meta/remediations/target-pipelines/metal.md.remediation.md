---
remediation_report: true
remediator_model: claude-sonnet-5-5
remediated_at: 2026-11-02T21:04:19Z
target_doc: target-pipelines/metal.md
review_report: ../../reviews/target-pipelines/metal.md.review.md
target_doc_source_commit_before: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
actions:
  fixed: 7
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for target-pipelines/metal.md

## Summary

All seven findings were verified against the current source and fixed in the document with minimal edits. No finding was rejected, deferred, or escalated. The doc grew from 95,941 to about 97,400 bytes, under the 98,304-byte cap.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | source/slang/slang-emit.cpp:1800-1811 runs DCE before the getStringHash check, as the review says. | Phase B diagram: performTypeInlining -> dce4 -> nevGate3 -> optional checkGetStringHashInsts -> lowerTuples. |
| F-002 | fixed | slang-emit.cpp:2738, 2930-2942, 2960-2961 gate these four passes. | Phase C diagram: added gate diamonds for simplifyIR, collectCooperativeMetadata, unexportNonEmbeddableIR, checkUnsupportedInst. |
| F-003 | fixed | _common.md requires one consolidated gate table; slang-emit.cpp:1982 branch and 1699-1705 nesting confirmed. | Merged flag, option and context tables; added shouldLegalizeExistentialAndResourceTypes row and checkForMissingReturns under both gates. |
| F-004 | fixed | validateIRModuleIfEnabled calls exist at the cited slang-emit.cpp lines; _common.md requires a summary comment. | Added a checkpoint-line comment under each of the Phase A, B and C diagrams. |
| F-005 | fixed | slang-ir-metal-legalize.cpp:458 defines the wrapper; slang-emit-metal.cpp:697-712 is in tryEmitInstStmtImpl. | Phase C row 21 file link and depth-texture text corrected. |
| F-006 | fixed | slang-emit.cpp:2676-2681 comment explains the Metal/C++/CUDA const-ref rationale. | Replaced blanket absence claim; uncertainty limited to undoParameterCopy. |
| F-007 | fixed | cross-cutting/targets.md does not define metallib atoms; slang-capabilities.capdef:200-208 does. | Both atom claims now link to slang-capabilities.capdef. |
