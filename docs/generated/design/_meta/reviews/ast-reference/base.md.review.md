---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-30T13:24:00+00:00
target_doc: ast-reference/base.md
target_doc_source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_watched_paths_digest: e0ca12ff3e02ab39e4f5cee7d1fb7b6641e6e7a1df3a23e44d1cceced99127a3
source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
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

# Review report for ast-reference/base.md

## Summary

The page is broadly useful and most source claims about `NodeBase`, `Val`, `Type`, `Decl`, `Expr`, `Stmt`, modifiers, support wrappers, and link targets checked out against the watched headers. The main issue is contract drift: the `## Roots` section does not follow the exact required root list/order, and `## Support types` omits the required diagnostic helper coverage. I also found one small source-description inaccuracy in the `## Source` section.

## Items checked

- Verified the target front matter contains all required generated-doc keys and uses the requested target source commit and digest.
- Checked all relative links in the page against files present under `docs/generated/design/` or watched source paths.
- Spot-checked 18 source claims against `source/slang/slang-ast-base.h`, including the inheritance edges for `NodeBase`, `SyntaxNodeBase`, `SyntaxNode`, `Modifier`, `ModifiableSyntaxNode`, `DeclBase`, `Decl`, `Expr`, `Stmt`, `Val`, `Type`, `DeclRefBase`, and `Scope`.
- Spot-checked 12 support-type claims against `source/slang/slang-ast-support-types.h`, including `NameLoc`, `Modifiers`, `QualType`, `SubstitutionSet`, `DeclRef<T>`, `LookupResult`, `TypeExp`, `WitnessTable`, `SyntaxClass<T>`, `BuiltinOperationKind`, `printDiagnosticArg`, and `getDiagnosticPos`.
- Compared the document structure against `docs/generated/design/_meta/prompts/_common.md` and `docs/generated/design/_meta/prompts/ast-reference-base.md`.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Roots`, lines 71-243 | The root subsections do not follow the required list and order. The prompt requires exactly `NodeBase`, `SyntaxNode`, `DeclBase`, `Decl`, `ModifiableSyntaxNode`, `Stmt`, `Expr`, `Modifier`, `Val`, and `Type` in that order, but the document adds `SyntaxNodeBase` and `DeclRefBase` as peer subsections and orders `Modifier` before `ModifiableSyntaxNode`/`DeclBase`/`Decl`. | `docs/generated/design/_meta/prompts/ast-reference-base.md` lines 32-34 specify the exact subsection order. `source/slang/slang-ast-base.h` lines 133 and 631 show `SyntaxNodeBase` and `DeclRefBase` exist, but they are not in that required roots list. | Rewrite `## Roots` to the exact required subsection sequence. Keep `SyntaxNodeBase` and `DeclRefBase` only as supporting context inside the `SyntaxNode` and `Val`/`Type` discussions, or in the hierarchy diagram, rather than as extra root subsections. |
| F-002 | major | `## Support types`, lines 245-264 | The support table omits the required diagnostic-related helpers. The prompt explicitly calls for `DiagnosticInfo`-related helpers, but the table has no row for diagnostic helper APIs such as `printDiagnosticArg` or `getDiagnosticPos`. | `docs/generated/design/_meta/prompts/ast-reference-base.md` lines 47-51 require `DiagnosticInfo`-related helpers. `source/slang/slang-ast-support-types.h` lines 57-66 and 82-85 declare the relevant diagnostic helper overloads. | Add one or more `## Support types` rows covering the diagnostic helper APIs, for example `printDiagnosticArg(...)` and `getDiagnosticPos(...)`, with a concise purpose tied to diagnostic formatting/source locations. |
| F-003 | minor | `## Source`, lines 31-34 | The page says `slang-ast-forward-declarations.h` "is FIDDLE-generated," but the watched file is a checked-in source header that contains FIDDLE template blocks and includes generated `.fiddle` output. Calling the whole header generated is slightly inaccurate. | `source/slang/slang-ast-forward-declarations.h` lines 9-18 show the source header declares `ASTNodeType` and includes `slang-ast-forward-declarations.h.fiddle` for generated output. | Change the sentence to say the header contains FIDDLE-generated output for `ASTNodeType` and forward declarations, rather than saying the header itself is FIDDLE-generated. |

## No-issues notes

- The hierarchy diagram's inheritance edges match the watched header, including `Val -> Type`, `Val -> DeclRefBase`, and `ModifiableSyntaxNode -> DeclBase`/`Stmt`.
- The field summaries for `NodeBase`, `SyntaxNodeBase`, `Modifier`, `ModifiableSyntaxNode`, `Decl`, `Expr`, `Val`, and `Type` match the declarations I checked.
- The `Scope` note is source-aligned: `Scope` is a concrete `NodeBase`, not a syntax node.
