---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-30T13:24:00+00:00
target_doc: ast-reference/declarations.md
target_doc_source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_watched_paths_digest: 4edc6b90abd358684a288a1139580cde8c06f54b81ac587b2fb00f57fb475a83
source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: pass
  style_consistency: partial
  source_alignment: partial
  front_matter_validity: pass
finding_count: 3
severity_breakdown:
  critical: 0
  major: 1
  minor: 2
  nit: 0
---

# Review report for ast-reference/declarations.md

## Summary

The declarations reference is mostly source-aligned: the `## Nodes` table covers every concrete `FIDDLE()` declaration I found in `slang-ast-decl.h`, and the grammar links resolve. The most important issue is that the `FuncExtensionDecl` callout includes IR-lowering behavior, which the per-doc prompt explicitly forbids for this page. I also found two smaller factual mismatches: the parser entry point is mis-capitalized, and implicit getter creation is attributed to the parser even though parsing only records the storage declaration/accessor syntax.

## Items checked

- Verified the target front matter contains all required generated-doc keys and preserves the target source commit and watched-path digest.
- Compared the `## Nodes` table against the concrete `FIDDLE()` classes in `source/slang/slang-ast-decl.h`; all 61 concrete classes I found are represented, while abstract `FIDDLE(abstract)` bases are omitted from the table.
- Spot-checked inheritance and field claims for `DeclGroup`, `VarDeclBase`, `ExtensionDecl`, `StructDecl`, `SynthesizedStructDecl`, `EnumDecl`, `EnumCaseDecl`, `ConstructorDecl`, `FuncExtensionDecl`, `ModuleDecl`, `FileReferenceDeclBase`, `GenericDecl`, `GenericTypeConstraintDecl`, `GenericValueParamDecl`, `SyntaxDecl`, and `AttributeDecl`.
- Checked parser claims around `ParseDecl`, `parseGenericDecl`, `parseFuncExtensionDecl`, `parseAssocType`, `parseInterfaceConstraintDecl`, `parsePropertyDecl`, `parseAccessorDecl`, `isDeclAllowed`, and `g_parseSyntaxEntries`.
- Verified the grammar anchors used by the table resolve in `docs/generated/design/syntax-reference/grammar.md`.
- Checked relative links to dependency docs and cited source files for existence.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `### FuncExtensionDecl`, lines 168-172 | The callout includes forbidden IR-level declaration-lowering detail: `The IR lowering visitor treats the FuncExtensionDecl itself as ignored (IGNORED_CASE in slang-lower-to-ir.cpp)`. The per-doc prompt says IR-level declaration lowering belongs in the IR/cross-cutting docs, not this AST reference page. | `docs/generated/design/_meta/prompts/ast-reference-declarations.md` lines 59-64 forbid IR-level declaration lowering and type-checking semantics. The offending link targets `source/slang/slang-lower-to-ir.cpp`, which is not one of this page's watched source files. | Delete the IR-lowering sentence and the `slang-lower-to-ir.cpp` link from this callout. Keep the callout focused on the AST shape declared in `slang-ast-decl.h` and the parser shape in `parseFuncExtensionDecl`. |
| F-002 | minor | `## Source`, lines 24-27 and `## See also`, lines 285-287 | The page names the top-level declaration dispatcher as `parseDecl()`, but the parser function is `ParseDecl` with a capital `P`. This is a concrete symbol-name mismatch. | `source/slang/slang-parser.cpp` lines 298 and 5967 declare/define `static DeclBase* ParseDecl(Parser* parser, ContainerDecl* containerDecl)`. | Change both `parseDecl` references to `ParseDecl`, or phrase the text as "the declaration parser dispatch" if the exact static helper name is not important. |
| F-003 | minor | `### AccessorDecl family`, lines 226-232 | The page says `The parser will synthesize a default GetterDecl for PropertyDecls that have an initializer but no explicit accessor block.` The watched parser code parses explicit `get`/`set`/`ref` accessors and records an empty storage body as a case to treat like `{ get; }`; the actual creation is not done by the parser. | `source/slang/slang-parser.cpp` lines 4685-4772 show `parseAccessorDecl` creates accessors only when the accessor keyword is present and `parseStorageDeclBody` leaves the empty-body case as a comment. `source/slang/slang-check-decl.cpp` lines 16228-16254 creates the implicit `GetterDecl` during semantic header checking. | Reword the sentence to avoid attributing synthesis to the parser, for example: "An empty property or subscript body is later treated as an implicit `get`; semantic checking materializes the `GetterDecl`." |

## No-issues notes

- The table's concrete-class coverage matches the watched declaration header, including easy-to-miss nodes such as `FileReferenceDeclBase`, `InterfaceDefaultImplDecl`, `GenericValuePackParamDecl`, `SemanticGetterDecl`, and `AttributeDecl`.
- The associated-type constraint callout matches the watched parser flow through `parseAssocType`, `parseOptionalGenericConstraints`, `maybeParseGenericConstraints`, and `isDeclAllowed`.
- The field summaries I checked for synthesized structs/functions, generic constraints, module declarations, and syntax declarations match `slang-ast-decl.h`.
