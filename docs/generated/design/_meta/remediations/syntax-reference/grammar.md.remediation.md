---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-30T14:06:08Z
target_doc: syntax-reference/grammar.md
review_report: ../../reviews/syntax-reference/grammar.md.review.md
target_doc_source_commit_before: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_source_commit_after: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
actions:
  fixed: 0
  rejected_bogus: 8
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for syntax-reference/grammar.md

## Summary
All eight findings are rejected as bogus. Each finding asserts the target document states a defective grammar form, but the current target text already states the correct form, and the cited watched source confirms that text is accurate. The review evidently captured an earlier snapshot; its cited line numbers no longer match the live body. No edit is needed, so the target document was not changed and the after-commit equals the before-commit. Action breakdown: eight rejected-bogus; zero fixed, rejected-out-of-scope, deferred, or escalated.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | rejected-bogus | Doc already lists `FuncExtensionDecl` under `CoreDecl` (grammar.md:115) and groups the internal keywords `__associatedfunc`, `type_param`, `__generic_value_param`, `semantic`, `__ignored_block`, `__transparent_block` in `InternalDecl` (grammar.md:124,132-140); matches g_parseSyntaxEntries at source/slang/slang-parser.cpp:10906-10934. | — |
| F-002 | rejected-bogus | Doc already gives `ModuleName` and `ImportPath` as a dotted identifier or string literal, the omitted-name fallback, and the `implementing` file-reference form (grammar.md:73-89); matches source/slang/slang-parser.cpp:1320-1349 and 1379-1402. | — |
| F-003 | rejected-bogus | Doc already models the dot-or-colon-colon namespace chain (grammar.md:91-93) and `UsingDecl` as `using namespace? Expr ;` (grammar.md:94-96); matches the separator loop and ParseExpression call at source/slang/slang-parser.cpp:4436-4563. | — |
| F-004 | rejected-bogus | Doc already adds the type-coercion `WhereTerm` of the form Type LParen Type RParen optional-implicit (grammar.md:252-253,540-542); matches TypeCoercionConstraintDecl construction at source/slang/slang-parser.cpp:2036-2048. | — |
| F-005 | rejected-bogus | Doc already states `ContinueStmt` as continue then semicolon with a no-label note (grammar.md:337-338); matches ParseContinueStatement reading only continue then Semicolon at source/slang/slang-parser.cpp:7539-7545, distinct from the break label form. | — |
| F-006 | rejected-bogus | Doc already states `ThrowStmt` as throw Expr with a note that the semicolon is not consumed and is parsed as a separate EmptyStmt (grammar.md:342-344); matches ParseThrowStatement at source/slang/slang-parser.cpp:7568-7574. | — |
| F-007 | rejected-bogus | Doc already lists is and as at relational precedence (grammar.md:367) and in the `RelationalExpr` production with the Type-operand note (grammar.md:388-393); matches the special-cased IsTypeExpr/AsTypeExpr handling at source/slang/slang-parser.cpp:7858-7882. | — |
| F-008 | rejected-bogus | Doc already restricts `AccessorName` to get, set, ref with an Unexpected-diagnostic note and no modify (grammar.md:263-264); matches the get/set/ref branches and Unexpected fallback at source/slang/slang-parser.cpp:4692-4707. | — |
