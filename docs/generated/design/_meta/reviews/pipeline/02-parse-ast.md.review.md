---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-30T13:33:18+00:00
target_doc: pipeline/02-parse-ast.md
target_doc_source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_watched_paths_digest: 08a85b2139e6f95014abd9994f69cdb88937d3ed412d5d370aa4c584aca92640
source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
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
  major: 0
  minor: 1
  nit: 0
---

# Review report for pipeline/02-parse-ast.md

## Summary

The page largely matches the parse/AST prompt and the watched parser and AST sources. The only issue I found is a small but concrete source-location error: the page says the `Val` family is rooted in `slang-ast-val.h`, but the base `Val` class is declared in `slang-ast-base.h`. Links, front matter, required sections, and the main two-stage parsing and syntax-declaration descriptions otherwise checked out.

## Items checked

- Verified the target front matter, including `source_commit` and `watched_paths_digest`, against the document's own recorded values.
- Ran a targeted relative-link resolution check for all 39 Markdown links in `pipeline/02-parse-ast.md`; all resolved in the workspace.
- Spot-checked 20 concrete claims against source: `parseSourceFile`, `parseUnparsedStmt`, `ParsingStage::Decl` / `Body`, `parseOptBody`, `UnparsedStmt`, `tryParseGenericApp`, `SyntaxParseInfo`, `g_parseSyntaxEntries`, `populateBaseLanguageModule`, parser recovery state, `maybeDiagnoseKeywordUsedAsName`, `BuiltinOperatorExpr`, `convertToBuiltinArithmeticOp`, `ASTBuilder::create`, `ASTBuilder::getOrCreate`, `parseOptionalGenericConstraints`, `maybeParseGenericConstraints`, `parseAssocType`, `parseInterfaceConstraintDecl`, and `isDeclAllowed`.
- Checked that the required prompt sections are present: inputs/outputs, parser, two-stage parsing, syntax-as-declaration, AST data model, generics ambiguity, modifier parsing, and failure modes.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | minor | `## AST data model`, lines 173-178 | The page says `Val` is "rooted in [slang-ast-val.h]", but the `Val` base class is declared in `slang-ast-base.h`; `slang-ast-val.h` contains concrete value-related classes and helpers, not the root. | `source/slang/slang-ast-base.h:374` declares `// Base class for compile-time values`, followed by `class Val : public NodeBase` at `source/slang/slang-ast-base.h:379-380`. | Change the `Val` bullet to cite `slang-ast-base.h` for the root class, optionally mentioning `slang-ast-val.h` only for concrete value subclasses. |

## No-issues notes

- The two-stage parsing description matches `parseSourceFile` setting `ParsingStage::Decl` and `parseUnparsedStmt` setting `ParsingStage::Body`.
- The syntax-as-declaration section matches `tryLookUpSyntaxDecl`, `tryParseUsingSyntaxDecl`, `g_parseSyntaxEntries`, and `populateBaseLanguageModule`.
- The generic-constraint details match the parser helpers for `nonempty`, `countof`, reversed `countof` diagnostics, `__hasDiffTypeInfo`, associated-type relocation, and `__constraint`.
