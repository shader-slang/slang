---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-05T14:58:02+00:00
target_doc: ast-reference/statements.md
target_doc_source_commit: 52339028a2aa703271533454c6b9528a534bac31
target_doc_watched_paths_digest: ba11abbe597dfa9416ac8424c666530b1e926d860fa6eba7e8b1c87f6ff4863c
source_commit: fb192be9f5b3b58555e034599e072158e5c48dfd
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
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

# Review report for ast-reference/statements.md

## Summary
The statements page covers the concrete statement classes, but it still has source-alignment and prompt-completeness issues. The most important factual issue is that `CatchStmt` is described as `try ... catch` syntax even though the parser constructs it from `do ... catch`.

## Items checked
- Ran `python3 docs/generated/design/_meta/regenerate.py show ast-reference/statements.md` and used the listed watched files at `52339028a2aa703271533454c6b9528a534bac31`.
- Compared all concrete `FIDDLE()` entries in `source/slang/slang-ast-stmt.h` with the `## Nodes` table and separately checked abstract intermediates in the hierarchy diagram.
- Read the per-document prompt, `_common.md`, and dependency docs `ast-reference/base.md` and `syntax-reference/grammar.md`.
- Resolved all relative markdown links and anchors in the target document.
- Spot-checked at least 12 source-alignment claims, including `BlockStmt`, `SeqStmt`, `BreakableStmt`, `ChildStmt`, `ForStmt`, `CompileTimeForStmt`, `CatchStmt`, `RequireCapabilityStmt`, labeled breaks, parser statement dispatch, `try` expression handling, and field names in the header.
- Checked required front matter keys and verified that the target digest is a 64-character hex value.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Nodes`, `CatchStmt` row | The `CatchStmt` row describes `try { ... } catch (e) { ... }`, but the parser builds statement-level catch handling from `do` followed by `catch`; `try` is routed as an expression statement. | `source/slang/slang-parser.cpp:6509-6512` sends `try` to `ParseExpressionStatement`, while `source/slang/slang-parser.cpp:7052-7065` reads `do` and then calls `ParseDoCatchStatement` when the next token is `catch`. | Change the `CatchStmt` summary to describe `do S catch (...) H` and remove the `try ... catch` wording from the statements page. |
| F-002 | minor | `## Family hierarchy` and `## Nodes` | The prose says `UniqueStmtIDNode` is excluded from the `## Nodes` table because it is not parsed as a statement, but the table includes a `UniqueStmtIDNode` row. | `source/slang/slang-ast-stmt.h:92-94` declares `class UniqueStmtIDNode : public Decl`, and the target document includes it in the table despite its own exclusion note. | Either remove the `UniqueStmtIDNode` row and keep the prose note, or revise the prose to explicitly say the row is included as a helper even though it is not in the `Stmt` hierarchy. |
| F-003 | major | `## Notable nodes` | The statement prompt requires notable-node callouts for `ReturnStmt`, `ContinueStmt`, `DiscardStmt`, and `EmptyStmt`, but these nodes appear only in the table and hierarchy, not in the `## Notable nodes` callouts. | `docs/generated/design/_meta/prompts/ast-reference-statements.md:37-46` lists those nodes under the required `## Notable nodes` coverage. | Add concise callouts or expand the existing callouts so `ReturnStmt`, `ContinueStmt`, `DiscardStmt`, and `EmptyStmt` are explicitly covered in `## Notable nodes`. |
