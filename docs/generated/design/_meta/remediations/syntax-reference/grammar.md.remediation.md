---
remediation_report: true
remediator_model: claude-sonnet-5-5
remediated_at: 2026-11-03T12:00:00Z
target_doc: syntax-reference/grammar.md
review_report: ../../reviews/syntax-reference/grammar.md.review.md
target_doc_source_commit_before: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
actions:
  fixed: 7
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 1
  escalated: 0
---

# Remediation report for syntax-reference/grammar.md

## Summary

Seven findings were fixed in the document after checking each against `source/slang/slang-parser.cpp`; one (F-008) was deferred. F-005 was fixed for the non-terminals the reviewer named, not as a full audit of every production. The document is now 63,170 bytes, under the 65,536-byte manifest cap.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `ParseGenericDeclImpl` reads `<` and `>` (slang-parser.cpp:1818, 1850); the bare `GenericParams?` was wrong. | Added `GenericParamList ::= '<' GenericParams '>'` and used it in all eight declaration rules, the extension and type-alias rules, and `GenericDecl` |
| F-002 | fixed | `tryParseGenericApp` leaves `baseKind` Unknown for unrecognized shapes and then speculates (slang-parser.cpp:3079-3177). | Rewrote the paragraph to give the three outcomes |
| F-003 | fixed | Parsing only builds an `UncheckedAttribute` (slang-parser.cpp:1046-1062); lookup is deferred to semantic checking. | Replaced the syntax-decl lookup sentence |
| F-004 | fixed | `Operator`, `RegToken`, `CaseStmt`, `DefaultStmt`, `GenericSpecialization` and `ModifierArgs` were undefined. | Defined `OperatorToken` (ParseDeclName), `Register`, `CaseStmt` and `DefaultStmt`; used `GenericArgs`; marked `ModifierTail` as deliberately abbreviated |
| F-005 | fixed | The named non-terminals lacked parser-function attribution. | Added citations to `TypeList`, `ArraySuffix`, `UnaryOp`, `ModifierList`, `ArgList`; other productions not re-audited |
| F-006 | fixed | `parseFuncTypeExpr` accepts an immediate `)` (slang-parser.cpp:3403-3416). | Made the `functype` parameter list optional |
| F-007 | fixed | `parseDeclarator` recurses after `*` (slang-parser.cpp:2784-2803). | Made `Declarator` pointer branch recursive |
| F-008 | deferred | The manifest cap (65,536) conflicts with the prompt's 48 KB line (`_meta/prompts/syntax-grammar.md:96-98`); cutting about 15 KB would drop required productions. Needs the operator to reconcile prompt and manifest. | — |
