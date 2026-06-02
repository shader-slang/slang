---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-05-15T16:50:36+00:00
target_doc: ast-reference/statements.md
target_doc_source_commit: 12bdd912949ee692a11a757b5829fe3ef819bebc
target_doc_watched_paths_digest: 12add2d77eb534b6741b9633b4251590baf28be0de7f2554aacada789942f5ec
source_commit: 2580ad341db243d8bd27edd0327f08a29be906b3
checklist:
  factual_accuracy: pass
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: pass
  front_matter_validity: pass
finding_count: 1
severity_breakdown:
  critical: 0
  major: 1
  minor: 0
  nit: 0
---

# Review report for ast-reference/statements.md

## Summary
The page is structurally lint-clean, but review found 1 finding; the most significant severity is major. The main remediation need is to align the page with watched source evidence and the per-page prompt contract before marking this review cycle complete.

## Items checked
- Checked statement table against all concrete `FIDDLE()` classes in `slang-ast-stmt.h`, abstract statement classes, notable-node coverage, and all links/anchors.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Nodes` | `UniqueStmtIDNode` is a concrete `FIDDLE()` class in the watched header but is omitted from the Nodes table. | `source/slang/slang-ast-stmt.h:92-94` declares `FIDDLE()` `class UniqueStmtIDNode : public Decl`. | Add a `UniqueStmtIDNode` row with `(none)` grammar and a summary noting it is a serialization/control-flow identity helper, not a parsed statement. |
