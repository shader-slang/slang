---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-12T14:16:12Z
target_doc: target-pipelines/metal.md
review_report: ../../reviews/target-pipelines/metal.md.review.md
target_doc_source_commit_before: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_source_commit_after: eb9403ef595a99c2ff6def1d538dbd7a792d9371
actions:
  fixed: 2
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for target-pipelines/metal.md

## Summary

Both findings were verified against source at the target commit and fixed by
editing the target document. F-001 (critical): the intro's claim that all
three Metal targets "share the same IR pipeline" was corrected to acknowledge
that `MetalLibAssembly` skips `wrapCBufferElementsForMetal`, whose switch arm
lists only `Metal` and `MetalLib` (`source/slang/slang-emit.cpp:1811-1816`).
F-002 (major): the Phase B diagram node `checkStaticAssert` had no companion
table row, violating the one-row-per-diagram-node contract; a row was added
labeling it as a direct helper call rather than a `SLANG_PASS`. No findings
were rejected, deferred, or escalated.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Confirmed at `source/slang/slang-emit.cpp:1811-1816`: the `SLANG_PASS(wrapCBufferElementsForMetal)` switch arm lists only `CodeGenTarget::Metal` and `CodeGenTarget::MetalLib`; `MetalLibAssembly` falls through to `default` and skips the pass. The intro's "all three share the same IR pipeline; they differ only in the downstream tool" was source-contradicted and inconsistent with the page's own Phase B caveat. | Reworded the intro to state the three targets share most Metal legalization but the IR pipeline is not byte-for-byte identical because `MetalLibAssembly` skips `wrapCBufferElementsForMetal` in `linkAndOptimizeIR`, citing the switch arm at line ~1811. |
| F-002 | fixed | Confirmed the Phase B diagram declares `cSA[checkStaticAssert]` (line 274) and routes it between `sAP` and `wCBE` (line 343), while the companion table had no row for it; the target-pipeline contract requires one table row per pass node in the diagram. `checkStaticAssert` is a direct helper call at `source/slang/slang-emit.cpp:1794` (defined at line 580), not a `SLANG_PASS`, matching the page's existing treatment of direct calls (rows 13, 28, 57). | Added Phase B table row 68 for `checkStaticAssert`, gate `(always)`, noting it is a direct helper call (line ~1794) rather than a `SLANG_PASS`; renumbered `wrapCBufferElementsForMetal` to row 69. |
