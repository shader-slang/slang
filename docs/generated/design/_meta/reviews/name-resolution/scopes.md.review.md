---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-05-15T16:50:36+00:00
target_doc: name-resolution/scopes.md
target_doc_source_commit: 12bdd912949ee692a11a757b5829fe3ef819bebc
target_doc_watched_paths_digest: 8e9260091e2657583e4bc366281ab8bdfb22377d4a78add7b8687c1ba66cd75b
source_commit: 2580ad341db243d8bd27edd0327f08a29be906b3
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: partial
  source_alignment: partial
  front_matter_validity: pass
finding_count: 2
severity_breakdown:
  critical: 0
  major: 2
  minor: 0
  nit: 0
---

# Review report for name-resolution/scopes.md

## Summary
The page is structurally lint-clean, but review found 2 findings; the most significant severity is major. The main remediation need is to align the page with watched source evidence and the per-page prompt contract before marking this review cycle complete.

## Items checked
- Checked required sections, `Scope`, `ContainerDecl`, `ScopeDecl`, `ScopeStmt`, scope-bearing AST nodes, parser scope helpers, edge cases, and watched-path boundaries.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Source` and multiple rules | The page cites and relies on files outside its watched paths, including checker and lookup implementation files. | The manifest watched paths for `scopes.md` are AST headers plus `slang-parser.cpp`; the page cites files such as `slang-check-expr.cpp`, `slang-check-decl.cpp`, `slang-session.cpp`, `slang-check-stmt.cpp`, and `slang-lookup.cpp`. | Either remove unsupported citations or expand `watched_paths`. |
| F-002 | major | `### Sibling scopes` | The required sibling-scope helper is said to be defined in `slang-check-expr.cpp`, which is outside watched paths. | The `name-resolution/scopes.md` manifest entry excludes `source/slang/slang-check-expr.cpp`. | Add the helper owner file to the manifest and cite declaration/definition consistently, or keep the page to watched sources. |
