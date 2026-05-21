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
| `int-literal-zero.slang`                 | boundary | `#content-tokens`                 |
| `int-literal-int32-max.slang`            | boundary | `#content-tokens`                 |
| `int-literal-hex-base.slang`             | boundary | `#content-tokens`                 |
| `int-literal-octal-base.slang`           | boundary | `#content-tokens`                 |
| `int-literal-binary-base.slang`          | boundary | `#content-tokens`                 |
| `int-literal-hex-promotes-to-64bit.slang`| boundary | `#content-tokens`                 |
| `numeric-suffix-ll-is-int64.slang`       | boundary | `#content-tokens`                 |
| `numeric-suffix-ull-is-uint64.slang`     | boundary | `#content-tokens`                 |
| `numeric-suffix-h-is-half.slang`         | boundary | `#content-tokens`                 |
| `numeric-suffix-lf-is-double.slang`      | boundary | `#content-tokens`                 |
| `float-literal-hex-form.slang`           | boundary | `#content-tokens`                 |
| `float-literal-leading-dot.slang`        | boundary | `#content-tokens`                 |
| `float-literal-exponent-form.slang`      | boundary | `#content-tokens`                 |
| `string-literal-empty.slang`             | boundary | `#content-tokens`                 |
| `string-literal-escape-sequences.slang`  | boundary | `#content-tokens`                 |
| `raw-string-empty-delimiter.slang`       | boundary | `#special-case-lexing-rules`      |
| `raw-string-multi-char-delimiter.slang`  | boundary | `#special-case-lexing-rules`      |
| `char-literal-null.slang`                | boundary | `#content-tokens`                 |
| `char-literal-escape-backslash.slang`    | boundary | `#content-tokens`                 |
| `block-comment-empty.slang`              | boundary | `#trivia-whitespace-and-comments` |
| `line-comment-backslash-extends.slang`   | boundary | `#special-case-lexing-rules`      |
| `block-comment-multi-fake-nest.slang`    | stress   | `#special-case-lexing-rules`      |
| `negative-integer-literal-too-large.slang` | negative | `#content-tokens`               |
| `negative-invalid-int-suffix.slang`      | negative | `#content-tokens`                 |
| `negative-multi-char-literal.slang`      | negative | `#content-tokens`                 |

## Boundary / negative / stress probes (expansion pass)

This pass appends 25 additional tests that probe documented claims at
their edges. The intents map as follows:

- `boundary` (21): one boundary value per file along the lexer's
  numeric, string, char, comment, and line-continuation axes.
- `negative` (3): one `DIAGNOSTIC_TEST` per documented or implied
  rejection — integer literal too large, invalid integer suffix,
  multi-character character literal.
- `stress` (1): a pattern-bloat probe (`/* /* /* /* still one block
  comment */`) that verifies non-nesting survives repetition.

Every appended file uses the same `doc_ref` (and `doc_section_digest`)
as the smoke test for the claim it probes; the boundaries are
additional anchors of those same claims, not new ones.

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
- Integer-literal bases (decimal / hex `0x` / octal `0` / binary
  `0b`) are exercised by the lexer but the doc does not enumerate
  the accepted prefixes. Boundary tests for each base now exist;
  the doc would benefit from a one-line note that all four bases
  are accepted.
- Floating-point literal forms (trailing dot `1.`, leading dot
  `.5`, decimal exponent `1e3`, hexadecimal float `0x1.0p4`) are
  accepted by the lexer but only the generic "FloatingPointLiteral"
  row in `#content-tokens` is documented. A short grammar note in
  `#content-tokens` listing the four accepted shapes would close
  this gap.
- The interaction of `\<newline>` continuation with line comments
  (the comment swallows the next physical line because continuations
  are folded before comment recognition) is implied by the
  "consumed and folded out" wording in `#special-case-lexing-rules`
  but not stated explicitly. A worked example in the doc would
  prevent a confusing-looking but correct behavior from surprising
  readers.
- Documented diagnostics (`integer literal is too large to be
  represented in any integer type`, `invalid suffix on integer
  literal`, `illegal character literal`) have no explicit error
  codes / messages listed in `tokens.md`. The negative tests in
  this pass copy the diagnostic text verbatim from the compiler;
  promoting them to a "Diagnostics" subsection in the doc would
  let future authors anchor by claim ID rather than by free text.

## Out of scope (no-GPU runner)

None for this bundle. Lexer-level behaviors are fully observable
through `slangi`.
