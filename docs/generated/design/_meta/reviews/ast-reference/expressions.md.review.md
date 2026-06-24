---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-12T13:17:07+00:00
target_doc: ast-reference/expressions.md
target_doc_source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_watched_paths_digest: cac7e2d9eb8ba3e143943a9ace5008f267481600341257629d6b70ee60b4f3a5
source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: pass
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

# Review report for ast-reference/expressions.md

## Summary
The expression reference is structurally complete: its 91 table rows match the 91 concrete FIDDLE-declared `Expr` classes, all abstract FIDDLE bases are excluded from the table, and the required sections are present. One major stale-field issue should be corrected before the page is considered fully source-aligned.

## Items checked
- Ran `regenerate.py show ast-reference/expressions.md` and used the listed prompt, dependency docs, and watched paths.
- Verified the target front matter fields against the current document and HEAD.
- Compared every `## Nodes` table row against the concrete FIDDLE class list in `source/slang/slang-ast-expr.h`: 91 rows, 91 concrete classes, no missing concrete classes, and no abstract FIDDLE classes in the table.
- Checked the family hierarchy against abstract bases such as `DeclRefExpr`, `LiteralExpr`, `ExprWithArgsBase`, `AppExprBase`, `SizeOfLikeExpr`, `PackQueryExpr`, `ShapePackTransformExpr`, `HigherOrderInvokeExpr`, and `DifferentiateExpr`.
- Checked relative links and grammar anchors used by the Source, Nodes, Notable nodes, and See also sections.
- Spot-checked more than 10 factual claims, including literal expressions, operator expression hierarchy, member expressions, casts, pack-query expressions, higher-order autodiff expressions, lambda expressions, type-expression nodes, parser entry points, and front-matter fields. The document has no source line-number citations in the body.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Nodes`, `PartiallyAppliedGenericExpr` row | The Key fields cell lists ``knownGenericArgs: List<Val*>``, but that field does not exist on `PartiallyAppliedGenericExpr`; the current source stores the supplied ordinary arguments in `providedOrdinaryArgs`. | `source/slang/slang-ast-expr.h:958` declares `baseGenericDeclRef`, and `source/slang/slang-ast-expr.h:961-964` declares `providedOrdinaryArgs`; there is no `knownGenericArgs` field. `source/slang/slang-check-overload.cpp:1547-1558` also fills `expr->providedOrdinaryArgs` when creating a partially applied generic expression. | Replace ``knownGenericArgs: List<Val*>`` with ``providedOrdinaryArgs: List<Val*>`` in the row. |

## No-issues notes
- The required notable topics are present where the corresponding classes exist; no `MaterializeExpr` family exists in the watched expression header.
- The helper types `SPIRVAsmOperand`, `SPIRVAsmInst`, and `MatrixCoord` are correctly excluded from the expression node table because they are not `Expr` subclasses.
