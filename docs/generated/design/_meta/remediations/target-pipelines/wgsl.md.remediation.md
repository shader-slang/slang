---
remediation_report: true
remediator_model: claude-sonnet-5-5
remediated_at: 2026-11-02T21:03:17Z
target_doc: target-pipelines/wgsl.md
review_report: ../../reviews/target-pipelines/wgsl.md.review.md
target_doc_source_commit_before: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
actions:
  fixed: 4
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for target-pipelines/wgsl.md

## Summary

All four findings were verified against the current source and fixed in the document. The two lowering-requirement scans now have table rows, three wrong File links were corrected, and the line-directive and varying-legalization descriptions were qualified. No findings were rejected, deferred, or escalated.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `source/slang/slang-emit.cpp:1085` and `:1666` are direct `calcRequiredLoweringPassSet` calls drawn in the diagrams but absent from the tables, violating the companion-table contract (`_common.md` line 363). | Added Phase A row 3 and Phase B row 20; renumbered later rows and updated in-table row references (31, 35, 73). |
| F-002 | fixed | `checkAutodiffPatterns` is defined at `source/slang/slang-ir-autodiff.cpp:1037`, `removeWeakUseInsts` at `slang-emit.cpp:1000`, `simplifyForEmit` at `slang-ir-simplify-for-emit.cpp:505`. | Replaced the three File links. |
| F-003 | fixed | `slang-emit.cpp:2985` guards the target switch with `lineDirectiveMode == LineDirectiveMode::Default`. | Qualified the Phase D row 1 and Conditional gates descriptions. |
| F-004 | fixed | `slang-ir-legalize-varying-params.cpp:3416` packs only parameters with a VaryingInput offset; `:4186-4187` returns early for a void result type. | Reworded the `legalizeIRForWGSL` step to conditional input packing and non-void result wrapping. |
