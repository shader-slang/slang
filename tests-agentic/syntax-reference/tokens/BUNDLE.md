---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-20T15:00:00+00:00
source_commit: 1099f9d83f3d63135650f759de64b32a4eee48aa
watched_paths_digest: 09f4cba6ed86dd22ab9eb213b2fb1a4aa730fccbd2dd249fb56c3b7502622005
source_doc: docs/llm-generated/syntax-reference/tokens.md
source_doc_digest: d75869f84bce1dbc6ad2449ddfdb74dada8405ec359b9dae628844b6d3389b6e
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for syntax-reference/tokens

## Intent

Tests verify the lexer-surface behaviors described in
[`docs/llm-generated/syntax-reference/tokens.md`](../../../docs/llm-generated/syntax-reference/tokens.md):
how the source character stream becomes a token stream, including
literal-typing suffixes, comment kinds, raw-string non-escape semantics,
and backslash line continuation. Each test uses `//TEST:INTERPRET` so
the lexer behavior is observed through the byte-code interpreter and
does not require a GPU.

This is the **pilot bundle** for Phase B1. It exercises the full
agentic-test contract end-to-end on a tightly-scoped doc before the
rest of the suite is bootstrapped.

## Claims enumerated

| Claim ID | Anchor                                                                                                                   | Claim (one line)                                                               | Tests                                |
| -------- | ------------------------------------------------------------------------------------------------------------------------ | ------------------------------------------------------------------------------ | ------------------------------------ |
| C-01     | [#content-tokens](../../../docs/llm-generated/syntax-reference/tokens.md#content-tokens)                                 | Integer literal suffixes (e.g. `u`) are part of the token and decide its type. | `numeric-suffix-u-is-uint.slang`     |
| C-02     | [#content-tokens](../../../docs/llm-generated/syntax-reference/tokens.md#content-tokens)                                 | Floating-point literal suffixes (e.g. `f`) are part of the token.              | `numeric-suffix-f-is-float.slang`    |
| C-03     | [#content-tokens](../../../docs/llm-generated/syntax-reference/tokens.md#content-tokens)                                 | Single-quoted character literals are tokenized as `CharLiteral`.               | `char-literal-value.slang`           |
| C-04     | [#content-tokens](../../../docs/llm-generated/syntax-reference/tokens.md#content-tokens)                                 | Double-quoted string literals are recognized and usable.                       | `string-literal-basic.slang`         |
| C-05     | [#special-case-lexing-rules](../../../docs/llm-generated/syntax-reference/tokens.md#special-case-lexing-rules)           | Raw string literals do not perform escape processing.                          | `raw-string-no-escape.slang`         |
| C-06     | [#trivia-whitespace-and-comments](../../../docs/llm-generated/syntax-reference/tokens.md#trivia-whitespace-and-comments) | `//` line comments end at the end of the physical line.                        | `line-comment-ends-at-newline.slang` |
| C-07     | [#trivia-whitespace-and-comments](../../../docs/llm-generated/syntax-reference/tokens.md#trivia-whitespace-and-comments) | `/* ... */` block comments are removed from the token stream.                  | `block-comment-removed.slang`        |
| C-08     | [#special-case-lexing-rules](../../../docs/llm-generated/syntax-reference/tokens.md#special-case-lexing-rules)           | Block comments do not nest; an outer `/* ... */` ends at the first `*/`.       | `block-comment-no-nesting.slang`     |
| C-09     | [#special-case-lexing-rules](../../../docs/llm-generated/syntax-reference/tokens.md#special-case-lexing-rules)           | A backslash immediately before a newline is consumed and folded out.           | `backslash-line-continuation.slang`  |

## Tests in this bundle

| File                                 | Intent     | Doc anchor                        |
| ------------------------------------ | ---------- | --------------------------------- |
| `numeric-suffix-u-is-uint.slang`     | functional | `#content-tokens`                 |
| `numeric-suffix-f-is-float.slang`    | functional | `#content-tokens`                 |
| `char-literal-value.slang`           | functional | `#content-tokens`                 |
| `string-literal-basic.slang`         | functional | `#content-tokens`                 |
| `raw-string-no-escape.slang`         | functional | `#special-case-lexing-rules`      |
| `line-comment-ends-at-newline.slang` | functional | `#trivia-whitespace-and-comments` |
| `block-comment-removed.slang`        | functional | `#trivia-whitespace-and-comments` |
| `block-comment-no-nesting.slang`     | functional | `#special-case-lexing-rules`      |
| `backslash-line-continuation.slang`  | functional | `#special-case-lexing-rules`      |

## Doc gaps observed

- The doc describes `RightArrow` (`->`), `DoubleRightArrow` (`=>`),
  `Ellipsis` (`...`), and `Scope` (`::`) as punctuation tokens but
  does not state user-facing claims about their semantic role beyond
  one-line notes like "lambda / requirement-binding". Tests for these
  would need to anchor to `syntax-reference/grammar.md` or
  `ast-reference/expressions.md`, neither of which has been
  generated yet. Deferred to Phase C cross-link, or to expansion once
  those bundles exist.
- The doc lists `TokenFlags::AtStartOfLine`, `AfterWhitespace`, and
  `ScrubbingNeeded` (under `#token-flags`) but these are internal
  lexer state with no surface observable consequence accessible via
  `slangc`. Recorded as undocumented-by-design rather than a doc gap.
- Numeric literal suffixes beyond `u` and `f` (`l`, `ul`, `h`, `lf`,
  ...) are mentioned but not exhaustively enumerated. Expansion pass
  candidate once a definitive list is added to the doc.

## Out of scope (no-GPU runner)

None for this bundle. Lexer-level behaviors are fully observable
through `slangi`.
