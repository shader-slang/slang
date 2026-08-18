---
remediation_report: true
remediator_model: claude-opus-5
remediated_at: 2026-08-04T09:07:00Z
target_doc: pipeline/02-parse-ast.md
review_report: ../../reviews/pipeline/02-parse-ast.md.review.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: 53b76e6d3009b8e6434d41573524c7ce5c499d23
actions:
  fixed: 3
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for pipeline/02-parse-ast.md

## Summary

All three minor findings were verified against the source at the recorded
commit and all three were fixed with local edits. The largest change is in
`## Generics ambiguity`, where the rollback description was replaced with the
copied-parser speculation that `tryParseGenericApp` actually performs, and the
over-broad "always after a declaration keyword" claim was widened to cover
C-style function declarators. No findings were rejected, deferred, or escalated.

## Actions

| Finding ID | Action | Rationale                                                                                                                                                                                                                                                                                                    | Fix summary                                                                                                                                                                                                                      |
| ---------- | ------ | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| F-001      | fixed  | Confirmed: `source/slang/slang-parser.cpp:3018-3021` builds `Parser newParser(*parser)` with a throwaway `DiagnosticSink`, speculates on the copy, and only calls `parseGenericApp(parser, base)` at `:3044` after the FOLLOW-set check; the real `tokenReader` is never rolled back because it never moved. | `## Parser`: removed the "used by the heuristic disambiguator" attribution from the `ParsingCursor` sentence. `## Generics ambiguity`: rollback wording replaced with the copied-parser speculation and the unread-`<` fallback. |
| F-002      | fixed  | Confirmed: `slang-parser.cpp:3680-3702` treats `(` or `<` after a traditional declarator as a function and calls `parseTraditionalFuncDecl`, which reaches `parseOptGenericDecl`, so `float f<T>(T x)` yields a `GenericDecl` with no leading declaration keyword.                                           | `## Generics ambiguity`: reworded to declaration _context_ and added the C-style declarator case alongside keyword-led declarations. Consolidated with the F-001 edit.                                                           |
| F-003      | fixed  | Confirmed: `source/slang/slang-ast-builder.h:252-254` uses `static_assert(!IsBaseOf<Val, T>::Value, ...)`, a compile-time rejection rather than a runtime assert.                                                                                                                                            | `### ASTBuilder`: "it asserts if `T` is a `Val`" -> a `Val` type fails to compile, rejected by a `static_assert`; the `getOrCreate<T>()` direction is retained.                                                                  |
