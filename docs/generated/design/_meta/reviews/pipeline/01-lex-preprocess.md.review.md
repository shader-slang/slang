---
review_report: true
reviewer_model: gpt-6
reviewed_at: 2026-11-02T21:17:02Z
target_doc: pipeline/01-lex-preprocess.md
target_doc_source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_watched_paths_digest: 25c315302bec8a98147ed08e896ab7b882f1e7ddd78ee373f4e252ec03861394
source_commit: be3c344414f6541ba6731813b36591a94cd05482
checklist:
  factual_accuracy: partial
  cross_references: partial
  completeness: pass
  style_consistency: partial
  source_alignment: partial
  front_matter_validity: pass
finding_count: 5
severity_breakdown:
  critical: 0
  major: 1
  minor: 4
  nit: 0
---

# Review report for pipeline/01-lex-preprocess.md

## Summary

The document covers the required structure and accurately describes most checked behavior. All 33 line-number citations match the source. The principal finding is a dangling language-reference link; smaller corrections concern declaration locations, literal handling, and the prompt’s size limit.

## Items checked

- Read `_review.md`, `_common.md`, the document-specific prompt, and the resolved watched paths returned by `regenerate.py show`.
- Verified against the **current working tree**. The comparison against the document’s source commit showed no differences under `source/`; HEAD matches the supplied report commit.
- Checked all **33 line-number citations**: nine in `slang-lexer.cpp`, two in `slang-parser.cpp`, one in `slang-compile-request.cpp`, and 21 in `slang-preprocessor.cpp`.
- Spot-checked more than **20 behavioral and structural claims**, including BOM decoding, token fields, source-location representation, diagnostic suppression, UTF-8 recovery, literal suffix classification, escape decoding, macro argument replay, recursion suppression, conditional skipping, directive dispatch, language selection, include handling, warning-state persistence, macro token locations, and EOF handling.
- Checked all **44 relative-link occurrences**, covering **19 distinct destinations**; 18 destinations exist. All three referenced generated peer pages are present in the manifest. Read the dependency overview only to check cross-reference consistency.
- Checked required sections, front-matter keys and value formats, relative-link syntax, Liquid safety, and document size. The file is **26,969 bytes**.
- Performed read-only inspection; no report or repository file was written.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | Literal scanning and value extraction, line 206 | The link to `docs/language-reference/expressions-literal.md` is dangling: that file is absent from the working tree. | Target document line 206; filesystem check of `docs/language-reference/expressions-literal.md`. The implemented escape grammar is available in `source/compiler-core/slang-lexer.cpp:1440`. | Remove the unavailable language-reference link and cite `slang-lexer.cpp`’s `_decodeStringEscape` as the local authority for the described grammar. |
| F-002 | minor | Lexer, lines 47–50 | The file-pointer bullet attributes `TokenList`, `TokenSpan`, and `TokenReader` to `slang-token.h` / `slang-token.cpp`, although those types belong to the lexer files. | `source/compiler-core/slang-lexer.h:13`, `:24`, and `:42` declare the three types; `source/compiler-core/slang-lexer.cpp:24` begins their implementations. | Move those three names into the `slang-lexer.h` / `.cpp` bullet. Keep `Token`, `TokenType`, and `TokenFlags` in the token-file bullet. |
| F-003 | minor | Lexer flags and special-case rules, lines 106–108 | Literal text is described as being kept `verbatim`, without accounting for escaped-newline removal. Tokens spanning a backslash line continuation contain scrubbed text. | `source/compiler-core/slang-lexer.cpp:337` sets `ScrubbingNeeded`; `:2536–2577` removes escaped newlines before assigning token content. | Qualify the statement: literal spelling is retained without value decoding, except that backslash-newline continuations are removed from token content. |
| F-004 | minor | Failure modes, lines 469–471 | The sentence jointly attributes out-of-range code-unit and code-point diagnostics to `getStringLiteralTokenValue` and `getCharLiteralValue`. Those range checks are specific to string decoding; character escape decoding returns the decoded 32-bit value without enforcing byte or UTF-8 representability. | `source/compiler-core/slang-lexer.cpp:1683–1686` discards the escape’s Unicode distinction. The `outOfRangeCodeUnit` and `outOfRangeCodePointForUtf8` emissions occur in string decoding at `:1915` and `:1937`. | Separate the helpers’ responsibilities: both diagnose malformed escapes; string decoding additionally checks byte/UTF-8 representability, while character decoding checks malformed bare UTF-8 and the one-character rule. |
| F-005 | minor | Whole document | At 26,969 bytes, the document exceeds the document-specific requirement `Document length under 24 KB`, although it remains below the manifest’s 32,768-byte cap. | `docs/generated/design/_meta/prompts/pipeline-01-lex-preprocess.md`, final quality-checklist item; measured target-file size: 26,969 bytes. | Condense repeated scanning-versus-decoding and diagnostic-suppression explanations until the complete document is below 24 KB, retaining every required section. |