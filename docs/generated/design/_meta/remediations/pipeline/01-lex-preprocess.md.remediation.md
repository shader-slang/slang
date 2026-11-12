---
remediation_report: true
remediator_model: claude-sonnet-5-5
remediated_at: 2026-11-02T21:25:00Z
target_doc: pipeline/01-lex-preprocess.md
review_report: ../../reviews/pipeline/01-lex-preprocess.md.review.md
target_doc_source_commit_before: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
actions:
  fixed: 5
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for pipeline/01-lex-preprocess.md

## Summary

All five findings were verified against the current source and fixed in
the target document. Fixes cover the dangling language-reference link,
the token-type file attribution, the literal-scrubbing qualification,
the split of string versus character decode diagnostics, and a
condensation pass bringing the document under 24 KB.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `docs/language-reference/expressions-literal.md` does not exist; `_decodeStringEscape` at `source/compiler-core/slang-lexer.cpp:1440` is the authority | Dropped the dangling link; the grammar is now stated as implemented in `_decodeStringEscape` |
| F-002 | fixed | `TokenList`, `TokenSpan`, `TokenReader` are declared in `source/compiler-core/slang-lexer.h` (lines 13, 24, 42 by the review; structs confirmed) | Moved the three types to the lexer file bullet; token-file bullet keeps `Token`, `TokenType`, `TokenFlags` |
| F-003 | fixed | `slang-lexer.cpp:337` sets `ScrubbingNeeded` and `:2536-2577` strips backslash-newline before `setContent` | Literal text now described as undecoded, with continuations removed when `ScrubbingNeeded` is set |
| F-004 | fixed | `getCharLiteralValue` (`slang-lexer.cpp:1683`) ignores the unicode flag; `outOfRangeCodeUnit` / `outOfRangeCodePointForUtf8` are emitted only at `:1915` / `:1937` in string decoding | Failure-modes bullet now says both helpers report malformed escapes, string decoding adds range checks, character decoding the one-character rule |
| F-005 | fixed | Prompt checklist requires under 24 KB; document was 26,969 bytes | Condensed redundant prose across several sections; now under 24,576 bytes with all sections kept |
