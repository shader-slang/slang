---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-05-15T16:50:36+00:00
target_doc: ast-reference/base.md
target_doc_source_commit: 12bdd912949ee692a11a757b5829fe3ef819bebc
target_doc_watched_paths_digest: 888b3ebf12ef38e6bb0fc5df559baf57ab8779e5db0363605462869416316fca
source_commit: 2580ad341db243d8bd27edd0327f08a29be906b3
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 1
severity_breakdown:
  critical: 0
  major: 1
  minor: 0
  nit: 0
---

# Review report for ast-reference/base.md

## Summary
The page is structurally lint-clean, but review found 1 finding; the most significant severity is major. The main remediation need is to align the page with watched source evidence and the per-page prompt contract before marking this review cycle complete.

## Items checked
- Checked required root sections, root parent edges, front matter, support-type table, and all links/anchors.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Root hierarchy` | The hierarchy includes `Scope` as if it were part of the abstract root diagram, but `Scope` is a concrete FIDDLE class. | `source/slang/slang-ast-base.h:111-112` declares `FIDDLE()` `class Scope : public NodeBase`. | Remove `Scope` from the root hierarchy diagram; keep it only in `## Support types`. |
