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
| C-01     | [#lexer-flags-and-special-case-rules](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#lexer-flags-and-special-case-rules) | Backslash before newline is consumed by the lexer; a token may span physical lines.                               | `backslash-line-continuation-in-macro.slang`                   |
| C-02     | [#lexer-flags-and-special-case-rules](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#lexer-flags-and-special-case-rules) | After a `#include` directive, `<foo/bar.h>` lexes as a single string-like token instead of a comparison expression. | `include-angle-bracket-mode.slang`                             |
| C-03     | [#macro-expansion](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#macro-expansion)                          | Object-like `#define` stores a lexed token sequence that is replayed on use.                                      | `object-macro-expansion.slang`                                 |
| C-04     | [#macro-expansion](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#macro-expansion)                          | Function-style macros bind parameter names to argument tokens and substitute on expansion.                        | `function-macro-argument-substitution.slang`                   |
| C-05     | [#macro-expansion](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#macro-expansion)                          | Inactive `#if` branches are not expanded; their tokens are discarded.                                             | `inactive-if-branch-not-expanded.slang`                        |
| C-06     | [#macro-expansion](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#macro-expansion)                          | Inside an inactive block, only directives that toggle the active/inactive state are evaluated.                    | `inactive-block-skips-non-conditional-directives.slang`        |
| C-07     | [#preprocessor-directives](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#preprocessor-directives)          | The standard C/HLSL directive set includes `#ifdef` to test whether a name is defined.                            | `ifdef-selects-branch.slang`                                   |
| C-08     | [#preprocessor-directives](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#preprocessor-directives)          | The directive set includes `#undef`, which removes a macro definition.                                            | `undef-makes-ifdef-false.slang`                                |
| C-09     | [#preprocessor-directives](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#preprocessor-directives)          | The directive set includes `#elif` for multi-arm conditional selection.                                           | `elif-chain-selects-first-true.slang`                          |
| C-10     | [#preprocessor-directives](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#preprocessor-directives)          | The directive set includes `#error`, which surfaces the message text as a preprocessor diagnostic.                | `error-directive-emits-diagnostic.slang`                       |
| C-11     | [#include-resolution](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#include-resolution)                    | `#include` resolves the target through `IncludeSystem` and pushes a fresh input stream for the file.              | `include-pushes-fresh-stream.slang`                            |
| C-12     | [#source-location-preservation](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#source-location-preservation) | `__LINE__` is a constructed token synthesized at the invocation site (line of use).                              | `line-macro-is-constructed-token.slang`                        |
| C-13     | [#source-location-preservation](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#source-location-preservation) | `__FILE__` is a constructed token synthesized at the invocation site (file path string).                          | `file-macro-is-constructed-token.slang`                        |
| C-14     | [#source-location-preservation](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#source-location-preservation) | The stringize operator `#x` synthesizes a fresh string-literal token from the spelling of its argument.           | `stringize-operator.slang`                                     |
| C-15     | [#source-location-preservation](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#source-location-preservation) | The token-paste operator `x##y` synthesizes a fresh single token from the concatenation of its operands.          | `token-paste-operator.slang`                                   |
| C-16     | [#failure-modes](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#failure-modes)                              | A `#include` whose target cannot be resolved emits a preprocessor diagnostic.                                     | `missing-include-diagnostic.slang`                             |
| C-17     | [#failure-modes](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#failure-modes)                              | An unknown preprocessor directive emits a diagnostic naming the directive.                                        | `unknown-directive-diagnostic.slang`                           |
| C-18     | [#failure-modes](../../../docs/llm-generated/pipeline/01-lex-preprocess.md#failure-modes)                              | A `#if` with no matching `#endif` emits an end-of-file-in-conditional diagnostic.                                 | `unbalanced-if-diagnostic.slang`                               |

## Tests in this bundle

| File                                                          | Intent     | Doc anchor                            |
| ------------------------------------------------------------- | ---------- | ------------------------------------- |
| `backslash-line-continuation-in-macro.slang`                  | functional | `#lexer-flags-and-special-case-rules` |
| `include-angle-bracket-mode.slang`                            | functional | `#lexer-flags-and-special-case-rules` |
| `object-macro-expansion.slang`                                | functional | `#macro-expansion`                    |
| `function-macro-argument-substitution.slang`                  | functional | `#macro-expansion`                    |
| `inactive-if-branch-not-expanded.slang`                       | functional | `#macro-expansion`                    |
| `inactive-block-skips-non-conditional-directives.slang`       | functional | `#macro-expansion`                    |
| `ifdef-selects-branch.slang`                                  | functional | `#preprocessor-directives`            |
| `undef-makes-ifdef-false.slang`                               | functional | `#preprocessor-directives`            |
| `elif-chain-selects-first-true.slang`                         | functional | `#preprocessor-directives`            |
| `error-directive-emits-diagnostic.slang`                      | negative   | `#preprocessor-directives`            |
| `include-pushes-fresh-stream.slang`                           | functional | `#include-resolution`                 |
| `line-macro-is-constructed-token.slang`                       | functional | `#source-location-preservation`       |
| `file-macro-is-constructed-token.slang`                       | functional | `#source-location-preservation`       |
| `stringize-operator.slang`                                    | functional | `#source-location-preservation`       |
| `token-paste-operator.slang`                                  | functional | `#source-location-preservation`       |
| `missing-include-diagnostic.slang`                            | negative   | `#failure-modes`                      |
| `unknown-directive-diagnostic.slang`                          | negative   | `#failure-modes`                      |
| `unbalanced-if-diagnostic.slang`                              | negative   | `#failure-modes`                      |

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

## Out of scope (no-GPU runner)

None for this bundle. All lex- and preprocess-stage claims are
fully observable through `slangi` or `-target cpp` text emit.
