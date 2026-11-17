---
review_report: true
reviewer_model: gpt-6
reviewed_at: 2026-11-03T06:56:19Z
target_doc: ast-reference/base.md
target_doc_source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_watched_paths_digest: ed4d65bac6747895fa0684522c80c4fd43cc432937a1a9a770725bc86e145560
source_commit: 56b1b0b83fb96f88f36da7e37cd9c2c7f27af70c
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: pass
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 3
severity_breakdown:
  critical: 0
  major: 0
  minor: 3
  nit: 0
---

# Review report for ast-reference/base.md

## Summary

The document satisfies its page-specific structure and accurately describes the principal AST roots, fields, and support types. Three small inaccuracies remain; the most consequential attributes shared declaration modifiers to the group's inheritance rather than the parser's explicit attachment to each declaration.

## Items checked

- Read `_review.md`, `_common.md`, and the document-specific prompt identified by `regenerate.py show ast-reference/base.md`.
- Verified against the **current working tree**. `git diff 2518a942d59b1228fc8cc70a37739baab82b6002 -- source/` returned no differences, confirming the supplied source-equivalence condition.
- Checked all 11 hierarchy edges and the ten required root subsections against `source/slang/slang-ast-base.h`.
- Spot-checked more than 20 claims, including node tags and casts, builder ownership, source locations, modifier fields, declaration check-state monotonicity, expression fields, value operands, reflection ownership, scope creation, declaration grouping, `QualType` flags, substitution fields, builtin enumeration membership, and diagnostic helper signatures.
- Checked builtin attribute syntax, lowering of builtin tags, ray-tracing tag consumers, and substitution-cache behavior against their source implementations.
- Resolved all 56 Markdown link occurrences, covering 15 unique targets; all nine referenced generated pages have manifest entries. Read the parsing dependency only for cross-reference checks.
- Found no line-number citations in the target document's body.
- Checked required front-matter keys and formats, section order, root field lists, family links, support-table columns, Mermaid conventions, and Liquid safety. The document is 18,449 bytes, below its 49,152-byte cap.
- No files were written.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | minor | `### DeclBase (ModifiableSyntaxNode)`, lines 145–148 | The explanation that modifiers apply to every declaration `Because the group node is itself a ModifiableSyntaxNode` gives the wrong mechanism. The parser explicitly completes each member with the shared modifier list; inheritance does not propagate the group's modifiers. | `source/slang/slang-parser.cpp:2954–2985`: `DeclGroupBuilder::beginGroup` inserts a shared-modifier marker, and `addToGroup` calls `CompleteDecl(parser, newDecl, containerDecl, modifiers, nullptr)`. At line 6324, the parser states that a group's members have already been completed. | Replace the inheritance-based explanation with a sentence explaining that `DeclGroupBuilder` attaches the shared modifiers to each member through `CompleteDecl`, making both `P` and `Q` static and const. |
| F-002 | minor | `## Support types`, lines 291–294 | The introduction says all listed helpers are declared in `slang-ast-support-types.h` and `are not NodeBase subclasses`, but the table includes `Scope`, correctly identified in its own row as a node declared in another header. | `source/slang/slang-ast-base.h:111–112`: `FIDDLE()` followed by `class Scope : public NodeBase`. | Qualify the introduction: most entries are non-node helpers declared in the support-types header; `Scope` is the explicit exception, declared in the base header. Keep the existing `Scope` row. |
| F-003 | minor | `## Support types`, `SyntaxClass<T>` row, line 306 | Calling this a handle to a `concrete AST class` unnecessarily excludes abstract classes. The casting machinery also uses metadata handles for abstract bases. | `source/slang/slang-ast-support-types.h:791–801`: `isSubClassOf<U>()` obtains `getSyntaxClass<U>()` without restricting `U` to concrete classes. `source/slang/slang-ast-base.h:65–67` uses that operation for node casts. | Change the purpose to “Reflection-style handle to an AST class, including abstract bases, used by the visitor and casting infrastructure.” |