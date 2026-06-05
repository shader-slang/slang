---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-05T14:18:11+00:00
target_doc: ast-reference/declarations.md
target_doc_source_commit: 52339028a2aa703271533454c6b9528a534bac31
target_doc_watched_paths_digest: 4fbff6632ead047a5c7b8f1f94ae347684e102e096ad26276f0083b8dab39d3e
source_commit: fb192be9f5b3b58555e034599e072158e5c48dfd
checklist:
  factual_accuracy: pass
  cross_references: partial
  completeness: pass
  style_consistency: pass
  source_alignment: pass
  front_matter_validity: pass
finding_count: 1
severity_breakdown:
  critical: 0
  major: 1
  minor: 0
  nit: 0
---

# Review report for ast-reference/declarations.md

## Summary
The declaration reference is mostly source-aligned: its 60 table rows match all 60 concrete FIDDLE-declared classes in `slang-ast-decl.h`, and the required AST-reference sections are present. The one finding is a dangling grammar link in the `AttributeDecl` row.

## Items checked
- Ran `regenerate.py show ast-reference/declarations.md` and used the listed prompt, dependency docs, and watched paths.
- Verified the target front matter fields and checked that the watched C++ files did not change between the target source commit and review HEAD.
- Compared every `## Nodes` table row against the concrete FIDDLE class list in `source/slang/slang-ast-decl.h`: 60 rows, 60 concrete classes, no missing classes, no abstract classes.
- Resolved all 66 Markdown links and anchors in the target document; one anchor did not resolve.
- Spot-checked more than 10 factual claims, including `GenericDecl`, `FuncExtensionDecl`, `InheritanceDecl`, `SyntaxDecl`, accessor declarations, namespace and module declarations, parser entry points, and front-matter fields. The document has no source line-number citations in the body.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Nodes`, `AttributeDecl` row | The grammar link points at `../syntax-reference/grammar.md#modifiers-and-attributes`, but that anchor does not exist in the grammar document. | `docs/generated/design/syntax-reference/grammar.md` has `## Modifiers` at line 220 and `## Attributes and decorations` at line 252, with no `modifiers-and-attributes` heading. | Change the `AttributeDecl` grammar link to the existing `#attributes-and-decorations` anchor, or add the missing anchor in the grammar page if that is the intended shared target. |

## No-issues notes
- Required sections appear in the expected order: source, family hierarchy, nodes, notable nodes, and see also.
- Required notable-node topics are covered, including generics, extensions, aggregate declarations, inheritance, syntax declarations, namespaces and modules, enums, accessors, and interface requirements.
