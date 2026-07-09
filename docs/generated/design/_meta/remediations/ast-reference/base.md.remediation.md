---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-30T13:58:01Z
target_doc: ast-reference/base.md
review_report: ../../reviews/ast-reference/base.md.review.md
target_doc_source_commit_before: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_source_commit_after: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
actions:
  fixed: 3
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ast-reference/base.md

## Summary

All three findings were verified against the watched headers and the per-doc prompt contract and are fixed in the target document. They were valid against the reviewed baseline (the committed version, per `git diff HEAD`: `SyntaxNodeBase`/`DeclRefBase` were peer root subsections, no diagnostic-helper row existed, and the Source bullet called the whole header FIDDLE-generated). Two majors addressed contract drift: F-001 (root subsection list/order) and F-002 (missing diagnostic-helper coverage); one minor, F-003, corrected the `## Source` description of `slang-ast-forward-declarations.h`. Breakdown: fixed 3; rejected-bogus 0; rejected-out-of-scope 0; deferred 0; escalated 0.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Prompt `ast-reference-base.md` lines 32-34 mandate exactly `NodeBase, SyntaxNode, DeclBase, Decl, ModifiableSyntaxNode, Stmt, Expr, Modifier, Val, Type`; `source/slang/slang-ast-base.h:133,631` confirm `SyntaxNodeBase`/`DeclRefBase` exist but are not required roots. Reviewed baseline listed them as peer roots and placed `Modifier` early. | `## Roots` reordered to the required sequence; `SyntaxNodeBase` folded into the `SyntaxNode` subsection, `DeclRefBase` into `Val`, removing both peer subsections (doc `###` headers now at lines 74,96,114,126,144,157,167,181,195,218). |
| F-002 | fixed | Prompt lines 47-51 require `DiagnosticInfo`-related helpers; `source/slang/slang-ast-support-types.h:57-66,82-85` declare `printDiagnosticArg`/`getDiagnosticPos` overloads absent from the reviewed table. | Added `printDiagnosticArg` / `getDiagnosticPos` row to `## Support types` (line 250). |
| F-003 | fixed | Header is checked-in source with a FIDDLE template plus `.fiddle` include (`source/slang/slang-ast-forward-declarations.h:9-18`), not wholly generated as the reviewed text claimed. | `## Source` reworded: `ASTNodeType` declared in the header with FIDDLE-generated enumerators via the included `.fiddle` block (lines 31-35). |
