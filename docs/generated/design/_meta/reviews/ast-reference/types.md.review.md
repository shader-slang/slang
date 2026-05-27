---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-05-15T16:50:36+00:00
target_doc: ast-reference/types.md
target_doc_source_commit: 12bdd912949ee692a11a757b5829fe3ef819bebc
target_doc_watched_paths_digest: 05b1016228f8c0bbf2fd6e6ea6d165c4d173a37c116aea0c0eccdb47c10abf97
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

# Review report for ast-reference/types.md

## Summary
The page is structurally lint-clean, but review found 1 finding; the most significant severity is major. The main remediation need is to align the page with watched source evidence and the per-page prompt contract before marking this review cycle complete.

## Items checked
- Checked concrete and abstract type-class classification, `Val -> Type` relationship, grammar links, and all links/anchors.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Nodes` | `Fp8Type` appears in the Nodes table even though it is declared `FIDDLE(abstract)`. | `source/slang/slang-ast-type.h:113-114` declares `FIDDLE(abstract)` `class Fp8Type : public DeclRefType`. | Remove `Fp8Type` from the Nodes table and keep it only in the hierarchy or abstract-intermediates discussion. |
