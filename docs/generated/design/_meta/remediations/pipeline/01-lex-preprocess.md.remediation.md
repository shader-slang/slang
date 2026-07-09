---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-30T14:04:49Z
target_doc: pipeline/01-lex-preprocess.md
review_report: ../../reviews/pipeline/01-lex-preprocess.md.review.md
target_doc_source_commit_before: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_source_commit_after: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
actions:
  fixed: 4
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for pipeline/01-lex-preprocess.md

## Summary

All four findings were verified against the watched lexer and preprocessor sources and fixed in the target document body. The angle-include behavior is attributed to the preprocessor's `#include` resolution rather than the lexer (F-001); the literal scan/decode split was corrected so scan-time structural diagnostics are attributed to scanning while decode-time wording is reserved for value conversion (F-002); the macro-expansion model was rewritten from a "pseudo-macro environment" to the actual op/index replay (F-003); and the constructed-token source-location taxonomy was split into builtins, stringized params, and token paste with each rule stated (F-004). No findings were rejected, deferred, or escalated.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Confirmed angle-form includes are assembled in `HandleIncludeDirective` from raw tokens between `OpLess`/`OpGreater` (source/slang/slang-preprocessor.cpp:3606,3618), not lexed as one string-like token. | `### #include resolution`: angle-form concatenation attributed to `HandleIncludeDirective`, quoted form to a single `StringLiteral`; no lexer claim remains. |
| F-002 | fixed | Confirmed `_lexStringLiteralBody` diagnoses empty/multi-char literals, EOF/newline, and ill-formed escapes during scanning (source/compiler-core/slang-lexer.cpp:1363,1376,1385,1393,1497,1520); decode helpers handle value conversion (source/compiler-core/slang-lexer.cpp:1678). | Reworded scanning paragraph, decode-sink sentence, and Failure-modes scan/decode split. |
| F-003 | fixed | Confirmed op-based model: parameter ops (`ExpandedParam`/`UnexpandedParam`/`StringizedParam`) carrying an index, replayed by `_getArgTokens`/`_initCurrentOpStream` (source/slang/slang-preprocessor.cpp:2455,2480,2495); no per-invocation pseudo-macro environment. | Replaced environment/pseudo-macro sentence with op/index replay description. |
| F-004 | fixed | Confirmed three rules: builtins via `_pushStreamForSourceLocBuiltin` (source/slang/slang-preprocessor.cpp:2311,2318); stringized uses `#`-token def loc (2510,2548); paste uses `makeTokenPaste()` view from `tokenPasteLoc` (2188-2194). | Split "constructed tokens" item into builtins, stringized params, and pasted tokens with per-case loc rule. |
