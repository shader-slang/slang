---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-12T13:17:20+00:00
target_doc: ast-reference/statements.md
target_doc_source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_watched_paths_digest: 1e20209a27420ffb3b4a21146a697ad5de4a4148e92e47a97368c920b78d2800
source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
checklist:
  factual_accuracy: pass
  cross_references: pass
  completeness: pass
  style_consistency: pass
  source_alignment: pass
  front_matter_validity: pass
finding_count: 0
severity_breakdown:
  critical: 0
  major: 0
  minor: 0
  nit: 0
---

# Review report for ast-reference/statements.md

## Summary
No findings were identified for the statements page. The page covers all concrete statement-family FIDDLE classes, keeps abstract intermediates out of the Nodes table, and its parser/source claims matched the watched files checked in this pass.

## Items checked
- Ran `python3 docs/generated/design/_meta/regenerate.py show ast-reference/statements.md` and used the listed prompt, dependency docs, and watched files at `eb9403ef595a99c2ff6def1d538dbd7a792d9371`.
- Compared all 30 concrete `FIDDLE()` entries in `source/slang/slang-ast-stmt.h` with the `## Nodes` table and verified that no `FIDDLE(abstract)` class appears as a row.
- Checked immediate parent names in the Nodes table against the header declarations, including `ScopeStmt`, `BreakableStmt`, `LoopStmt`, `ChildStmt`, `CaseStmtBase`, and `JumpStmt`.
- Spot-checked source-alignment claims for `BlockStmt`, `SeqStmt`, `IfStmt`, `SwitchStmt`, `CaseStmt`, `DefaultStmt`, `ForStmt`, `CompileTimeForStmt`, `TargetSwitchStmt`, `StageSwitchStmt`, `DeferStmt`, `CatchStmt`, `LabelStmt`, `BreakStmt`, `ContinueStmt`, `DiscardStmt`, `ExpressionStmt`, `DeclStmt`, `EmptyStmt`, and `RequireCapabilityStmt`.
- Verified that the statement parser dispatches `do ... catch`, `try` expression statements, `$for` compile-time statements, `__target_switch`, `__stage_switch`, and `__GPU_FOREACH` consistently with the document.
- Resolved the relative links and anchors; the body has no source line-number citations.

## Findings

(no findings)

## No-issues notes
- The `UniqueStmtIDNode` row is explicitly framed as a serialized helper outside the `Stmt` hierarchy, matching its `Decl` parent in the header.
- The `CatchStmt` row and notable callout describe `do ... catch`, while the parser routes `try` through expression-statement parsing.
- The required notable-node coverage for block/sequence statements, branch statements, loops, returns, defer, labeled control flow, expression/declaration/empty wrappers, and capability statements is present.
