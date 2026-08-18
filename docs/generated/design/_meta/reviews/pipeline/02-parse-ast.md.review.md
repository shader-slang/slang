---
review_report: true
reviewer_model: gpt-5.6-sol
reviewed_at: 2026-08-04T08:19:21+00:00
target_doc: pipeline/02-parse-ast.md
target_doc_source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_watched_paths_digest: 0199faf4e426ba466aba526a350e702abad4289ecdf404224cde287299d5da24
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
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

# Review report for pipeline/02-parse-ast.md

## Summary

The page is comprehensive and mostly source-aligned, with all links and line-number citations resolving correctly. Three minor inaccuracies remain: generic-application speculation does not use `ParsingCursor`, C-style generic functions contradict the claim that every generic declaration follows a declaration keyword, and `ASTBuilder::create<T>()` rejects `Val` types at compile time rather than asserting at runtime.

## Items checked

- Verified 30 cited line anchors and ranges against `slang-parser.cpp`, `slang-parser.h`, and `slang-ast-base.h` at commit `53b76e6d3009b8e6434d41573524c7ce5c499d23`; every citation lands on the stated declaration or behavior.
- Spot-checked more than 25 factual claims, including parser entry-point signatures, parsing stages, deferred bodies, syntax lookup, recovery sets, AST family bases, FIDDLE-backed casts, AST interning, generic constraints, associated-type constraint relocation, modifier parsing, operator-name diagnostics, and floating-point literal diagnostics.
- Resolved all 45 relative Markdown links (22 unique targets) at the recorded source commit and confirmed every referenced generated peer is present in the manifest.
- Swept 170 backticked identifiers and 15 source filenames against the recorded source tree; no fabricated symbol or filename was found.
- Verified the required section structure, front-matter fields, 32 KB size cap, and watched-path digest; `regenerate.py lint` reported no errors or warnings.

## Findings

| ID    | Severity | Location                                                         | Description                                                                                                                                                                                                                                                                                                                                                                    | Evidence                                                                                                                                                                                                                                                                                                      | Recommendation                                                                                                                                                                                       |
| ----- | -------- | ---------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| F-001 | minor    | `## Parser`, lines 48-52; `## Generics ambiguity`, lines 274-281 | The page says `TokenReader::ParsingCursor` save/restore is used by the generic-application disambiguator and describes that path as parsing then rolling back. `tryParseGenericApp` instead copies the `Parser`, speculatively parses on the copy, and leaves the original token reader untouched; it reparses on the original parser only after the follow-set check commits. | `source/slang/slang-parser.cpp:3009-3048` constructs `Parser newParser(*parser)`, parses with `newParser`, and calls `parseGenericApp(parser, base)` only on success. `source/slang/slang-parser.cpp:7956-7965` delegates the `<` case to that helper.                                                        | Replace the `ParsingCursor` claim with the actual copied-parser speculation mechanism, and say failure leaves `<` unread for ordinary infix parsing rather than saying the parser rolls back.        |
| F-002 | minor    | `## Generics ambiguity`, lines 282-285                           | The statement that generic declarations are unambiguous because they always appear after a declaration keyword is too broad. Traditional C-style generic functions such as `float f<T>(T x)` reach `ParseDeclaratorDecl`; there, `<` after the parsed declarator is itself one of the tokens that selects the function-declaration branch.                                     | `source/slang/slang-parser.cpp:3576-3584` begins the traditional declarator path, and `source/slang/slang-parser.cpp:3680-3702` treats either `(` or `<` after the declarator as a function and calls `parseTraditionalFuncDecl`; `source/slang/slang-parser.cpp:2357-2367` then calls `parseOptGenericDecl`. | Reword this to say declaration context makes the generic parameter list unambiguous, while noting that both keyword-led declarations and C-style function declarators can introduce a `GenericDecl`. |
| F-003 | minor    | `### ASTBuilder`, lines 259-263                                  | The page says `ASTBuilder::create<T>()` “asserts” if `T` is a `Val`, which suggests a runtime assertion. The API uses `static_assert`, so such code fails to compile.                                                                                                                                                                                                          | `source/slang/slang-ast-builder.h:249-255` contains `static_assert(!IsBaseOf<Val, T>::Value, "ASTBuilder::create cannot be used to create a Val, use getOrCreate instead.")`.                                                                                                                                 | Change “it asserts” to “it fails to compile via `static_assert`” and retain the direction to use `getOrCreate<T>()`.                                                                                 |

## No-issues notes

- The two-stage parsing account matches `parseSourceFile`, `parseOptBody`, `SemanticsVisitor::maybeParseStmt`, and `parseUnparsedStmt`.
- The syntax-as-declaration account matches `tryLookUpSyntaxDecl`, `tryParseUsingSyntaxDecl`, `g_parseSyntaxEntries`, and `populateBaseLanguageModule`.
- All generic-constraint forms and diagnostics described in the page match `maybeParseGenericConstraints`, `parseOptionalGenericConstraints`, `parseAssocType`, and `parseInterfaceConstraintDecl`.
