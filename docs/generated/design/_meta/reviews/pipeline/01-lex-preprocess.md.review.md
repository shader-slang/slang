---
review_report: true
reviewer_model: gpt-5.6-sol
reviewed_at: 2026-08-04T08:17:55+00:00
target_doc: pipeline/01-lex-preprocess.md
target_doc_source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_watched_paths_digest: 1ec94893dbf01f578c85e79607171f593c06c00e297add42831695b6d50bc8d0
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: pass
  style_consistency: pass
  source_alignment: fail
  front_matter_validity: pass
finding_count: 8
severity_breakdown:
  critical: 0
  major: 2
  minor: 6
  nit: 0
---

# Review report for pipeline/01-lex-preprocess.md

## Summary

The page has the required structure, valid links, valid front matter, and many accurate implementation details, but it contains two substantial source-alignment errors and six smaller inaccuracies. Most importantly, it invents a general macro expansion/spelling-location chain that the source does not implement; only token-paste source views carry an initiating-location chain for diagnostics.

## Items checked

- Verified 32 factual claims against the nine resolved watched files at commit `53b76e6d3009b8e6434d41573524c7ce5c499d23`, including token layout, source decoding, lexer behavior, literal handling, macro ops, include handling, conditional skipping, warning tracking, and preprocessor output.
- Re-derived all four line-number citations in the body: `_decodeStringEscape` at line 1440, `getCharLiteralValue` at lines 1660-1725, `_lexStringLiteralBody` at line 1731, and `_lexRawStringLiteralBody` at line 1782.
- Resolved all 38 relative-link occurrences (17 unique targets) at the target source commit and confirmed every referenced generated peer is present in the manifest.
- Swept 129 backticked identifiers and 12 whole source filenames against the recorded source tree; apparent identifier misses were syntax placeholders or methods defined inline without a qualified spelling.
- Verified every required section, the 14,996-byte size against the 24 KB cap, the dependency page `pipeline/overview.md`, and the front-matter digest by recomputing `1ec94893dbf01f578c85e79607171f593c06c00e297add42831695b6d50bc8d0`.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `### Lexer flags and special-case rules`, lines 85-92; `## Failure modes`, lines 361-368 | The page says `kLexerFlag_SuppressDiagnostics` makes `Lexer::getDiagnosticSink` suppress every scan-time diagnostic. Malformed UTF-8 bypasses that accessor and diagnoses through `lexer->m_sink`, so it is still reported in an inactive conditional branch. | `source/compiler-core/slang-lexer.cpp:375-383` calls `diagnose(lexer->m_sink, ..., LexerDiagnostics::invalidUtf8ByteSequence, ...)`; the conditional suppression accessor is only `source/compiler-core/slang-lexer.h:145-150`. | State that the flag suppresses scan-time sites that use `getDiagnosticSink`, but malformed UTF-8 currently bypasses it via `m_sink`; remove the claims that suppression is universal. |
| F-002 | minor | `### Literal scanning and value extraction`, lines 121-135; `## Failure modes`, lines 339-345 | The page calls the scan/decode split “close to absolute” and says scan-time errors are limited to invalid characters, malformed UTF-8, and unterminated literals. Scanning also diagnoses invalid digits for a numeric base, legacy octal literals, and an invalid raw-string delimiter quote. | See `source/compiler-core/slang-lexer.cpp:543-554` (`invalidDigitForBase`), `source/compiler-core/slang-lexer.cpp:1791-1798` (`quoteCannotBeDelimiter`), and `source/compiler-core/slang-lexer.cpp:2095-2099` (`octalLiteral`). | Soften the “close to absolute” and “limited to” wording, then list these scan-time syntax checks before explaining which value semantics remain deferred. |
| F-003 | minor | `### What the lexer does not do`, lines 203-205 | The page says downstream passes “(preprocessor, parser) can decide whether to filter” whitespace and comments. Although `Lexer::lexToken` emits them, preprocessing always removes them from its output, so the parser never receives them to make that decision. | `source/compiler-core/slang-lexer.cpp:2473-2590` emits individual token kinds; `source/slang/slang-preprocessor.cpp:4895-4920` drops whitespace, newlines, comments, and invalid tokens before returning the flat `TokenList`. | Say that `lexToken` emits whitespace/comments, while `lexAllSemanticTokens` and the preprocessor filter them; remove the parser as a filtering consumer. |
| F-004 | minor | `### Stack of input streams`, lines 231-236 | The claimed per-stream lexer “`#include` mode” does not exist. Include files have their own `InputFile` and lexer, but angle-form include syntax is parsed by the preprocessor from ordinary `OpLess`/`OpGreater` tokens. | `source/slang/slang-preprocessor.cpp:944-1008` defines per-file lexer and conditional state; `source/slang/slang-preprocessor.cpp:3613-3625` concatenates raw tokens and selects `IncludeSystem::Mode::System`. | Replace “`#include` mode” with the actual per-input-file conditional/diagnostic state, and leave angle-include handling to the later `#include` subsection. |
| F-005 | minor | `### Macro expansion`, lines 251-255 | The inactive-branch list says only `#if`, `#else`, `#elif`, and `#endif` are evaluated, omitting `#ifdef` and `#ifndef`, which carry the same `ProcessWhenSkipping` flag. | `source/slang/slang-preprocessor.cpp:4624-4630` marks `if`, `ifdef`, `ifndef`, `else`, `elif`, and `endif` with `ProcessWhenSkipping`. | Add `#ifdef` and `#ifndef` to the parenthetical list. |
| F-006 | minor | `### Preprocessor directives`, lines 259-265 | Calling the supported directives “the standard C / HLSL set” is inaccurate: the table also contains Slang-specific `#language`/`#lang` and GLSL `#version`/`#extension` entries. | `source/slang/slang-preprocessor.cpp:4624-4645` contains the complete static directive table, including explicitly labeled GLSL entries and the language directives. | Describe the table as C/HLSL-style directives plus Slang language-selection and GLSL directives, and replace “verify by reading” with the concrete categories. |
| F-007 | major | `## Source-location preservation`, lines 303-335 | The page says `SourceManager` records raw macro-body tokens as part of a macro invocation and distinguishes general macro “expansion” locations from “spelling” locations so diagnostics can walk back to the call site. Raw-span expansion merely replays stored definition tokens, and no macro-invocation `SourceView` is created. Diagnostic chain walking is implemented only for `PathInfo::Type::TokenPaste` views. | `source/slang/slang-preprocessor.cpp:2440-2458` replays raw definition tokens unchanged; `source/slang/slang-preprocessor.cpp:1473-1486` stores invocation locations only on `MacroInvocation`; `source/compiler-core/slang-diagnostic-sink.cpp:456-493` follows initiating views only while their path type is `TokenPaste`. | Remove the general expansion/spelling-chain claim. State that raw body tokens keep definition locations, direct argument tokens keep call-site locations, builtins use the invocation location, and only pasted tokens create an initiating-location chain that diagnostics explicitly follow. |
| F-008 | minor | `## Failure modes`, lines 369-372 | The claim that an unrecoverable preprocessor error “produces an `EndOfFile` token early” is unsupported. The implemented handlers generally diagnose and return or skip the rest of a directive, while `ReadAllTokens` emits EOF after the input-file stack is exhausted. | `source/slang/slang-preprocessor.cpp:4672-4723` handles directive errors and continues; `source/slang/slang-preprocessor.cpp:4785-4838` pops exhausted inputs; `source/slang/slang-preprocessor.cpp:4895-4913` appends the resulting EOF. | Replace the early-EOF claim with concrete recovery behavior: malformed directives skip/finish their line, include failures return from the handler, bad macro invocations may expand to no tokens, and unclosed conditionals are diagnosed when a file reaches EOF. |

## No-issues notes

- The token layout accurately matches `TokenType : uint8_t`, `TokenFlags`, the content union, and 32-bit `SourceLoc`.
- The four explicit line-number citations are exact at the recorded commit.
- The macro-op replay, angle-include resolution, and persisted warning-location counter descriptions match the source.
