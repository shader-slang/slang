---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-05T14:18:11+00:00
target_doc: ast-reference/expressions.md
target_doc_source_commit: 52339028a2aa703271533454c6b9528a534bac31
target_doc_watched_paths_digest: 0d26ac1f4551507e46c4f987f6ed9056ef663fb2db265a373049f3b0d384bd13
source_commit: fb192be9f5b3b58555e034599e072158e5c48dfd
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: pass
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 2
severity_breakdown:
  critical: 0
  major: 0
  minor: 2
  nit: 0
---

# Review report for ast-reference/expressions.md

## Summary
The expression reference is structurally complete: its 91 table rows match all 91 concrete FIDDLE-declared classes in `slang-ast-expr.h`, all links resolve, and the required sections are present. Two minor factual issues should be corrected before the page is considered fully source-aligned.

## Items checked
- Ran `regenerate.py show ast-reference/expressions.md` and used the listed prompt, dependency docs, and watched paths.
- Verified the target front matter fields and checked that the watched C++ files did not change between the target source commit and review HEAD.
- Compared every `## Nodes` table row against the concrete FIDDLE class list in `source/slang/slang-ast-expr.h`: 91 rows, 91 concrete classes, no missing classes, no abstract classes.
- Resolved all 82 Markdown links and anchors in the target document.
- Spot-checked more than 10 factual claims, including literal expressions, operator expression hierarchy, member expressions, casts, pack-query expressions, higher-order autodiff expressions, lambda expressions, type-expression nodes, parser entry points, and front-matter fields. The document has no source line-number citations in the body.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | minor | `## Nodes`, `OperatorExpr` row | The row describes `OperatorExpr` as an `abstract intermediate`, but the source declares it with plain `FIDDLE()`, not `FIDDLE(abstract)`. | `source/slang/slang-ast-expr.h` line 275 declares `FIDDLE()` for `class OperatorExpr : public InvokeExpr`. | Reword the summary to avoid calling `OperatorExpr` abstract, for example say it is the shared operator-expression base for infix, prefix, postfix, select, and short-circuit forms. |
| F-002 | minor | `## Notable nodes`, `LiteralExpr family` | The text says adjacent string literals are `merged at lex time`, but the parser concatenates adjacent `StringLiteral` tokens while building `StringLiteralExpr::value`. | `source/slang/slang-parser.cpp` lines 8681-8695 check for adjacent `TokenType::StringLiteral` tokens and append them in a `StringBuilder` before assigning `constExpr->value`. | Change the wording to say adjacent string literals are merged by expression parsing, after lexing has produced adjacent string-literal tokens. |

## No-issues notes
- The required notable topics are present where the corresponding classes exist; no `MaterializeExpr` family exists in the watched expression header.
- The helper types `SPIRVAsmOperand`, `SPIRVAsmInst`, and `MatrixCoord` are correctly excluded from the expression node table because they are not `Expr` subclasses.
