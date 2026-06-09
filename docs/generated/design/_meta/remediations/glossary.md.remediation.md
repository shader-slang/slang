---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-05T15:45:00Z
target_doc: glossary.md
review_report: ../reviews/glossary.md.review.md
target_doc_source_commit_before: 52339028a2aa703271533454c6b9528a534bac31
target_doc_source_commit_after: 52339028a2aa703271533454c6b9528a534bac31
actions:
  fixed: 3
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for glossary.md

## Summary

The review reported three minor findings, all fixed. F-001: the `hoistable instruction` entry overstated module-scope placement; it now states hoisting goes as far toward module scope as operands allow, matching `addHoistableInst` in `source/slang/slang-ir.cpp` (lines 1645-1707). F-002: the `target intrinsic` and `target legalization driver` entries were out of alphabetical order; they were swapped. F-003: the `[Slang]`-tagged `scope` entry carried an `External:` link reserved for `[General]` entries; the line was removed.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `addHoistableInst` starts at module scope but backs off to the innermost parent defining an operand (`source/slang/slang-ir.cpp:1645-1666,1677-1707`); module scope is not guaranteed. | Rephrased the entry to say hoistable insts are hoisted as far toward module scope as operands allow. |
| F-002 | fixed | Case-insensitive A-Z order puts `target intrinsic` before `target legalization driver` (prompt `glossary.md:7-9`). | Swapped the two entries so `target intrinsic` precedes `target legalization driver`. |
| F-003 | fixed | Prompt `glossary.md:81` reserves `External:` for `[General]` entries; `scope` is `[Slang]`. | Removed the `External:` Wikipedia line from the `scope` entry. |
