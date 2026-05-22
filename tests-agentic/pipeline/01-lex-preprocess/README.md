---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-20T15:00:00+00:00
source_commit: ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
watched_paths_digest: 9219ab7235ed9a6e491e941233bae687c32aefd89c797c71c233bfcbbb57b829
source_doc: docs/llm-generated/pipeline/01-lex-preprocess.md
source_doc_digest: 53eade2c502f9bd2f30352a014642950f055df72b7f060a240ff34569eafbeea
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for pipeline/01-lex-preprocess

## Intent
Tests verify the lex-and-preprocess pipeline behaviors described in
[`docs/llm-generated/pipeline/01-lex-preprocess.md`](../../../docs/llm-generated/pipeline/01-lex-preprocess.md):
the lexer's special-case handling (backslash continuation,
`<...>`-after-`#include`), macro expansion (object-like,
function-like, `#`, `##`, `__LINE__`, `__FILE__`), conditional
compilation (`#if`/`#ifdef`/`#elif`/`#else`/`#endif`/`#undef`),
`#include` resolution, and the failure modes called out by the
`## Failure modes` section.

Coverage strategy: at least one functional test per substantive
positive claim (object/function macros, condition selection,
inactive-branch skipping, include resolution, line continuation,
angle-bracket include) and at least one diagnostic test per
"is rejected"-style claim (`#error`, missing include, unknown
directive, unbalanced `#if`). Pure lex-surface claims that
`syntax-reference/tokens` already exercises (numeric suffixes,
comment shapes, raw strings) are intentionally NOT duplicated
here.


## Functional coverage
| Claim | Intent | Anchor | Tests |
| --- | --- | --- | --- |
| A `#if` directive with no matching `#endif` is reported as a preprocessor diagnostic. | negative | [#failure-modes](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#failure-modes) | [`unbalanced-if-diagnostic.slang`](unbalanced-if-diagnostic.slang) |
| A `#include` whose target cannot be resolved emits a preprocessor diagnostic through the DiagnosticSink. | negative | [#failure-modes](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#failure-modes) | [`missing-include-diagnostic.slang`](missing-include-diagnostic.slang) |
| An unknown preprocessor directive emits a diagnostic through the DiagnosticSink. | negative | [#failure-modes](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#failure-modes) | [`unknown-directive-diagnostic.slang`](unknown-directive-diagnostic.slang) |
| Negative probe: a `#include` of the file itself triggers the documented "cyclic include" preprocessor diagnostic. | negative | [#failure-modes](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#failure-modes) | [`include-self-cycle-diagnostic.slang`](include-self-cycle-diagnostic.slang) |
| Stress probe of nested `#include`: chain depth 5 (this file -> d1 -> d2 -> d3 -> d4 -> d5) leaves the leaf declaration visible at the root. | stress | [#include-resolution](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#include-resolution) | [`include-nested-5deep.slang`](include-nested-5deep.slang) |
| `#include` resolves the named string against `IncludeSystem` and pushes a fresh input stream so the file's tokens become part of the enclosing translation unit. | functional | [#include-resolution](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#include-resolution) | [`include-pushes-fresh-stream.slang`](include-pushes-fresh-stream.slang) |
| After a `#include` directive the lexer treats `<foo/bar.h>` as a single string-like token rather than a comparison expression. | functional | [#lexer-flags-and-special-case-rules](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#lexer-flags-and-special-case-rules) | [`include-angle-bracket-mode.slang`](include-angle-bracket-mode.slang) |
| Boundary probe of backslash line continuation: a `\<newline>` inside a string literal is folded out, so the two physical lines become one logical literal. | boundary | [#lexer-flags-and-special-case-rules](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#lexer-flags-and-special-case-rules) | [`backslash-line-continuation-in-string.slang`](backslash-line-continuation-in-string.slang) |
| The lexer handles backslash line continuations; a `\` immediately before a newline is consumed so the macro definition spans multiple physical lines as if it were one logical line. | functional | [#lexer-flags-and-special-case-rules](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#lexer-flags-and-special-case-rules) | [`backslash-line-continuation-in-macro.slang`](backslash-line-continuation-in-macro.slang) |
| Boundary probe of function-style macros: a zero-parameter macro `FOO()` is valid and expands without taking arguments. | boundary | [#macro-expansion](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#macro-expansion) | [`function-macro-zero-args.slang`](function-macro-zero-args.slang) |
| Boundary probe of function-style macros: an empty argument (the comma is present, the token list before/after is empty) substitutes to nothing. | boundary | [#macro-expansion](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#macro-expansion) | [`function-macro-empty-argument.slang`](function-macro-empty-argument.slang) |
| Function-style macros allocate a fresh environment that maps each parameter name to its argument, so the argument tokens replace the parameter at expansion. | functional | [#macro-expansion](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#macro-expansion) | [`function-macro-argument-substitution.slang`](function-macro-argument-substitution.slang) |
| Inside an inactive `#if` block, only directives that may toggle the active/inactive state (`#if`, `#else`, `#elif`, `#endif`) are evaluated; other directives are skipped without effect. | functional | [#macro-expansion](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#macro-expansion) | [`inactive-block-skips-non-conditional-directives.slang`](inactive-block-skips-non-conditional-directives.slang) |
| Negative probe of function-style macros: invoking a 2-parameter macro with only 1 argument is rejected with the documented arg-count diagnostic. | negative | [#macro-expansion](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#macro-expansion) | [`macro-wrong-argument-count-diagnostic.slang`](macro-wrong-argument-count-diagnostic.slang) |
| Object-like `#define` stores a lexed token sequence that is replayed verbatim at every use, so the expanded value is what the parser sees. | functional | [#macro-expansion](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#macro-expansion) | [`object-macro-expansion.slang`](object-macro-expansion.slang) |
| Stress probe of function-style macros: an 8-parameter macro binds all 8 names to their arguments and substitutes correctly. | stress | [#macro-expansion](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#macro-expansion) | [`function-macro-eight-args.slang`](function-macro-eight-args.slang) |
| Tokens inside an inactive `#if` branch flow through the lexer for accounting but their contents are not expanded or parsed. | functional | [#macro-expansion](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#macro-expansion) | [`inactive-if-branch-not-expanded.slang`](inactive-if-branch-not-expanded.slang) |
| Boundary probe of `#error`: a bare `#error` with no message text still fires the documented preprocessor error. | boundary | [#preprocessor-directives](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#preprocessor-directives) | [`error-directive-empty-message.slang`](error-directive-empty-message.slang) |
| Boundary probe of `#error`: a very long message body (upper edge of the message axis) is preserved verbatim in the diagnostic. | boundary | [#preprocessor-directives](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#preprocessor-directives) | [`error-directive-long-message.slang`](error-directive-long-message.slang) |
| Boundary probe of `#error`: an identifier in the message body is preserved verbatim, observable in the emitted diagnostic. | boundary | [#preprocessor-directives](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#preprocessor-directives) | [`error-directive-macro-name-in-message.slang`](error-directive-macro-name-in-message.slang) |
| Boundary probe of `#if`: `0xFFFFFFFF` (uint32 MAX) evaluates as non-zero and selects the active branch. | boundary | [#preprocessor-directives](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#preprocessor-directives) | [`if-expression-uint32-max-is-true.slang`](if-expression-uint32-max-is-true.slang) |
| Boundary probe of `#if`: `2147483647` (int32 MAX) is parsed and evaluated as non-zero. | boundary | [#preprocessor-directives](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#preprocessor-directives) | [`if-expression-int32-max-is-true.slang`](if-expression-int32-max-is-true.slang) |
| Boundary probe of `#if`: a literal `0` selects the inactive branch and `#else` activates. | boundary | [#preprocessor-directives](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#preprocessor-directives) | [`if-expression-zero-is-false.slang`](if-expression-zero-is-false.slang) |
| Boundary probe of `#if`: an undefined identifier in the expression is treated as `0` per the standard C/HLSL `#if` semantics referenced by the doc. | boundary | [#preprocessor-directives](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#preprocessor-directives) | [`if-undefined-macro-is-zero.slang`](if-undefined-macro-is-zero.slang) |
| Boundary probe of `#pragma once`: a header with `#pragma once` is only processed once even when included twice, preventing duplicate declarations. | boundary | [#preprocessor-directives](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#preprocessor-directives) | [`pragma-once-prevents-redefinition.slang`](pragma-once-prevents-redefinition.slang) |
| Boundary probe of `#pragma`: an unknown sub-directive followed by argument tokens still produces the "unknown pragma ignored" warning, and the trailing tokens are skipped to end-of-line. | boundary | [#preprocessor-directives](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#preprocessor-directives) | [`pragma-unknown-with-args.slang`](pragma-unknown-with-args.slang) |
| Boundary probe of `#pragma`: an unknown sub-directive name is ignored with a warning diagnostic, rather than rejected as a hard error. | boundary | [#preprocessor-directives](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#preprocessor-directives) | [`pragma-unknown-emits-warning.slang`](pragma-unknown-emits-warning.slang) |
| Boundary probe of `#warning`: a bare `#warning` with no message text still fires the documented preprocessor-warning diagnostic. | boundary | [#preprocessor-directives](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#preprocessor-directives) | [`warning-directive-empty-message.slang`](warning-directive-empty-message.slang) |
| Boundary probe of arithmetic in `#if`: `(INT_MAX + 1)` wraps to a negative value (observed behavior). | boundary | [#preprocessor-directives](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#preprocessor-directives) | [`if-expression-signed-overflow-wraps.slang`](if-expression-signed-overflow-wraps.slang) |
| Boundary probe: the canonical `#ifndef GUARD / #define GUARD / #endif` header-guard pattern protects a second inclusion from re-defining its contents. | boundary | [#preprocessor-directives](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#preprocessor-directives) | [`header-guard-pattern.slang`](header-guard-pattern.slang) |
| Stress probe of `#elif`: in a 5-arm `#if`/`#elif*4`/`#else` chain, only the matching arm runs and the others are inactive. | stress | [#preprocessor-directives](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#preprocessor-directives) | [`elif-chain-five-arms.slang`](elif-chain-five-arms.slang) |
| Stress probe of conditional nesting: five levels of `#if`/`#endif` track correctly and the innermost active branch wins. | stress | [#preprocessor-directives](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#preprocessor-directives) | [`if-nested-five-deep.slang`](if-nested-five-deep.slang) |
| The preprocessor supports `#elif` as part of the standard C/HLSL conditional set; in a chain it selects the first arm whose expression evaluates to non-zero. | functional | [#preprocessor-directives](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#preprocessor-directives) | [`elif-chain-selects-first-true.slang`](elif-chain-selects-first-true.slang) |
| The preprocessor supports the standard C/HLSL `#define`/`#undef` pair, and `#undef` removes a macro so that a subsequent `#ifdef` is false. | functional | [#preprocessor-directives](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#preprocessor-directives) | [`undef-makes-ifdef-false.slang`](undef-makes-ifdef-false.slang) |
| The preprocessor supports the standard C/HLSL conditional set, including `#ifdef` selecting the active branch when the named macro is defined. | functional | [#preprocessor-directives](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#preprocessor-directives) | [`ifdef-selects-branch.slang`](ifdef-selects-branch.slang) |
| The standard C/HLSL `#error` directive is supported and causes the preprocessor to emit a diagnostic with the directive's message text. | negative | [#preprocessor-directives](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#preprocessor-directives) | [`error-directive-emits-diagnostic.slang`](error-directive-emits-diagnostic.slang) |
| Boundary probe of `##`: pasting an identifier with the empty token sequence yields just that identifier. | boundary | [#source-location-preservation](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#source-location-preservation) | [`token-paste-with-empty-rhs.slang`](token-paste-with-empty-rhs.slang) |
| Boundary probe of `__LINE__`: after a `#line 1000` directive, `__LINE__` reports a large value (boundary far from physical line 1). | boundary | [#source-location-preservation](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#source-location-preservation) | [`line-macro-after-line-directive.slang`](line-macro-after-line-directive.slang) |
| Boundary probe of stringize `#x`: an empty argument stringizes to an empty string literal `""`. | boundary | [#source-location-preservation](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#source-location-preservation) | [`stringize-empty-argument.slang`](stringize-empty-argument.slang) |
| The stringize operator `#x` synthesizes a fresh string-literal token whose content is the spelling of the argument. | functional | [#source-location-preservation](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#source-location-preservation) | [`stringize-operator.slang`](stringize-operator.slang) |
| The token-paste operator `x##y` synthesizes a fresh single token formed by concatenating the spellings of its two operands. | functional | [#source-location-preservation](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#source-location-preservation) | [`token-paste-operator.slang`](token-paste-operator.slang) |
| `__FILE__` is a constructed token synthesized fresh at the use site; it expands to a string literal naming the current source file. | functional | [#source-location-preservation](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#source-location-preservation) | [`file-macro-is-constructed-token.slang`](file-macro-is-constructed-token.slang) |
| `__LINE__` is a constructed token synthesized fresh at each use; it expands to the line number of the use site. | functional | [#source-location-preservation](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#source-location-preservation) | [`line-macro-is-constructed-token.slang`](line-macro-is-constructed-token.slang) |


## Untested claims
NA


## Doc gaps observed
| Anchor | Kind | Gap | Suggested addition |
| --- | --- | --- | --- |
| [#source-location-preservation](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#source-location-preservation) | undocumented-behavior | The `## Source-location preservation` section distinguishes three categories of expansion-time tokens — raw body, argument, and constructed — but does not give a directly observable, no-GPU way to verify the **raw-body** vs **argument** distinction (e.g. that an argument token's spelling location remains the call-site rather than the macro body). The distinction is observed through diagnostics whose exact wording is not promised by the doc. Tests deferred until either the diagnostics doc commits to a specific message or the source doc adds an observable claim. |  |
| [#if](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#if) | undocumented-behavior | `## Lexer flags and special-case rules` mentions `kLexerFlag_SuppressDiagnostics` as state used inside inactive `#if` blocks, but there is no user-surface claim about it beyond "tokens are still emitted as `TokenType::Invalid`". That is internal lexer state with no observable consequence at the slangc/slangi level. Recorded as undocumented-by-design. |  |
| [#inputs-and-outputs](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#inputs-and-outputs) | undocumented-behavior | `## Inputs and outputs` states that "phase 1 runs to completion and produces a flat token list" — this is an architectural invariant rather than a behavioral claim. There is no slangc observable that distinguishes "streamed" from "batched" tokens, so no test is written. Could be moved to an architecture-only section in a future doc revision. |  |
| [#preprocessor-stack-of-input-streams](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#preprocessor-stack-of-input-streams) | undocumented-behavior | `## Preprocessor / Stack of input streams` is described as a data-structure choice with no observable surface claim. Same rationale as above. |  |
| [#if](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#if) | undocumented-behavior | The `## Preprocessor directives` section says "the standard C / HLSL set" and points readers at the source file to enumerate the list, rather than enumerating it in the doc itself. Tests here cover a representative sample (`#if`/`#ifdef`/`#elif`/ `#else`/`#endif`/`#define`/`#undef`/`#error`/`#include`); an authoritative enumeration in the doc would let this bundle grow exhaustively. Suggestion: add an explicit directive table to the doc. |  |
| [#02-parse-astmd](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#02-parse-astmd) | undocumented-behavior | The lexer claim "no distinction between identifiers and keywords; the parser resolves keyword status via lookup" is technically a parser-stage observable (parsing succeeds where the user might expect a lexer error). The doc itself hands off to `02-parse-ast.md` for the lookup story, so no test is anchored here. |  |
| [#if](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#if) | undocumented-behavior | The `## Failure modes` section enumerates only "unbalanced `#if`, unknown directive, missing include" but the compiler also emits a dedicated **cyclic-include** diagnostic (`cyclic-include`, code 15302) — observed by `include-self-cycle-diagnostic.slang`. Suggestion: add the cyclic-include failure mode to the failure-modes list. |  |
| [#if](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#if) | undocumented-behavior | The doc says the `#if` arithmetic evaluator handles "standard C/HLSL" expressions but does not commit to **integer overflow semantics** inside `#if`. The boundary tests (`if-expression-signed-overflow-wraps.slang`, `if-expression-uint32-max-is-true.slang`) probe the observed behavior (signed wrap, `0xFFFFFFFF` treated as non-zero) but the doc could promise these explicitly. |  |
| [#error](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#error) | undocumented-behavior | The doc treats `#error`, `#warning`, `#pragma`, and `#line` as members of "the standard C/HLSL set" without further detail. In particular the behavior that `#error` **does not expand macros in its message** (verified by `error-directive-macro-name-in-message.slang`) is not documented; nor is the unknown-pragma policy ("warning, ignore, continue" — verified by `pragma-unknown-emits-warning.slang`). Suggestion: add a short sub-section enumerating each directive and its message-expansion / failure-mode contract. |  |
| [#a-macro-that-references-its-own-name-in-its-body](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#a-macro-that-references-its-own-name-in-its-body) | undocumented-behavior | Recursive macro expansion ("a macro that references its own name in its body") is not explicitly addressed in the doc. The doc describes "fresh environment that maps parameter names to pseudo-macros" but does not commit to a recursion-detection rule. Boundary test deferred until the doc commits a claim. |  |
| [#x](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#x) | undocumented-behavior | The doc's `## Source-location preservation` lists `__LINE__`, `__FILE__`, `#x`, `x##y` as "constructed tokens" but does not discuss the `#line` directive's interaction with `__LINE__` (verified by `line-macro-after-line-directive.slang`). Suggestion: add a sentence noting that `#line N` shifts the logical counter used by constructed `__LINE__` tokens. |  |


## Boundary / stress coverage added in expansion
The expansion pass added boundary, stress, and negative probes
against the axes called out in `_common.md` (Pressure the
compiler):

- **`#if` expression**: 0 (lower edge), `0xFFFFFFFF` (uint32
  MAX), `2147483647` (int32 MAX), signed arithmetic overflow
  (`INT_MAX + 1` wraps), undefined identifier treated as 0,
  five-deep nesting, five-arm `#elif` chain.
- **Macro expansion**: zero-parameter macro, eight-parameter
  macro (many-args stress), empty argument substitution,
  wrong-argument-count diagnostic (negative).
- **Stringize / paste**: empty argument to `#x` yields `""`,
  `##` with empty right-hand operand collapses to the left
  operand.
- **`#include`**: five-deep chain, self-include cycle (negative
  — cyclic include diagnostic), header-guard pattern via
  `#ifndef` / `#define`.
- **Predefined macros**: `__LINE__` after `#line N` reports a
  shifted value (far-line boundary). `__FILE__` boundary is
  covered by the existing functional test.
- **Line continuation**: backslash-newline inside a string
  literal (in addition to the existing in-macro test).
- **`#error` / `#warning`**: empty-message edge, long-message
  edge, macro-name-in-message (verbatim, not expanded).
- **`#pragma`**: unknown pragma is a warning (not an error);
  unknown pragma with arguments still warns; `#pragma once` in
  a doubly-included header prevents duplicate declarations.

Skipped boundary axes:

- **Recursive macro definition** (negative) — the doc does not
  commit to detection semantics; recorded as a doc gap above.
- **Line continuation in `#include` path** — Slang does not
  document line-continuation behavior inside an include path
  specifically; no observable claim to anchor to.
