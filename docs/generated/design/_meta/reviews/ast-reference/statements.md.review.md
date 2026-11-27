---
review_report: true
reviewer_model: gpt-6
reviewed_at: 2026-11-03T06:58:24Z
target_doc: ast-reference/statements.md
target_doc_source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_watched_paths_digest: f39f7b2520b2e5061ebb8ffd7d9d6c3ee67d8463ef419fefebeebc4ad95a45
source_commit: 56b1b0b83fb96f88f36da7e37cd9c2c7f27af70c
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: pass
  style_consistency: partial
  source_alignment: partial
  front_matter_validity: pass
finding_count: 2
severity_breakdown:
  critical: 0
  major: 0
  minor: 1
  nit: 1
---

# Review report for ast-reference/statements.md

## Summary

The document covers all concrete statement-header classes, and its line citations and links resolve correctly. The principal factual issue is a prose statement identifying the `IfStmt` predicate as `Stmt*`, although the header and the document’s own table correctly identify it as `Expr*`. Several notable-node callouts also exceed the contract’s prescribed length.

## Items checked

- Reviewed against the **current working tree**; `git diff` against the document’s source commit reported no differences under `source/`. Confirmed HEAD matches the supplied review commit.
- Read `_review.md`, `_common.md`, and the document-specific prompt identified by `regenerate.py show ast-reference/statements.md`.
- Checked all 30 concrete classes against `slang-ast-stmt.h`, including table coverage, field types, and hierarchy relationships.
- Verified all 33 line-citation expressions, including paired locations and ranges, against `slang-parser.cpp`, `slang-options.cpp`, and `slang-check-stmt.cpp`.
- Spot-checked more than 10 behavioral claims: block sequencing, empty blocks, deferred body parsing, scope restoration, `if let` desugaring, case-label siblings, HLSL loop scoping, loop-initializer validation, compile-time range parsing, deprecation diagnostics, shared target-case bodies, catch nesting, labeled breaks, defer restrictions, and capability-token filtering.
- Resolved all 45 Markdown link occurrences (15 distinct destinations), checked both explicit anchors, and confirmed referenced generated pages appear in the manifest. Read dependency excerpts only for cross-reference checks.
- Checked required section order, table columns, required notable-node coverage, front-matter keys and formats, Mermaid syntax, and Liquid safety. The document is 25,891 bytes, below its 49,152-byte cap.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | minor | `### IfStmt`, line 197 | `Holds the predicate and both branches as raw Stmt*` incorrectly assigns the statement-pointer type to the predicate. The table already gives the correct type. | `source/slang/slang-ast-stmt.h:86` declares `Expr* predicate`; lines 87–88 declare the two branches as `Stmt*`. | Change the sentence to specify an `Expr*` predicate and two `Stmt*` branches. |
| F-002 | nit | `## Notable nodes`, particularly lines 138–173, 228–258, and 291–319 | Multiple callouts are substantially longer than the required 2–5 sentences each. For example, the block/sequence callout spans two lengthy paragraphs containing approximately 13 sentences. | `docs/generated/design/_meta/prompts/_common.md`, “AST reference family contract,” item 5 requires “short prose callouts (2-5 sentences each).” | Condense each callout to 2–5 sentences, retaining the required AST distinctions and parser references. Remove repetition of table fields and extended diagnostic examples. |