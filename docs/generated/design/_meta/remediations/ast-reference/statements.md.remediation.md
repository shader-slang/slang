---
remediation_report: true
remediator_model: claude-opus-5
remediated_at: 2026-08-04T09:06:00Z
target_doc: ast-reference/statements.md
review_report: ../../reviews/ast-reference/statements.md.review.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: 53b76e6d3009b8e6434d41573524c7ce5c499d23
actions:
  fixed: 5
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ast-reference/statements.md

## Summary

Every cited parser line was re-derived at `HEAD` and each finding held
up, so all five were fixed. The major one replaced the claim that
`ScopeStmt` inheritance means a statement opens a lexical scope with the
four routines that actually assign `scopeDecl`. The rest were a wording
fix ("rejects" to "diagnoses"), a grammar spelling, removal of IR
lowering detail the contract forbids, and merging the audience sentence
into the opening paragraph. The front matter was not touched.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Confirmed: `stmt->scopeDecl` is assigned only at `source/slang/slang-parser.cpp:6738` (`GpuForeachStmt`), `:6858` (`CompileTimeForStmt`), `:7141` (`BlockStmt`), and `:7417` (`ForStmt`). `ParseSwitchStmt` at `:6572-6582` sets no `scopeDecl` and takes its body from `parseBlockStatement`; `:7457-7480` builds `WhileStmt`/`DoWhileStmt` with no scope operation. | Rewrote the `ScopeStmt` paragraph to list the four assigning routines and explain switch and while/do-while; softened the hierarchy intro and the "scope-introducing" loop claim |
| F-002 | fixed | Confirmed: `source/slang/slang-parser.cpp:7427-7437` diagnoses `UnexpectedTokenExpectedTokenType` and falls through; the function returns `stmt` at `:7454`. Nothing is rejected. | "rejects" changed to "diagnoses", noting the constructed loop node is kept for recovery |
| F-003 | fixed | Confirmed: `source/slang/slang-parser.cpp:7571-7577` reads `defer` then calls `ParseStatement()`; `docs/generated/design/syntax-reference/grammar.md:439` gives `DeferStmt ::= 'defer' Stmt`. No trailing semicolon belongs to `defer`. | Nodes-table summary changed from `defer S;` to `defer S` (see also F-004) |
| F-004 | fixed | `docs/generated/design/_meta/prompts/ast-reference-statements.md:52-53` forbids IR-level statement lowering; lines 39-42 ask only for the AST shape plus a pipeline citation. | `### DeferStmt` now describes `ParseDeferStatement` and the `statement` field and defers timing to `pipeline/04-ast-to-ir.md`; the table cell's "lowered to scope-exit handlers" clause was dropped |
| F-005 | fixed | `docs/generated/design/_meta/prompts/_common.md:65-66` requires the first paragraph to state both coverage and intended reader. | Audience sentence merged into the opening paragraph and the separate `Audience:` paragraph removed |
