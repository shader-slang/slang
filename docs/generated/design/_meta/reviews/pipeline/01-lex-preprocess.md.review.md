---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-30T13:43:37+00:00
target_doc: pipeline/01-lex-preprocess.md
target_doc_source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_watched_paths_digest: e9924a6d79a718b4f741ca99add14031255b67e8c578cc6bf6b39f89c956daaf
source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: pass
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 4
severity_breakdown:
  critical: 0
  major: 1
  minor: 3
  nit: 0
---

# Review report for pipeline/01-lex-preprocess.md

## Summary

The page satisfies the required shape and its links/front matter are valid, but several detailed behavior claims drift from the watched lexer and preprocessor sources. The most important issue is the literal-diagnostics description: it says escape errors are decode-time only, while `_lexStringLiteralBody` also diagnoses invalid escape forms during scanning.

## Items checked

- Verified the required front matter keys and copied `source_commit` / `watched_paths_digest` from the target document.
- Checked the required structure from `pipeline-01-lex-preprocess.md`: title, inputs/outputs, lexer, preprocessor, source-location preservation, and failure modes.
- Spot-checked 18 concrete claims against watched inputs: `TokenType`, `TokenFlags`, `Token`, `TokenList`, `TokenSpan`, and `TokenReader` in `source/compiler-core/slang-token.h` / `source/compiler-core/slang-lexer.h`; `SourceLoc::RawValue` and `SourceView` in `source/compiler-core/slang-source-loc.h`; lexer flags, line continuations, identifiers, whitespace/comment token emission, literal scanning, literal decoding, and invalid-token diagnostics in `source/compiler-core/slang-lexer.cpp`; `PreprocessorDesc`, handlers, `preprocessSource`, input streams, conditionals, macro invocation ops, directive lookup, include handling, warning-state tracking, and EOF/error paths in `source/slang/slang-preprocessor.cpp`.
- Resolved relative links to watched source files, `../syntax-reference/tokens.md`, `02-parse-ast.md`, `../cross-cutting/diagnostics.md`, `docs/language-reference/expressions-literal.md`, and `source/compiler-core/slang-lexer-diagnostic-defs.h`.
- Verified that the page defers token-kind enumeration rather than duplicating the catalog.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | minor | `### Lexer flags and special-case rules`, lines 81-87 | The page says the lexer has `Special handling of <...> after a #include directive, so that <foo/bar.h> lexes as a single string-like token`. The source shows angle-form includes are assembled in the preprocessor from ordinary raw tokens between `OpLess` and `OpGreater`; they are not lexed as one string-like token by `Lexer`. | `source/slang/slang-preprocessor.cpp:3606` reads `TokenType::OpLess`, loops with `AdvanceRawToken` until `TokenType::OpGreater`, and sets `includeMode = IncludeSystem::Mode::System` at `source/slang/slang-preprocessor.cpp:3618`. | Move this bullet out of "lexer handles" wording and say `HandleIncludeDirective` in `slang-preprocessor.cpp` concatenates raw tokens for angle-form includes before calling `IncludeSystem`. |
| F-002 | major | `### Literal scanning and value extraction`, lines 100-124; `## Failure modes`, lines 262-272 | The page overstates the split between scanning and decoding by saying string/char escape and encoding errors are reported at decode time, not scan time. `_lexStringLiteralBody` diagnoses empty or multi-character character literals, EOF/newline in literals, and malformed `\x`, `\u`, `\U`, or braced Unicode escapes during scanning; decode-time helpers also diagnose some value-conversion errors later. | `source/compiler-core/slang-lexer.cpp:1351` implements `_lexStringLiteralBody`; scan-time diagnostics are emitted at `source/compiler-core/slang-lexer.cpp:1361`, `source/compiler-core/slang-lexer.cpp:1383`, `source/compiler-core/slang-lexer.cpp:1492`, and `source/compiler-core/slang-lexer.cpp:1516`. Decode-time diagnostics remain in `getStringLiteralTokenValue` at `source/compiler-core/slang-lexer.cpp:1657` and `source/compiler-core/slang-lexer.cpp:1675`. | Revise the literal section to say scanning records raw text but still validates enough escape structure to report malformed literal syntax; reserve decode-time wording for value conversion, out-of-range code units/code points, and helper-specific escape decoding. |
| F-003 | minor | `### Macro expansion`, lines 181-185 | The page says function-style macros allocate `a fresh environment that maps parameter names to pseudo-macros for the arguments`. The implementation does not allocate a per-invocation macro environment for parameters; it precomputes `MacroDefinition::Op` entries (`ExpandedParam`, `UnexpandedParam`, `StringizedParam`) and `MacroInvocation` replays argument token ranges by parameter index. | `source/slang/slang-preprocessor.cpp:595` describes macro body ops; `_parseMacroOps` maps parameter names to op indices at `source/slang/slang-preprocessor.cpp:3729`; `MacroInvocation::_getArgTokens` returns token ranges at `source/slang/slang-preprocessor.cpp:2344`; `_initCurrentOpStream` replays or expands those ranges at `source/slang/slang-preprocessor.cpp:2455` and `source/slang/slang-preprocessor.cpp:2480`. | Replace the environment/pseudo-macro sentence with the op-based model: definitions store parameter references as macro ops, and invocations replay the matching argument token ranges, optionally wrapping them in an expansion stream. |
| F-004 | minor | `## Source-location preservation`, lines 231-249 | The source-location taxonomy overgeneralizes constructed-token locations. `__LINE__` and `__FILE__` tokens use `m_macroInvocationLoc`, but stringized parameter tokens use the location of the `#` token in the macro definition, and token-paste results get a new `SourceView` initiated from the `##` token location. | Builtins call `_pushStreamForSourceLocBuiltin`, which passes `m_macroInvocationLoc` to `_pushSingleTokenStream` at `source/slang/slang-preprocessor.cpp:2311`. Stringized parameters pass `loc = m_macro->tokens.m_tokens[tokenIndex].loc` at `source/slang/slang-preprocessor.cpp:2509` and `_pushSingleTokenStream(..., loc, ...)` at `source/slang/slang-preprocessor.cpp:2548`. Token paste creates a `PathInfo::makeTokenPaste()` source view initiated by `tokenPasteLoc` at `source/slang/slang-preprocessor.cpp:2188`. | Split "constructed tokens" into builtins, stringized arguments, and token paste, and state the exact source-location rule for each case instead of assigning all of them to `m_macroInvocationLoc`. |

## No-issues notes

- The token data-model section matches `TokenType : uint8_t`, `TokenFlags`, `charsCount`, `charsNameUnion`, and `SourceLoc::RawValue`.
- The warning-state section accurately describes the persisted absolute-location counter and release assert.
- The preprocessor directive table and include-resolution links resolve to the relevant implementation files.
