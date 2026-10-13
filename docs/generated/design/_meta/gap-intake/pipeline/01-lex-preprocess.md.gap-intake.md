---
gap_intake_report: true
intake_model: claude-opus-5[1m]
intake_at: 2026-08-11T16:29:02Z
target_doc: pipeline/01-lex-preprocess.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: ec47ea72b6aa5fefc3b36f8a780dbd3ecf5b1f6e
gap_count: 11
actions:
  fixed: 9
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 2
  escalated_to_finding: 0
---

# Gap-intake report for pipeline/01-lex-preprocess.md

## Summary

Eleven gaps from `design/pipeline/01-lex-preprocess`: nine fixed, two
deferred, none rejected and none escalated — no observation
contradicted the watched source, so nothing here is a compiler defect.
The nine fixes land in five sections: `### Literal scanning and value
extraction` (scan-check severities plus the `1e300` / `1e300lf` suffix
example), `### Macro expansion` (argument-count mismatch and the
busy-list recursion rule), `### Preprocessor directives` (three new
paragraphs: the effect of `#language` / `#lang` / `#version` /
`#extension`, the `#pragma once` and unknown-sub-directive policy, and
the `#if` arithmetic model), `## Source-location preservation`
(category 2 rewritten around `_getArgTokens` with a falsifiable
example), and `## Failure modes` (cyclic include and the stray
conditional closer). Gaps `ef72a4b5521d` and `2ed4b39212fa` are one
underlying observation about the directive tables and were fixed by a
single consolidated edit to `### Preprocessor directives`.

Both deferrals are the same shape: the fix is real and in scope but the
confirming source lies outside this page's `watched_paths`. Naming the
encodings `decodeContentBlob` accepts needs
`source/core/slang-char-encode.{h,cpp}`; naming the caller that turns
`BadSignificand` / `BadSuffix` into user diagnostics needs
`source/slang/slang-parser.cpp`. Two additions say slightly more than
the gap asked because the source does: the variadic argument-count rule
(only the non-variadic parameters are required) alongside the
non-variadic one, and the divide-by-zero check as the one operand
inspection `EvaluateInfixOp` performs. The page grew from 20,392 to
24,554 bytes against a 24,576-byte cap, so the cap is now effectively
full and should be raised before the next round of additions.

## Actions

| Gap ID       | Action   | Evidence                                                                                                                                                                                                                                                                                                                            | Fix summary                                                                                                                                                    |
| ------------ | -------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 8082592cbed7 | fixed    | `source/slang/slang-preprocessor.cpp:3710-3715` (`includedFiles.contains` → `Diagnostics::CyclicInclude`, then `return`), `:3530` (`#endif` with no open conditional → `Diagnostics::DirectiveWithoutIf`), `:4773` (one `EndOfFileInPreprocessorConditional` per still-open conditional); `include-self-cycle-diagnostic.slang`.     | added cyclic include and the stray `#else` / `#elif` / `#endif` closer to the enumerated preprocessor failure modes, with the recovery behaviour for each       |
| c51da6ca6e95 | deferred | `source/compiler-core/slang-source-loc.cpp:619-646` names only `CharEncoding::determineEncoding` / `getEncoding(type)->decode`; the encoding set lives in `source/core/slang-char-encode.cpp:118-160`, outside `watched_paths`, and whether a CLI-supplied file reaches this path needs the file-loading code, also outside. Needs a `watched_paths` expansion. | —                                                                                                                                                              |
| 49106a3707d8 | fixed    | `source/compiler-core/slang-lexer.cpp:2097` (warn, then `_lexNumber(lexer, 8)` continues), `:551` and `:1797` (the other two); severities `10002 Warning`, `10003 Error`, `10010 Error` in `source/compiler-core/slang-lexer-diagnostic-defs.h:27-49`; `octal-literal-scan-warning.slang` (W10002).                                 | stated that only `octalLiteral` is a warning and `017` still scans as a usable base-8 literal, the other two being errors                                       |
| 72ada6869b5d | fixed    | `source/compiler-core/slang-lexer.cpp:1273-1284` (empty suffix → `Float`, `l`/`lf`/`fl` → `Double`), `:1331-1333` (the `Float` arm parses into a `float`, so `1e300` is out of range); `float-double-suffix-preserves-double-range.slang`.                                                                                          | added a two-line `1e300lf` / `1e300` example showing that the suffix alone decides representability                                                            |
| b4e069d3023f | deferred | The sole caller at HEAD is `parseFloatingPointLiteralExpr` in `source/slang/slang-parser.cpp:8715-8790`, which maps `BadSignificand` → `InvalidFloatingPointLiteralNumber` and `BadSuffix` → `InvalidFloatingPointLiteralSuffix`. That file is outside this page's `watched_paths`, so the claim cannot be cited here; needs a `watched_paths` expansion or a peer-page owner. | —                                                                                                                                                              |
| aa4d7f543736 | fixed    | `source/slang/slang-preprocessor.cpp:1462-1471` (`MacroInvocation::isBusy` walks the in-flight list), `:1723-1730` (`_maybeBeginMacroInvocation` returns without expanding when busy, leaving `m_lookaheadToken` to be delivered as an identifier).                                                                                 | stated the busy-list rule: a macro naming itself is left as a plain identifier, not depth-limited or diagnosed, and the same list stops mutual recursion        |
| 4cc2b13c259b | fixed    | `source/slang/slang-preprocessor.cpp:1865-1900` — `WrongNumberOfArgumentsToMacro` at `:1870` (non-variadic) and `:1892` (variadic under-supply), each followed by `delete invocation; return;` so nothing is pushed; `macro-wrong-argument-count-diagnostic.slang` (E15501).                                                        | added the argument-count mismatch to the replay paragraph, noting the invocation contributes no tokens and that variadic macros require only fixed parameters   |
| ef72a4b5521d | fixed    | `source/slang/slang-preprocessor.cpp:4536-4605` (`#language` / `#lang` set language + version), `:4501-4527` (`#version`), `:4496-4499` (`#extension` skips its line), `:4445-4451` (`kPragmaDirectives` = `once`, `warning`), `:4272` and `:3680` (`#pragma once` identity round-trip); `source/slang/slang-preprocessor.h:65-69` (`outDetectedLanguage` / `outLanguageVersion`); `pragma-once-prevents-redefinition.slang`. | added per-directive effects for the four non-C entries and the `#pragma` sub-directive set; one consolidated edit that also covers `2ed4b39212fa`               |
| 3e1202309230 | fixed    | `source/slang/slang-preprocessor.cpp:2984` (`typedef int PreprocessorExpressionValue`), `:3117-3120` (undefined identifier warns and returns `0`), `:3194-3262` (`EvaluateInfixOp` applies the C++ operator; only `/` and `%` check anything); `if-undefined-macro-is-zero.slang`, `if-expression-uint32-max-is-true.slang`.        | added the `#if` evaluation model: signed `int` value type, undefined-identifier-is-zero, and overflow neither detected nor diagnosed                            |
| 2ed4b39212fa | fixed    | `source/slang/slang-preprocessor.cpp:4459-4470` (`findPragmaDirective` falls back to `kUnknownPragmaDirective`), `:4242-4249` (`UnknownPragmaDirectiveIgnored` then `SkipToEndOfLine`), contrasted with `:4614-4620` (`HandleInvalidDirective` errors); severity `warning` at `source/slang/slang-diagnostics.lua:763-768`.          | stated that an unknown `#pragma` sub-directive warns and is skipped, unlike the unknown-`#`-directive error; part of the same consolidated edit as `ef72a4b5521d` |
| f64c1edac915 | fixed    | `source/slang/slang-preprocessor.cpp:2351-2379` (`_getArgTokens` builds a `TokenReader` over the recorded call-site token range), `:2487-2505` (`ExpandedParam` wraps it in a `PretokenizedInputStream` with no location rewriting), against `:2440-2459` (the `RawSpan` arm replaying definition tokens).                                                | rewrote category 2 around `_getArgTokens` and added the `TWICE(x)` example whose diagnostic location would move into the body under the category-1 rule         |
