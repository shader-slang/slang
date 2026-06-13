---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-12T13:17:07+00:00
target_doc: ast-reference/declarations.md
target_doc_source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_watched_paths_digest: ad2a7f5a76a091d730e7cbf54483a1de9f01f4aeeb2e3ec0963d4e400436f2b6
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
  major: 0
  minor: 1
  nit: 0
---

# Review report for ast-reference/declarations.md

## Summary
The declaration reference is structurally complete and mostly source-aligned: its 60 table rows match the 60 concrete FIDDLE-declared declaration classes, and abstract FIDDLE bases are kept out of the table. The only issue found is a minor spelling mismatch for the source token represented by `RequireCapabilityDecl`.

## Items checked
- Ran `regenerate.py show ast-reference/declarations.md` and used the listed prompt, dependency docs, and watched paths.
- Verified the target front matter fields against the current document and HEAD.
- Compared every `## Nodes` table row against the concrete FIDDLE class list in `source/slang/slang-ast-decl.h`: 60 rows, 60 concrete classes, no missing concrete classes, and no abstract FIDDLE classes in the table.
- Checked the family hierarchy against abstract bases such as `ContainerDecl`, `VarDeclBase`, `AggTypeDeclBase`, `TypeConstraintDecl`, `CallableDecl`, `FunctionDeclBase`, `AccessorDecl`, `NamespaceDeclBase`, `IncludeDeclBase`, and `GenericTypeParamDeclBase`.
- Checked relative links and grammar anchors used by the Source, Nodes, Notable nodes, and See also sections.
- Spot-checked more than 10 factual claims, including `GenericDecl`, `FuncExtensionDecl`, `InheritanceDecl`, `SyntaxDecl`, accessor declarations, namespace/module/file declarations, interface constraints, parser entry points, and front-matter fields. The document has no source line-number citations in the body.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | minor | `## Nodes`, `RequireCapabilityDecl` row | The summary spells the surface declaration as ``require_capability`` without the leading double underscore, but the parser and grammar use ``__require_capability``. | `source/slang/slang-parser.cpp:10497` registers `_makeParseDecl("__require_capability", parseRequireCapabilityDecl)`, and `docs/generated/design/syntax-reference/grammar.md:245` defines `RequireCapabilityDecl ::= '__require_capability' '(' CapabilityExpr ')' ';'`. | Change the row summary to say ``__require_capability`` declaration. |

## No-issues notes
- Required sections appear in the expected order: source, family hierarchy, nodes, notable nodes, and see also.
- Required notable-node topics are covered, including generics, extensions, aggregate declarations, inheritance, syntax declarations, namespaces/modules/files, enums, accessors, and interface requirements.
