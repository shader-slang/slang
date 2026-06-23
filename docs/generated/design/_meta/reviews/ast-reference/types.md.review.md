---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-12T13:17:20+00:00
target_doc: ast-reference/types.md
target_doc_source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_watched_paths_digest: 41373768d4b2491e1d78d77785d48737b8704c56161f5e163acb779e61a784be
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

# Review report for ast-reference/types.md

## Summary
The types page satisfies the concrete-class coverage rule: all concrete `FIDDLE()` classes in `slang-ast-type.h` appear in the Nodes tables and abstract classes do not appear as rows. The only issue found is a hierarchy-diagram parent edge for `ThisType` that conflicts with the source and with the table row later in the page.

## Items checked
- Ran `python3 docs/generated/design/_meta/regenerate.py show ast-reference/types.md` and used the listed prompt, dependency docs, and watched files at `eb9403ef595a99c2ff6def1d538dbd7a792d9371`.
- Compared all 119 concrete `FIDDLE()` classes in `source/slang/slang-ast-type.h` with the `## Nodes` tables and verified that no `FIDDLE(abstract)` class appears as a row.
- Checked immediate parent names in the Nodes tables against the header declarations and verified the required `Val -> Type` relationship.
- Spot-checked source-backed claims for `DeclRefType`, `BasicExpressionType`, vector/matrix types, `ArrayExpressionType`, pointer and parameter-passing types, resource and texture types, parameter-group types, data-layout types, `FuncType`, `ThisType`, `ExtractExistentialType`, `ExistentialSpecializedType`, `AndType`, `ModifiedType`, and pack types.
- Resolved the relative links and anchors; the body has no source line-number citations.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | minor | `## Family hierarchy` | The diagram shows `Type --> ThisType`, but `ThisType` is declared as a `DeclRefType` subclass. The Nodes table later uses `DeclRefType` as the parent, so the diagram is internally inconsistent as well as source-inaccurate. | `source/slang/slang-ast-type.h:1308-1311` declares `class ThisType : public DeclRefType`. | Change the hierarchy edge to `DeclRefType --> ThisType` so the diagram matches both the source and the Nodes table. |
