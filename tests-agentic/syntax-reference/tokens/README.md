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

## Coverage

| Claim | Intent | Anchor | Tests |
| --- | --- | --- | --- |
| A double-quoted `StringLiteral` is recognized by the lexer and its text content survives end-to-end through to emitted code. | functional | [#content-tokens](../../../docs/llm-generated/syntax-reference/tokens.md#content-tokens) | [`string-literal-basic.slang`](string-literal-basic.slang) |
| A single-quoted character literal (`CharLiteral`) is recognized by the lexer and carries the character's integer code-point value. | functional | [#content-tokens](../../../docs/llm-generated/syntax-reference/tokens.md#content-tokens) | [`char-literal-value.slang`](char-literal-value.slang) |
| Boundary - `2147483647` (int32 MAX) is a valid IntegerLiteral that fits in `int` without overflow. | boundary | [#content-tokens](../../../docs/llm-generated/syntax-reference/tokens.md#content-tokens) | [`int-literal-int32-max.slang`](int-literal-int32-max.slang) |
| Boundary - a CharLiteral whose content is an escape sequence (`'\\'`, `'\n'`) lexes as one token and decodes to the escaped code point. | boundary | [#content-tokens](../../../docs/llm-generated/syntax-reference/tokens.md#content-tokens) | [`char-literal-escape-backslash.slang`](char-literal-escape-backslash.slang) |
| Boundary - a FloatingPointLiteral with a decimal exponent (`1e3`) is a single token; the `e` and digits are part of the literal text. | boundary | [#content-tokens](../../../docs/llm-generated/syntax-reference/tokens.md#content-tokens) | [`float-literal-exponent-form.slang`](float-literal-exponent-form.slang) |
| Boundary - a FloatingPointLiteral with a leading dot and no integer part (`.5`) is a single token starting at the `.`. | boundary | [#content-tokens](../../../docs/llm-generated/syntax-reference/tokens.md#content-tokens) | [`float-literal-leading-dot.slang`](float-literal-leading-dot.slang) |
| Boundary - a binary integer literal (`0b1111`) lexes as IntegerLiteral and decodes to its base-2 value. | boundary | [#content-tokens](../../../docs/llm-generated/syntax-reference/tokens.md#content-tokens) | [`int-literal-binary-base.slang`](int-literal-binary-base.slang) |
| Boundary - a hex IntegerLiteral that does not fit in 32 bits (`0x100000000`) is still a valid token and resolves to `int64_t`. | boundary | [#content-tokens](../../../docs/llm-generated/syntax-reference/tokens.md#content-tokens) | [`int-literal-hex-promotes-to-64bit.slang`](int-literal-hex-promotes-to-64bit.slang) |
| Boundary - a hexadecimal floating-point literal (`0x1.0p4`) is a valid FloatingPointLiteral and decodes to its base-16 mantissa times 2^exponent. | boundary | [#content-tokens](../../../docs/llm-generated/syntax-reference/tokens.md#content-tokens) | [`float-literal-hex-form.slang`](float-literal-hex-form.slang) |
| Boundary - a hexadecimal integer literal (`0xFF`) is a valid IntegerLiteral and decodes to its base-16 value. | boundary | [#content-tokens](../../../docs/llm-generated/syntax-reference/tokens.md#content-tokens) | [`int-literal-hex-base.slang`](int-literal-hex-base.slang) |
| Boundary - an octal integer literal (`0777`) lexes as IntegerLiteral and decodes to its base-8 value. | boundary | [#content-tokens](../../../docs/llm-generated/syntax-reference/tokens.md#content-tokens) | [`int-literal-octal-base.slang`](int-literal-octal-base.slang) |
| Boundary - escape sequences (`\t`, `\n`, `\\`, `\"`) inside a regular StringLiteral are accepted by the lexer and round-trip through to emitted C++. | boundary | [#content-tokens](../../../docs/llm-generated/syntax-reference/tokens.md#content-tokens) | [`string-literal-escape-sequences.slang`](string-literal-escape-sequences.slang) |
| Boundary - the `h` suffix on a FloatingPointLiteral types it as `half` (16-bit float). | boundary | [#content-tokens](../../../docs/llm-generated/syntax-reference/tokens.md#content-tokens) | [`numeric-suffix-h-is-half.slang`](numeric-suffix-h-is-half.slang) |
| Boundary - the `lf` suffix on a FloatingPointLiteral types it as `double` (64-bit float). | boundary | [#content-tokens](../../../docs/llm-generated/syntax-reference/tokens.md#content-tokens) | [`numeric-suffix-lf-is-double.slang`](numeric-suffix-lf-is-double.slang) |
| Boundary - the `ll` suffix on an IntegerLiteral types it as `int64_t` (signed 64-bit). | boundary | [#content-tokens](../../../docs/llm-generated/syntax-reference/tokens.md#content-tokens) | [`numeric-suffix-ll-is-int64.slang`](numeric-suffix-ll-is-int64.slang) |
| Boundary - the `ull` suffix on an IntegerLiteral types it as `uint64_t` (unsigned 64-bit). | boundary | [#content-tokens](../../../docs/llm-generated/syntax-reference/tokens.md#content-tokens) | [`numeric-suffix-ull-is-uint64.slang`](numeric-suffix-ull-is-uint64.slang) |
| Boundary - the empty StringLiteral (`""`) is a valid token whose content is zero characters. | boundary | [#content-tokens](../../../docs/llm-generated/syntax-reference/tokens.md#content-tokens) | [`string-literal-empty.slang`](string-literal-empty.slang) |
| Boundary - the integer literal `0` lexes as IntegerLiteral with int type at the lower edge of the int range. | boundary | [#content-tokens](../../../docs/llm-generated/syntax-reference/tokens.md#content-tokens) | [`int-literal-zero.slang`](int-literal-zero.slang) |
| Boundary - the null escape `'\0'` lexes as a CharLiteral and decodes to integer 0. | boundary | [#content-tokens](../../../docs/llm-generated/syntax-reference/tokens.md#content-tokens) | [`char-literal-null.slang`](char-literal-null.slang) |
| Negative - a CharLiteral with more than one character (`'AB'`) is rejected with the "illegal character literal" diagnostic. | negative | [#content-tokens](../../../docs/llm-generated/syntax-reference/tokens.md#content-tokens) | [`negative-multi-char-literal.slang`](negative-multi-char-literal.slang) |
| Negative - an integer literal followed by a non-documented suffix (`xyz`) triggers the "invalid suffix on integer literal" diagnostic. | negative | [#content-tokens](../../../docs/llm-generated/syntax-reference/tokens.md#content-tokens) | [`negative-invalid-int-suffix.slang`](negative-invalid-int-suffix.slang) |
| Negative - an integer literal too wide for any built-in integer type triggers the documented "integer literal is too large" diagnostic. | negative | [#content-tokens](../../../docs/llm-generated/syntax-reference/tokens.md#content-tokens) | [`negative-integer-literal-too-large.slang`](negative-integer-literal-too-large.slang) |
| The `f` suffix on a floating-point literal types the token as `float` (single precision), distinguishing it from the unsuffixed form. | functional | [#content-tokens](../../../docs/llm-generated/syntax-reference/tokens.md#content-tokens) | [`numeric-suffix-f-is-float.slang`](numeric-suffix-f-is-float.slang) |
| The `u` suffix on an integer literal makes the token's type unsigned; the suffix is part of the literal token's raw text and is decoded by the parser/checker. | functional | [#content-tokens](../../../docs/llm-generated/syntax-reference/tokens.md#content-tokens) | [`numeric-suffix-u-is-uint.slang`](numeric-suffix-u-is-uint.slang) |
| A backslash immediately before a newline is consumed and folded out; the two halves of an identifier split across the continuation become a single token. | functional | [#special-case-lexing-rules](../../../docs/llm-generated/syntax-reference/tokens.md#special-case-lexing-rules) | [`backslash-line-continuation.slang`](backslash-line-continuation.slang) |
| Block comments do not nest; an inner `/*` inside `/* ... */` is content, and the outer comment ends at the first `*/`. | functional | [#special-case-lexing-rules](../../../docs/llm-generated/syntax-reference/tokens.md#special-case-lexing-rules) | [`block-comment-no-nesting.slang`](block-comment-no-nesting.slang) |
| Boundary - a multi-character delimiter (`long`) on a raw string opens with `R"long(` and closes only at `)long"`. | boundary | [#special-case-lexing-rules](../../../docs/llm-generated/syntax-reference/tokens.md#special-case-lexing-rules) | [`raw-string-multi-char-delimiter.slang`](raw-string-multi-char-delimiter.slang) |
| Boundary - a raw string with an empty delimiter (`R"(...)"`) lexes correctly, terminating only at `)"`. | boundary | [#special-case-lexing-rules](../../../docs/llm-generated/syntax-reference/tokens.md#special-case-lexing-rules) | [`raw-string-empty-delimiter.slang`](raw-string-empty-delimiter.slang) |
| Boundary - backslash-line-continuation runs before lexing, so a `\<newline>` at the end of a `//` line splices the following physical line into the comment and the line is consumed by it. | boundary | [#special-case-lexing-rules](../../../docs/llm-generated/syntax-reference/tokens.md#special-case-lexing-rules) | [`line-comment-backslash-extends.slang`](line-comment-backslash-extends.slang) |
| Inside a raw string literal `R"d(...)d"`, backslashes are taken literally — no escape processing is performed by the lexer. | functional | [#special-case-lexing-rules](../../../docs/llm-generated/syntax-reference/tokens.md#special-case-lexing-rules) | [`raw-string-no-escape.slang`](raw-string-no-escape.slang) |
| Stress - multiple `/*` sequences inside a block comment do not stack; the comment still ends at the first `*/` regardless of how many fake openers precede it. | stress | [#special-case-lexing-rules](../../../docs/llm-generated/syntax-reference/tokens.md#special-case-lexing-rules) | [`block-comment-multi-fake-nest.slang`](block-comment-multi-fake-nest.slang) |
| A `BlockComment` (`/* ... */`) is treated as trivia; code on either side of the comment is parsed normally and the comment text has no semantic effect. | functional | [#trivia-whitespace-and-comments](../../../docs/llm-generated/syntax-reference/tokens.md#trivia-whitespace-and-comments) | [`block-comment-removed.slang`](block-comment-removed.slang) |
| A `LineComment` (`// ...`) ends at the end of its physical line; code on the following line is not part of the comment. | functional | [#trivia-whitespace-and-comments](../../../docs/llm-generated/syntax-reference/tokens.md#trivia-whitespace-and-comments) | [`line-comment-ends-at-newline.slang`](line-comment-ends-at-newline.slang) |
| Boundary - an empty block comment (`/**/`) is a valid BlockComment token and is removed from the token stream. | boundary | [#trivia-whitespace-and-comments](../../../docs/llm-generated/syntax-reference/tokens.md#trivia-whitespace-and-comments) | [`block-comment-empty.slang`](block-comment-empty.slang) |

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

| Anchor | Kind | Gap | Suggested addition |
| --- | --- | --- | --- |
| [#lambda-requirement-binding](../../../docs/llm-generated/syntax-reference/tokens.md#lambda-requirement-binding) | undocumented-behavior | The doc describes `RightArrow` (`->`), `DoubleRightArrow` (`=>`), `Ellipsis` (`...`), and `Scope` (`::`) as punctuation tokens but does not state user-facing claims about their semantic role beyond one-line notes like "lambda / requirement-binding". Tests for these would need to anchor to `syntax-reference/grammar.md` or `ast-reference/expressions.md`, neither of which has been generated yet. Deferred to Phase C cross-link, or to expansion once those bundles exist. |  |
| [#token-flags](../../../docs/llm-generated/syntax-reference/tokens.md#token-flags) | undocumented-behavior | The doc lists `TokenFlags::AtStartOfLine`, `AfterWhitespace`, and `ScrubbingNeeded` (under `#token-flags`) but these are internal lexer state with no surface observable consequence accessible via `slangc`. Recorded as undocumented-by-design rather than a doc gap. |  |
| [#and](../../../docs/llm-generated/syntax-reference/tokens.md#and) | undocumented-behavior | Numeric literal suffixes beyond `u` and `f` (`l`, `ul`, `h`, `lf`, ...) are mentioned but not exhaustively enumerated. Expansion pass candidate once a definitive list is added to the doc. |  |
| [#0x](../../../docs/llm-generated/syntax-reference/tokens.md#0x) | undocumented-behavior | Integer-literal bases (decimal / hex `0x` / octal `0` / binary `0b`) are exercised by the lexer but the doc does not enumerate the accepted prefixes. Boundary tests for each base now exist; the doc would benefit from a one-line note that all four bases are accepted. |  |
| [#content-tokens](../../../docs/llm-generated/syntax-reference/tokens.md#content-tokens) | undocumented-behavior | Floating-point literal forms (trailing dot `1.`, leading dot `.5`, decimal exponent `1e3`, hexadecimal float `0x1.0p4`) are accepted by the lexer but only the generic "FloatingPointLiteral" row in `#content-tokens` is documented. | A short grammar note in `#content-tokens` listing the four accepted shapes would close this gap. |
| [#special-case-lexing-rules](../../../docs/llm-generated/syntax-reference/tokens.md#special-case-lexing-rules) | missing-example | The interaction of `\<newline>` continuation with line comments (the comment swallows the next physical line because continuations are folded before comment recognition) is implied by the "consumed and folded out" wording in `#special-case-lexing-rules` but not stated explicitly. | A worked example in the doc would prevent a confusing-looking but correct behavior from surprising readers. |
| [#diagnostics](../../../docs/llm-generated/syntax-reference/tokens.md#diagnostics) | undocumented-behavior | Documented diagnostics (`integer literal is too large to be represented in any integer type`, `invalid suffix on integer literal`, `illegal character literal`) have no explicit error codes / messages listed in `tokens.md`. The negative tests in this pass copy the diagnostic text verbatim from the compiler; promoting them to a "Diagnostics" subsection in the doc would let future authors anchor by claim ID rather than by free text. |  |

## Out of scope (no-GPU runner)

None for this bundle. Lexer-level behaviors are fully observable
through `slangi`.
