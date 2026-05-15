---
remediation_report: true
remediator_model: claude-opus-4.7
remediated_at: 2026-05-15T20:00:00+00:00
target_doc: syntax-reference/keywords-and-builtins.md
review_report: ../../reviews/syntax-reference/keywords-and-builtins.md.review.md
target_doc_source_commit_before: e75b9a3d03659cefb39882da3adecb2eb8751e0d
target_doc_source_commit_after: 470b96e8c29ca660c537d4d0f88cc21a12f962e6
actions:
  fixed: 3
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for syntax-reference/keywords-and-builtins.md

## Summary

All three findings addressed: `struct`/`class`/`enum` are now
described as parser-lookahead special cases (not as
`populateBaseLanguageModule` registrations), `catch` is described as
the `do ... catch` handler form, and `new` is added to the
expression keywords table.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | The parser handles `struct`/`class`/`enum` directly by identifier lookahead in `parseDecl` and `parseDeclWithModifiers`, not through `populateBaseLanguageModule`. | Rewrote the trailing paragraph of `## Decl keywords` to describe `struct`, `class`, and `enum` as parser-lookahead special cases, citing `slang-parser.cpp` lines 3118-3134 (`parseDecl`) and 10170-10358 (`parseDeclWithModifiers`) and naming the per-kind parse routines (`parseStructDecl`, `parseClassDecl`, `parseEnumDecl`). |
| F-002 | fixed | `try` is an expression keyword; `catch` is paired with `do`, not `try`. | Edited the `catch` row in the Statement keywords table to cite lines 6919-6967 and say "the `do ... catch` handler form; `catch` does **not** pair with `try` at statement level". Added a concluding paragraph noting that `try` is an *expression* keyword and pointing readers to the `## Expression keywords` table. |
| F-003 | fixed | The prompt for this page explicitly calls out `new`, and the parser handles it specially in `parsePrefixExpr` (lines 9206-9209); it was missing from the page. | Added a `new` row to the Expression keywords table with a citation to `slang-parser.cpp` lines 9206-9209 and a note that it is parsed specially in `parsePrefixExpr` (not via `_makeParseExpr`). |
