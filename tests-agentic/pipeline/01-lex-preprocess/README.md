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

## Claims enumerated

| Claim ID | Anchor                                                                                                                  | Claim (one line)                                                                                                  | Tests                                                          |
| -------- | ----------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------- |
| C-01     | [#lexer-flags-and-special-case-rules](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#lexer-flags-and-special-case-rules) | Backslash before newline is consumed by the lexer; a token may span physical lines.                               | [`backslash-line-continuation-in-macro.slang`](backslash-line-continuation-in-macro.slang)                   |
| C-02     | [#lexer-flags-and-special-case-rules](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#lexer-flags-and-special-case-rules) | After a `#include` directive, `<foo/bar.h>` lexes as a single string-like token instead of a comparison expression. | [`include-angle-bracket-mode.slang`](include-angle-bracket-mode.slang)                             |
| C-03     | [#macro-expansion](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#macro-expansion)                          | Object-like `#define` stores a lexed token sequence that is replayed on use.                                      | [`object-macro-expansion.slang`](object-macro-expansion.slang)                                 |
| C-04     | [#macro-expansion](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#macro-expansion)                          | Function-style macros bind parameter names to argument tokens and substitute on expansion.                        | [`function-macro-argument-substitution.slang`](function-macro-argument-substitution.slang)                   |
| C-05     | [#macro-expansion](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#macro-expansion)                          | Inactive `#if` branches are not expanded; their tokens are discarded.                                             | [`inactive-if-branch-not-expanded.slang`](inactive-if-branch-not-expanded.slang)                        |
| C-06     | [#macro-expansion](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#macro-expansion)                          | Inside an inactive block, only directives that toggle the active/inactive state are evaluated.                    | [`inactive-block-skips-non-conditional-directives.slang`](inactive-block-skips-non-conditional-directives.slang)        |
| C-07     | [#preprocessor-directives](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#preprocessor-directives)          | The standard C/HLSL directive set includes `#ifdef` to test whether a name is defined.                            | [`ifdef-selects-branch.slang`](ifdef-selects-branch.slang)                                   |
| C-08     | [#preprocessor-directives](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#preprocessor-directives)          | The directive set includes `#undef`, which removes a macro definition.                                            | [`undef-makes-ifdef-false.slang`](undef-makes-ifdef-false.slang)                                |
| C-09     | [#preprocessor-directives](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#preprocessor-directives)          | The directive set includes `#elif` for multi-arm conditional selection.                                           | [`elif-chain-selects-first-true.slang`](elif-chain-selects-first-true.slang)                          |
| C-10     | [#preprocessor-directives](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#preprocessor-directives)          | The directive set includes `#error`, which surfaces the message text as a preprocessor diagnostic.                | [`error-directive-emits-diagnostic.slang`](error-directive-emits-diagnostic.slang)                       |
| C-11     | [#include-resolution](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#include-resolution)                    | `#include` resolves the target through `IncludeSystem` and pushes a fresh input stream for the file.              | [`include-pushes-fresh-stream.slang`](include-pushes-fresh-stream.slang)                            |
| C-12     | [#source-location-preservation](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#source-location-preservation) | `__LINE__` is a constructed token synthesized at the invocation site (line of use).                              | [`line-macro-is-constructed-token.slang`](line-macro-is-constructed-token.slang)                        |
| C-13     | [#source-location-preservation](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#source-location-preservation) | `__FILE__` is a constructed token synthesized at the invocation site (file path string).                          | [`file-macro-is-constructed-token.slang`](file-macro-is-constructed-token.slang)                        |
| C-14     | [#source-location-preservation](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#source-location-preservation) | The stringize operator `#x` synthesizes a fresh string-literal token from the spelling of its argument.           | [`stringize-operator.slang`](stringize-operator.slang)                                     |
| C-15     | [#source-location-preservation](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#source-location-preservation) | The token-paste operator `x##y` synthesizes a fresh single token from the concatenation of its operands.          | [`token-paste-operator.slang`](token-paste-operator.slang)                                   |
| C-16     | [#failure-modes](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#failure-modes)                              | A `#include` whose target cannot be resolved emits a preprocessor diagnostic.                                     | [`missing-include-diagnostic.slang`](missing-include-diagnostic.slang)                             |
| C-17     | [#failure-modes](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#failure-modes)                              | An unknown preprocessor directive emits a diagnostic naming the directive.                                        | [`unknown-directive-diagnostic.slang`](unknown-directive-diagnostic.slang)                           |
| C-18     | [#failure-modes](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#failure-modes)                              | A `#if` with no matching `#endif` emits an end-of-file-in-conditional diagnostic.                                 | [`unbalanced-if-diagnostic.slang`](unbalanced-if-diagnostic.slang)                               |

## Tests in this bundle

| File                                                          | Intent     | Doc anchor                            |
| ------------------------------------------------------------- | ---------- | ------------------------------------- |
| [`backslash-line-continuation-in-macro.slang`](backslash-line-continuation-in-macro.slang)                  | functional | `#lexer-flags-and-special-case-rules` |
| [`include-angle-bracket-mode.slang`](include-angle-bracket-mode.slang)                            | functional | `#lexer-flags-and-special-case-rules` |
| [`object-macro-expansion.slang`](object-macro-expansion.slang)                                | functional | `#macro-expansion`                    |
| [`function-macro-argument-substitution.slang`](function-macro-argument-substitution.slang)                  | functional | `#macro-expansion`                    |
| [`inactive-if-branch-not-expanded.slang`](inactive-if-branch-not-expanded.slang)                       | functional | `#macro-expansion`                    |
| [`inactive-block-skips-non-conditional-directives.slang`](inactive-block-skips-non-conditional-directives.slang)       | functional | `#macro-expansion`                    |
| [`ifdef-selects-branch.slang`](ifdef-selects-branch.slang)                                  | functional | `#preprocessor-directives`            |
| [`undef-makes-ifdef-false.slang`](undef-makes-ifdef-false.slang)                               | functional | `#preprocessor-directives`            |
| [`elif-chain-selects-first-true.slang`](elif-chain-selects-first-true.slang)                         | functional | `#preprocessor-directives`            |
| [`error-directive-emits-diagnostic.slang`](error-directive-emits-diagnostic.slang)                      | negative   | `#preprocessor-directives`            |
| [`include-pushes-fresh-stream.slang`](include-pushes-fresh-stream.slang)                           | functional | `#include-resolution`                 |
| [`line-macro-is-constructed-token.slang`](line-macro-is-constructed-token.slang)                       | functional | `#source-location-preservation`       |
| [`file-macro-is-constructed-token.slang`](file-macro-is-constructed-token.slang)                       | functional | `#source-location-preservation`       |
| [`stringize-operator.slang`](stringize-operator.slang)                                    | functional | `#source-location-preservation`       |
| [`token-paste-operator.slang`](token-paste-operator.slang)                                  | functional | `#source-location-preservation`       |
| [`missing-include-diagnostic.slang`](missing-include-diagnostic.slang)                            | negative   | `#failure-modes`                      |
| [`unknown-directive-diagnostic.slang`](unknown-directive-diagnostic.slang)                          | negative   | `#failure-modes`                      |
| [`unbalanced-if-diagnostic.slang`](unbalanced-if-diagnostic.slang)                              | negative   | `#failure-modes`                      |
| [`if-expression-zero-is-false.slang`](if-expression-zero-is-false.slang)                           | boundary   | `#preprocessor-directives`            |
| [`if-expression-uint32-max-is-true.slang`](if-expression-uint32-max-is-true.slang)                      | boundary   | `#preprocessor-directives`            |
| [`if-expression-int32-max-is-true.slang`](if-expression-int32-max-is-true.slang)                       | boundary   | `#preprocessor-directives`            |
| [`if-expression-signed-overflow-wraps.slang`](if-expression-signed-overflow-wraps.slang)                   | boundary   | `#preprocessor-directives`            |
| [`if-undefined-macro-is-zero.slang`](if-undefined-macro-is-zero.slang)                            | boundary   | `#preprocessor-directives`            |
| [`if-nested-five-deep.slang`](if-nested-five-deep.slang)                                   | stress     | `#preprocessor-directives`            |
| [`elif-chain-five-arms.slang`](elif-chain-five-arms.slang)                                  | stress     | `#preprocessor-directives`            |
| [`function-macro-zero-args.slang`](function-macro-zero-args.slang)                              | boundary   | `#macro-expansion`                    |
| [`function-macro-eight-args.slang`](function-macro-eight-args.slang)                             | stress     | `#macro-expansion`                    |
| [`function-macro-empty-argument.slang`](function-macro-empty-argument.slang)                         | boundary   | `#macro-expansion`                    |
| [`stringize-empty-argument.slang`](stringize-empty-argument.slang)                              | boundary   | `#source-location-preservation`       |
| [`token-paste-with-empty-rhs.slang`](token-paste-with-empty-rhs.slang)                            | boundary   | `#source-location-preservation`       |
| [`line-macro-after-line-directive.slang`](line-macro-after-line-directive.slang)                       | boundary   | `#source-location-preservation`       |
| [`include-nested-5deep.slang`](include-nested-5deep.slang)                                  | stress     | `#include-resolution`                 |
| [`include-self-cycle-diagnostic.slang`](include-self-cycle-diagnostic.slang)                         | negative   | `#failure-modes`                      |
| [`header-guard-pattern.slang`](header-guard-pattern.slang)                                  | boundary   | `#preprocessor-directives`            |
| [`backslash-line-continuation-in-string.slang`](backslash-line-continuation-in-string.slang)                 | boundary   | `#lexer-flags-and-special-case-rules` |
| [`error-directive-empty-message.slang`](error-directive-empty-message.slang)                         | boundary   | `#preprocessor-directives`            |
| [`error-directive-long-message.slang`](error-directive-long-message.slang)                          | boundary   | `#preprocessor-directives`            |
| [`error-directive-macro-name-in-message.slang`](error-directive-macro-name-in-message.slang)                 | boundary   | `#preprocessor-directives`            |
| [`warning-directive-empty-message.slang`](warning-directive-empty-message.slang)                       | boundary   | `#preprocessor-directives`            |
| [`pragma-unknown-emits-warning.slang`](pragma-unknown-emits-warning.slang)                          | boundary   | `#preprocessor-directives`            |
| [`pragma-unknown-with-args.slang`](pragma-unknown-with-args.slang)                              | boundary   | `#preprocessor-directives`            |
| [`pragma-once-prevents-redefinition.slang`](pragma-once-prevents-redefinition.slang)                     | boundary   | `#preprocessor-directives`            |
| [`macro-wrong-argument-count-diagnostic.slang`](macro-wrong-argument-count-diagnostic.slang)                 | negative   | `#macro-expansion`                    |

## Doc gaps observed

- The `## Source-location preservation` section distinguishes three
  categories of expansion-time tokens — raw body, argument, and
  constructed — but does not give a directly observable, no-GPU
  way to verify the **raw-body** vs **argument** distinction
  (e.g. that an argument token's spelling location remains the
  call-site rather than the macro body). The distinction is
  observed through diagnostics whose exact wording is not
  promised by the doc. Tests deferred until either the
  diagnostics doc commits to a specific message or the source
  doc adds an observable claim.
- `## Lexer flags and special-case rules` mentions
  `kLexerFlag_SuppressDiagnostics` as state used inside inactive
  `#if` blocks, but there is no user-surface claim about it
  beyond "tokens are still emitted as `TokenType::Invalid`".
  That is internal lexer state with no observable consequence at
  the slangc/slangi level. Recorded as undocumented-by-design.
- `## Inputs and outputs` states that "phase 1 runs to completion
  and produces a flat token list" — this is an architectural
  invariant rather than a behavioral claim. There is no slangc
  observable that distinguishes "streamed" from "batched" tokens,
  so no test is written. Could be moved to an architecture-only
  section in a future doc revision.
- `## Preprocessor / Stack of input streams` is described as a
  data-structure choice with no observable surface claim. Same
  rationale as above.
- The `## Preprocessor directives` section says "the standard C /
  HLSL set" and points readers at the source file to enumerate
  the list, rather than enumerating it in the doc itself. Tests
  here cover a representative sample (`#if`/`#ifdef`/`#elif`/
  `#else`/`#endif`/`#define`/`#undef`/`#error`/`#include`); an
  authoritative enumeration in the doc would let this bundle
  grow exhaustively. Suggestion: add an explicit directive table
  to the doc.
- The lexer claim "no distinction between identifiers and
  keywords; the parser resolves keyword status via lookup" is
  technically a parser-stage observable (parsing succeeds where
  the user might expect a lexer error). The doc itself hands off
  to `02-parse-ast.md` for the lookup story, so no test is
  anchored here.
- The `## Failure modes` section enumerates only "unbalanced
  `#if`, unknown directive, missing include" but the compiler
  also emits a dedicated **cyclic-include** diagnostic
  (`cyclic-include`, code 15302) — observed by
  `include-self-cycle-diagnostic.slang`. Suggestion: add the
  cyclic-include failure mode to the failure-modes list.
- The doc says the `#if` arithmetic evaluator handles "standard
  C/HLSL" expressions but does not commit to **integer overflow
  semantics** inside `#if`. The boundary tests
  (`if-expression-signed-overflow-wraps.slang`,
  `if-expression-uint32-max-is-true.slang`) probe the observed
  behavior (signed wrap, `0xFFFFFFFF` treated as non-zero) but
  the doc could promise these explicitly.
- The doc treats `#error`, `#warning`, `#pragma`, and `#line` as
  members of "the standard C/HLSL set" without further detail.
  In particular the behavior that `#error` **does not expand
  macros in its message** (verified by
  `error-directive-macro-name-in-message.slang`) is not
  documented; nor is the unknown-pragma policy ("warning, ignore,
  continue" — verified by `pragma-unknown-emits-warning.slang`).
  Suggestion: add a short sub-section enumerating each directive
  and its message-expansion / failure-mode contract.
- Recursive macro expansion ("a macro that references its own
  name in its body") is not explicitly addressed in the doc.
  The doc describes "fresh environment that maps parameter
  names to pseudo-macros" but does not commit to a
  recursion-detection rule. Boundary test deferred until the
  doc commits a claim.
- The doc's `## Source-location preservation` lists `__LINE__`,
  `__FILE__`, `#x`, `x##y` as "constructed tokens" but does not
  discuss the `#line` directive's interaction with `__LINE__`
  (verified by `line-macro-after-line-directive.slang`).
  Suggestion: add a sentence noting that `#line N` shifts the
  logical counter used by constructed `__LINE__` tokens.

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

## Out of scope (no-GPU runner)

None for this bundle. All lex- and preprocess-stage claims are
fully observable through `slangi` or `-target cpp` text emit.
