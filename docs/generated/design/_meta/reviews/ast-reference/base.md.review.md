---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-12T13:17:20+00:00
target_doc: ast-reference/base.md
target_doc_source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_watched_paths_digest: 6e50af3ceb4925d38a918b256232bcea84837870228b5ab013c5722f1eb1d9a7
source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
checklist:
  factual_accuracy: pass
  cross_references: pass
  completeness: partial
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

# Review report for ast-reference/base.md

## Summary
The page is broadly accurate against `slang-ast-base.h` and the root hierarchy edges match the source. The main issue is that the required support-type table omits a prompt-required helper that is declared in the watched support header.

## Items checked
- Ran `python3 docs/generated/design/_meta/regenerate.py show ast-reference/base.md` and used the listed prompt, dependency doc, and watched files at `eb9403ef595a99c2ff6def1d538dbd7a792d9371`.
- Verified the front matter fields, required title, `## Source`, `## Root hierarchy`, `## Roots`, `## Support types`, and `## See also` sections.
- Checked the root parent edges against `source/slang/slang-ast-base.h`, including `NodeBase`, `SyntaxNodeBase`, `SyntaxNode`, `Modifier`, `ModifiableSyntaxNode`, `DeclBase`, `Decl`, `Stmt`, `Expr`, `Val`, `Type`, and `DeclRefBase`.
- Checked the support-type table against `source/slang/slang-ast-support-types.h`, including `DeclRef`, `LookupResult`, `QualType`, `SubstitutionSet`, `TypeExp`, `WitnessTable`, `SyntaxClass`, and `Scope`.
- Resolved the document's relative links and anchors; the body has no source line-number citations.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Support types` | The per-page prompt requires the support table to cover `NameLoc`, but the table omits it. `NameLoc` is one of the watched support-header helpers and is used directly by `Decl::nameAndLoc`, so leaving it out makes the required support-type section incomplete. | `docs/generated/design/_meta/prompts/ast-reference-base.md:47-51` requires `NameLoc`; `source/slang/slang-ast-support-types.h:259-260` declares `FIDDLE() struct NameLoc`; `source/slang/slang-ast-base.h:771` uses `NameLoc nameAndLoc` on `Decl`. | Add a `NameLoc` row to `## Support types` with `slang-ast-support-types.h` as the header and a purpose such as "pairs a `Name*` with the `SourceLoc` where it appeared." |
