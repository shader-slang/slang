---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-05-15T16:50:36+00:00
target_doc: ast-reference/index.md
target_doc_source_commit: 12bdd912949ee692a11a757b5829fe3ef819bebc
target_doc_watched_paths_digest: 93ae42ace19ae6731e173284d4ae34f4cb35be0cf408f0e32f578fb2a2cc5501
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
  major: 0
  minor: 1
  nit: 0
---

# Review report for ast-reference/index.md

## Summary
The page is structurally lint-clean, but review found 1 finding; the most significant severity is minor. The main remediation need is to align the page with watched source evidence and the per-page prompt contract before marking this review cycle complete.

## Items checked
- Checked front matter, links/anchors, page table counts, and taxonomy against `slang-ast-base.h`.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | minor | `## Family taxonomy` | The diagram includes `NonTypeVal`, which is not an AST class/root declared in `slang-ast-base.h`. | `source/slang/slang-ast-base.h` declares roots such as `Val`, `Type`, and `DeclRefBase`, but no `NonTypeVal`. | Replace the pseudo-node with actual declared roots or explain it outside the taxonomy diagram. |
