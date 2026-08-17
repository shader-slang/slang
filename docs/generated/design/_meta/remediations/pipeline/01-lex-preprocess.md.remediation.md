---
remediation_report: true
remediator_model: claude-opus-5
remediated_at: 2026-08-04T09:05:00Z
target_doc: pipeline/01-lex-preprocess.md
review_report: ../../reviews/pipeline/01-lex-preprocess.md.review.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: 53b76e6d3009b8e6434d41573524c7ce5c499d23
actions:
  fixed: 8
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for pipeline/01-lex-preprocess.md

## Summary

All eight findings were independently verified against the cited source at the
recorded commit and all eight were fixed. Both major findings - universal
diagnostic suppression, and the invented macro expansion/spelling chain - were
wrong about the implementation, and their claims were replaced with the actual
behaviour. The six minor findings were single-sentence or single-clause
corrections. Nothing was rejected, deferred, or escalated.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `source/compiler-core/slang-lexer.h:147-150` is the only conditional accessor; `source/compiler-core/slang-lexer.cpp:379-383` diagnoses `invalidUtf8ByteSequence` via `lexer->m_sink`. | `### Lexer flags`: suppression scoped to `getDiagnosticSink` sites, UTF-8 bypass named; matching clause in `## Failure modes`. |
| F-002 | fixed | Scan-time syntax diagnostics confirmed at `slang-lexer.cpp:545-554`, `:1795-1798`, `:2095-2099`. | `### Literal scanning`: "close to absolute" -> "nearly clean" plus the three checks; `## Failure modes` "limited to" -> "cover", list extended. |
| F-003 | fixed | `slang-preprocessor.cpp:4915-4920` drops whitespace/newline/comment/invalid tokens, so the parser never sees them; `slang-lexer.h:137` declares `lexAllSemanticTokens`. | `### What the lexer does not do`: filtering attributed to `lexAllSemanticTokens` and `ReadAllTokens`; parser removed as a filterer. |
| F-004 | fixed | `slang-lexer.h:109` shows `kLexerFlag_SuppressDiagnostics` is the only lexer flag - no `#include` mode; `slang-preprocessor.cpp:950-981` shows per-`InputFile` lexer and `Conditional` stack. | `### Stack of input streams`: "`#include` mode" replaced with per-`InputFile` lexer plus conditional state. |
| F-005 | fixed | `slang-preprocessor.cpp:4625-4630` marks `if`, `ifdef`, `ifndef`, `else`, `elif`, `endif` `ProcessWhenSkipping`. | `### Macro expansion`: added `#ifdef` and `#ifndef` to the parenthetical. |
| F-006 | fixed | The `kDirectives` table at `slang-preprocessor.cpp:4624-4645` includes `language`/`lang` and GLSL `version`/`extension`. | `### Preprocessor directives`: "standard C / HLSL set - verify by reading" replaced with concrete categories, naming `kDirectives`. |
| F-007 | fixed | No macro-invocation `SourceView` is created, and `source/compiler-core/slang-diagnostic-sink.cpp:458-494` walks initiating locations only while `PathInfo::Type` is `TokenPaste`; `slang-source-loc.h` has no expansion/spelling split. | `## Source-location preservation`: dropped the invocation-recording claim in item 1; closing paragraph now describes the `TokenPaste`-only `seeTokenPasteLocation` walk. |
| F-008 | fixed | `slang-preprocessor.cpp:4695-4710` skips to end of line on a bad directive, `:4800-4834` pops exhausted inputs, `:4909-4913` appends EOF only after the stack empties. | `## Failure modes`: early-EOF claim replaced with per-error recovery behaviours and the `ReadAllTokens` EOF rule. |
