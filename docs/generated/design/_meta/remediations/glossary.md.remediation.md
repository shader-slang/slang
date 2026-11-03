---
remediation_report: true
remediator_model: claude-sonnet-5-5
remediated_at: 2026-11-03T12:00:00Z
target_doc: glossary.md
review_report: ../reviews/glossary.md.review.md
target_doc_source_commit_before: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
actions:
  fixed: 8
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for glossary.md

## Summary

All eight findings were verified against the current source and fixed in the glossary with minimal edits. The front-matter digest and date were refreshed; source_commit is unchanged. F-007 was applied to the four named entries (capability atom, decl-ref, fossil format, target legalization driver); other entries were not audited for anchor count.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | docs/language-reference/glossary.md is absent; README.md line 3 says the reference moved to shader-slang/spec. | Conventions link now targets language-reference/README.md and notes the move. |
| F-002 | fixed | source/slang/slang-ir.cpp lines 752-768 give argument access only for UnconditionalBranch and Loop; ifElse and switch have no destination args. | SSA, block parameter and terminator entries now name only unconditionalBranch and loop as argument-carrying. |
| F-003 | fixed | slang-check-expr.cpp around line 4903 returns null for matrix operands in GLSL operator scope, so "never" is wrong. | Reworded as a fast path for supported operators with fallback to normal resolution; removed "never". |
| F-004 | fixed | slang-check-conversion.cpp line 1656 takes one arg; slang-check-overload.cpp line 919 does the `conversionCostSum +=`. | Attributed summing to overload checking; helper described as adjusting one conversion. |
| F-005 | fixed | source/standard-modules/CMakeLists.txt lines 54-64 add four module groups. | "Three exist" changed to "Four module groups exist". |
| F-006 | fixed | Prompt requires alphabetical order; two entries were misplaced. | Moved standard module after specialization and link-time symbol alias before linkage. |
| F-007 | fixed | Prompt allows one code anchor and 2-4 sentences per entry. | Trimmed the four named entries to one anchor each and shortened them. |
| F-008 | fixed | Prompt forbids command-line options in the glossary. | Removed `-experimental-feature` and the E00104 spelling from standard module. |
