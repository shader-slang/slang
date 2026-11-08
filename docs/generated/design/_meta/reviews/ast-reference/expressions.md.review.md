---
review_report: true
reviewer_model: gpt-6
reviewed_at: 2026-11-03T06:55:57Z
target_doc: ast-reference/expressions.md
target_doc_source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_watched_paths_digest: 69dce39dbf6b349f7f3cd835ce76adc9b75dc93c8ef91cd0955e180dfa1cd465
source_commit: 56b1b0b83fb96f88f36da7e37cd9c2c7f27af70c
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: partial
  source_alignment: partial
  front_matter_validity: pass
finding_count: 5
severity_breakdown:
  critical: 0
  major: 0
  minor: 4
  nit: 1
---

# Review report for ast-reference/expressions.md

## Summary

The document covers all 94 concrete expression classes and has valid cross-references. It needs localized corrections and contract cleanup. The most significant inaccuracy is the claim that floating-point literal values are not rounded to their suffix type: the lexer helper already performs suffix-dependent conversion before the parser stores the value.

## Items checked

- Reviewed against the **current working tree**, which, as specified by the operator, equals the document’s source commit for all of `source/`. No files were written.
- Read `_review.md`, `_common.md`, and the document-specific prompt; ran `regenerate.py show ast-reference/expressions.md`.
- Compared all 94 table class names with concrete FIDDLE declarations in `slang-ast-expr.h`; found no missing classes or abstract-class rows.
- Spot-checked more than 10 claims: expression inheritance, application fields, member-access fields, optional-conversion fields and construction, literal conversion, lambda body construction, tuple version gating, disabled tuple-type parsing, HLSL integer-specifier parsing, static subscript names, parser entry points, and size/alignment/count operand parsing.
- Resolved all 87 Markdown links, covering 20 distinct destinations; checked referenced anchors and generated-page manifest entries. Dependency documents were consulted only for cross-reference checks.
- Found **zero line-number citations** in the target document’s body.
- Checked required sections, table columns, callouts, front-matter keys and formats, diagram conventions, and the size cap: 33,558 bytes against 98,304 allowed.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | minor | `## Nodes`, line 95; `### LiteralExpr family`, lines 296–309 | The table says `value` is “not rounded to `suffixType`,” and the callout describes it as the “value as written.” The parser stores its helper’s result unchanged, but that result has already undergone suffix-dependent rounding or truncation. | `source/compiler-core/slang-lexer.cpp:1309` truncates hexadecimal values for Half/Float; line 1329 parses decimal Float into a `float`; line 1347 rounds Half. `source/slang/slang-parser.cpp:9180` stores the returned value. | Describe `value` as the suffix-converted value returned by the lexer helper. Retain the narrower statement that the parser performs no additional rounding, and remove the table’s contrary claim. |
| F-002 | minor | `### LambdaExpr and LambdaDecl`, lines 375–380 | The claim that block-bodied and expression-bodied lambdas “build the same shape” is incorrect. Their outer `LambdaExpr` matches, but their `bodyStmt` nodes differ. | `source/slang/slang-parser.cpp:8838` assigns `parseBlockStatement()` for a braced body; line 8844 creates a `ReturnStmt` directly for an expression body. | State explicitly that the braced form stores a block, whereas the expression form stores a direct synthesized `ReturnStmt`; remove “build the same shape.” |
| F-003 | minor | `## Nodes`, lines 107, 109, 116, 118 | Several named fields omit their required types: `argumentDelimeterLocs`, `tryClauseType`, and both `memberOperatorLoc` entries. | `docs/generated/design/_meta/prompts/_common.md`, AST table contract, requires `name: Type`. Declarations are in `source/slang/slang-ast-expr.h:231`, `:263`, `:339`, and `:355`. | Use `argumentDelimeterLocs: List<SourceLoc>`, `tryClauseType: TryClauseType`, and `memberOperatorLoc: SourceLoc` in the affected cells. |
| F-004 | nit | `### LiteralExpr family`; `### Type-expression family` | These callouts exceed the required 2–5 sentences each; both span multiple explanatory paragraphs. | `docs/generated/design/_meta/prompts/_common.md`, AST reference section 5, specifies “2–5 sentences each.” The target’s callouts occupy lines 285–314 and 391–424. | Condense each callout to at most five sentences, preserving source-backed distinctions and using existing links for further detail. |
| F-005 | minor | `### BuiltinOperatorExpr`, lines 234–238 | The explanation of how IR lowering chooses floating-point versus integer remainder exceeds this page’s explicitly permitted scope, although the header supports the claim. | `docs/generated/design/_meta/prompts/ast-reference-expressions.md`, “Forbidden content,” excludes “IR lowering of expressions.” `source/slang/slang-ast-expr.h:296` documents the field and its lowering use. | Keep the field’s check-time meaning and its relevance to `Mod`; replace the lowering implementation explanation with a link to `../pipeline/04-ast-to-ir.md`. |