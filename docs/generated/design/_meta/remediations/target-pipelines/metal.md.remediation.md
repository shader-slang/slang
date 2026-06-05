---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-05T17:00:14Z
target_doc: target-pipelines/metal.md
review_report: ../../reviews/target-pipelines/metal.md.review.md
target_doc_source_commit_before: 43e8ca0cef30f575bd1750589b2c7cd9f2b6e030
target_doc_source_commit_after: 43e8ca0cef30f575bd1750589b2c7cd9f2b6e030
actions:
  fixed: 3
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for target-pipelines/metal.md

## Summary

Acted on the follow-up review of the rebuilt Phase B diagram. All
three major findings were fixed by editing the target document: the
two minimal-optimization branch passes that were missing from Phase B
(F-001), the mis-drawn `shouldLegalizeExistentialAndResourceTypes`
false arm (F-002), and the Phase C table row ordering for the Metal
legalizer block (F-003). No findings were rejected, deferred, or
escalated.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Source runs `eliminateDeadCode` when `minimalOptimization && reqSet.generics` (`source/slang/slang-emit.cpp:1414-1417`) and runs `applySparseConditionalConstantPropagation` + `eliminateDeadCode` when `minimalOptimization` (`source/slang/slang-emit.cpp:1516-1529`); both were absent. | Added `genGate{reqSet.generics}` + `eliminateDeadCode` on the `minOptGate1` false arm, and `applySparseConditionalConstantPropagation` + `eliminateDeadCode` on the `minOptGate2` (`minimalOptimization`) true arm; added Phase B table rows 30, 47, 48 and renumbered the table to 68 rows. |
| F-002 | fixed | The `shouldLegalizeExistentialAndResourceTypes` false branch runs only the else-path `legalizeEmptyTypes` then rejoins (`source/slang/slang-emit.cpp:1685-1690`), not the existential/resource sequence. | Removed the existGate false-arm edge into `etlGate`; added an `lET_else` node so the false arm runs `legalizeEmptyTypes (else path)` and rejoins before `legalizeMatrixTypes`; added Phase B table row 60 gated on `!shouldLegalizeExistentialAndResourceTypes`. |
| F-003 | fixed | Source order is `legalizeIRForMetal` (1997) -> `floatNonUniformResourceIndex` (2035) -> `legalizeLogicalAndOr` (2040) -> `legalizeImageSubscript` (2056); the Phase C table listed `legalizeImageSubscript` first, contradicting both source and the diagram. | Reordered Phase C table rows 6-9 to match source and the diagram, and refreshed the per-row line citations. |
