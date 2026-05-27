---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-05-15T16:50:36+00:00
target_doc: syntax-reference/grammar.md
target_doc_source_commit: 3da83a82d83ad1b0fbd58465ed3a89d2880533dd
target_doc_watched_paths_digest: 40df0e0f2fba874f2bb1d1a886aacac0f20100fdb9c27817c150f2b7ecea2322
source_commit: 2580ad341db243d8bd27edd0327f08a29be906b3
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: fail
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 3
severity_breakdown:
  critical: 0
  major: 2
  minor: 1
  nit: 0
---

# Review report for syntax-reference/grammar.md

## Summary
The page is structurally lint-clean, but review found 3 findings; the most significant severity is major. The main remediation need is to align the page with watched source evidence and the per-page prompt contract before marking this review cycle complete.

## Items checked
- Checked required headings, top-level/declaration/statement productions, generic disambiguation notes, parser functions for lambda/new/try/catch, and relative links.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | required sections | Required top-level sections `## Attributes and decorations` and `## Generics and where-clauses` are missing; content is folded into other sections. | `docs/generated/design/_meta/prompts/syntax-grammar.md` requires those headings. | Add the required headings or split existing content under those headings. |
| F-002 | major | `## Statements` | The grammar claims `TryStmt ::= try Block CatchClause+`, but parser treats `try` as an expression keyword and `catch` after `do`. | `source/slang/slang-parser.cpp:6919-6967` parses `do ... catch`; `source/slang/slang-parser.cpp:7614-7620` parses `try` expressions. | Remove `TryStmt` or rewrite it as the actual `do ... catch` form plus `try` expression. |
| F-003 | minor | `## Notation` | The page uses terminal names such as `IDENT` and `INT_LIT` that are not token kind names in source. | `source/compiler-core/slang-token-defs.h:22-26` uses names such as `Identifier` and `IntegerLiteral`. | Align terminal names with the token catalog or explicitly define aliases. |
