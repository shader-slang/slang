---
remediation_report: true
remediator_model: claude-opus-5
remediated_at: 2026-08-04T09:05:00Z
target_doc: ast-reference/expressions.md
review_report: ../../reviews/ast-reference/expressions.md.review.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: 53b76e6d3009b8e6434d41573524c7ce5c499d23
actions:
  fixed: 6
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ast-reference/expressions.md

## Summary

All six findings were checked against the source at
`53b76e6d3009b8e6434d41573524c7ce5c499d23` and all six held up, so all
six were fixed. Four were single-sentence corrections (audience
placement, member-name parsing, literal token population, overload
diagnostics), one added two edges to the hierarchy diagram, and the
major finding removed the forbidden IR-lowering prose from the
`CastOptionalExpr` callout. `regenerate.py lint
ast-reference/expressions.md` passes.

## Actions

| Finding ID | Action | Rationale                                                                                                                                                                                                                              | Fix summary                                                                                                              |
| ---------- | ------ | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------ |
| F-001      | fixed  | `_common.md:65-66` requires the first paragraph to state coverage and reader; the audience sat in its own paragraph.                                                                                                                   | Folded the `Audience:` sentence into the opening paragraph and deleted the separate paragraph.                           |
| F-002      | fixed  | The per-doc prompt (`ast-reference-expressions.md:17`) requires the `OverloadedExpr` family in the diagram; both classes derive directly from `Expr` (`source/slang/slang-ast-expr.h:57,76`).                                          | Added `Expr --> OverloadedExpr` and `Expr --> OverloadedExpr2` to the mermaid diagram.                                   |
| F-003      | fixed  | `source/slang/slang-parser.cpp:9175-9206` creates `StaticMemberExpr`/`MemberExpr`/`DerefMemberExpr` and assigns their names directly, so member names never pass through `VarExpr`.                                                    | Narrowed the claim to bare/unqualified names and named the three member-position node types as exceptions.               |
| F-004      | fixed  | Numeric/char/string paths set `constExpr->token` (`slang-parser.cpp:8595,8702,8985,9014`); the four keyword-literal callbacks (`slang-parser.cpp:7984-8009`) set no token, only `loc` from the keyword (`slang-parser.cpp:1171-1173`). | Split the sentence: token-bearing literal paths vs keyword literals that record only source location.                    |
| F-005      | fixed  | Two distinct diagnostics exist: `NoApplicableOverloadForNameWithArgs` (`source/slang/slang-check-overload.cpp:3510`) and `AmbiguousOverloadForNameWithArgs` (`slang-check-overload.cpp:3603`).                                         | Replaced the single "ambiguous" claim with the two failure modes.                                                        |
| F-006      | fixed  | The per-doc prompt (`ast-reference-expressions.md:58-59`) forbids IR lowering of expressions on this page.                                                                                                                             | Removed the two lowering sentences and pointed at `../pipeline/04-ast-to-ir.md`; the AST-field explanation is unchanged. |
