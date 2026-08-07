---
generated: true
model: claude-opus-5
generated_at: 2026-08-03T13:08:24Z
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
watched_paths_digest: 1a02145d6df9d35fc274dac0e859c24bfa85f644da98d8b22f9096154e3f16ae
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Token Reference

This document catalogs the tokens emitted by the Slang lexer. The
intended reader is a developer extending the lexer or writing tooling
that consumes Slang source.

## Source

The catalog is reverse-engineered from:

- [slang-token.h](../../../../source/compiler-core/slang-token.h) —
  `Token`, `TokenType`, `TokenFlags`.
- [slang-token-defs.h](../../../../source/compiler-core/slang-token-defs.h)
  — the X-macro list of every `TokenType` value, included by
  [slang-token.h](../../../../source/compiler-core/slang-token.h) and
  several other places.
- [slang-token.cpp](../../../../source/compiler-core/slang-token.cpp)
  — `TokenTypeToString`, which expands the same definition list into
  the diagnostic spelling for each kind.
- [slang-lexer.h](../../../../source/compiler-core/slang-lexer.h) /
  [slang-lexer.cpp](../../../../source/compiler-core/slang-lexer.cpp)
  — the tokenizer that produces them.
- [slang-preprocessor.cpp](../../../../source/slang/slang-preprocessor.cpp)
  — the consumer that reads the token flags, and the owner of
  angle-bracket include-path assembly in `HandleIncludeDirective`.

## Token-kind taxonomy

Tokens come in five groups: end markers, content tokens (literals
and identifiers), trivia (whitespace and comments), preprocessor
markers, and punctuation / operators. The `PUNCTUATION(id, text)`
macro in
[slang-token-defs.h](../../../../source/compiler-core/slang-token-defs.h)
expands to a `TOKEN(id, "'<text>'")` so every punctuation kind is
both a `TokenType` enumerator and a string used in diagnostics. The
"Lexer source range" column points at the lexer code that emits the
kind. In `slang-lexer.cpp` the per-character dispatch lives in the
free function `_lexTokenImpl` (around line 1966), wrapped by the
`Lexer::lexToken` member (around line 2473), which attaches flags and
returns token text with any folded line continuations already
removed. The folding itself happens in the character-level helpers
`_peek` (which looks past a continuation) and `_advance` (which
consumes one and records `ScrubbingNeeded`).

### End markers and special

| TokenKind | Lexer source range | Notes |
| --- | --- | --- |
| `Unknown` | default-constructed `Token` | Should not appear in valid input |
| `EndOfFile` | `kEOF` arm of `_lexTokenImpl` (`slang-lexer.cpp` around line 1974) | Returned when the lexer reaches end of input |
| `Invalid` | fall-through block at the end of `_lexTokenImpl` (`slang-lexer.cpp` lines 2440-2470) | Lexer hit a character that matched no dispatch arm and is not a non-ASCII code point (those are folded into identifiers); it chooses between `illegalCharacterPrint`, `unexpectedEndOfInput`, and `illegalCharacterHex` by the character's value, and skips the diagnostic entirely when `kLexerFlag_SuppressDiagnostics` is set (see `Lexer::getDiagnosticSink` in [slang-lexer.h](../../../../source/compiler-core/slang-lexer.h)). The `Invalid` token is produced either way |

### Content tokens

| TokenKind | Lexer source range | Notes |
| --- | --- | --- |
| `Identifier` | identifier rule in `slang-lexer.cpp` | Includes every keyword; classification deferred to the parser via syntax-decl lookup |
| `IntegerLiteral` | integer-literal rule in `slang-lexer.cpp` | Suffixes (`u`, `l`, `ul`, ...) are part of the token's raw text |
| `FloatingPointLiteral` | float-literal rule in `slang-lexer.cpp` | Suffixes (`f`, `lf`, ...) are part of the token's raw text |
| `StringLiteral` | `_lexStringLiteralBody(lexer, '"')` (line 2172) / `_lexRawStringLiteralBody` (line 2166) in `slang-lexer.cpp` | Raw token text includes the opening / closing quotes; escape sequences are decoded and validated later by `getStringLiteralTokenValue` |
| `CharLiteral` | `_lexStringLiteralBody(lexer, '\'')` in `slang-lexer.cpp` (line 2177) | Single-quoted character literal. The lexer only finds the closing quote; the one-character rule is enforced at decode time by `getCharLiteralValue` (lines 1660-1725) |

### Trivia (whitespace and comments)

The lexer emits these as their own tokens so the preprocessor and
parser can choose whether to skip them. Most parsing layers filter
them out of the token stream they iterate.

| TokenKind | Lexer source range | Notes |
| --- | --- | --- |
| `WhiteSpace` | whitespace rule in `slang-lexer.cpp` | Run of spaces / tabs |
| `NewLine` | end-of-line rule in `slang-lexer.cpp` | Logical line terminator (after backslash continuations are folded) |
| `LineComment` | `//` rule in `slang-lexer.cpp` | `// ...` to end of line |
| `BlockComment` | `/* ... */` rule in `slang-lexer.cpp` | `/* ... */`; nested block comments are not supported |

### Preprocessor markers

| TokenKind | Lexer source range | Notes |
| --- | --- | --- |
| `Pound` | `#` punctuation in `slang-lexer.cpp` | Preprocessor directive prefix |
| `PoundPound` | `##` punctuation in `slang-lexer.cpp` | Preprocessor token paste |
| `CompletionRequest` | `#?` arm of the `#` branch in `_lexTokenImpl` (`slang-lexer.cpp` around line 2367) | `#?`; emitted at the cursor position to request completion |

### Punctuation and structural symbols

Listed by spelling; the lexer routes each through the per-character
`switch` in `_lexTokenImpl` in `slang-lexer.cpp`.

| TokenKind | Lexer source range | Notes |
| --- | --- | --- |
| `Semicolon` | `;` punctuation | |
| `Comma` | `,` punctuation | |
| `Dot` | `.` punctuation | |
| `DotDot` | `..` punctuation | Lexed as a distinct kind; no parser consumer at this commit |
| `Ellipsis` | `...` punctuation | Consumed by the preprocessor for variadic macro parameters |
| `LBrace` | `{` punctuation | |
| `RBrace` | `}` punctuation | |
| `LBracket` | `[` punctuation | |
| `RBracket` | `]` punctuation | |
| `LParent` | `(` punctuation | |
| `RParent` | `)` punctuation | |
| `Colon` | `:` punctuation | |
| `Scope` | `::` punctuation | Namespace / qualified-name separator |
| `QuestionMark` | `?` punctuation | Conditional / optional |
| `RightArrow` | `->` punctuation | Function return type, member access through pointer |
| `DoubleRightArrow` | `=>` punctuation | Lambda syntax (its only parser consumer) |
| `At` | `@` punctuation | |
| `Dollar` | `$` punctuation | |
| `DollarDollar` | `$$` punctuation | |

### Operators

Assignment, arithmetic, comparison, logical, and bitwise operators.

| TokenKind | Lexer source range | Notes |
| --- | --- | --- |
| `OpAssign` | `=` punctuation | |
| `OpAdd` | `+` punctuation | |
| `OpSub` | `-` punctuation | |
| `OpMul` | `*` punctuation | |
| `OpDiv` | `/` punctuation | |
| `OpMod` | `%` punctuation | |
| `OpNot` | `!` punctuation | Logical not |
| `OpBitNot` | `~` punctuation | Bitwise not |
| `OpLsh` | `<<` punctuation | |
| `OpRsh` | `>>` punctuation | |
| `OpEql` | `==` punctuation | |
| `OpNeq` | `!=` punctuation | |
| `OpGreater` | `>` punctuation | |
| `OpLess` | `<` punctuation | Disambiguated from generic application by the parser; see [../pipeline/02-parse-ast.md](../pipeline/02-parse-ast.md) |
| `OpGeq` | `>=` punctuation | |
| `OpLeq` | `<=` punctuation | |
| `OpAnd` | `&&` punctuation | Logical and |
| `OpOr` | `\|\|` punctuation | Logical or |
| `OpBitAnd` | `&` punctuation | Bitwise / address-of |
| `OpBitOr` | `\|` punctuation | |
| `OpBitXor` | `^` punctuation | |
| `OpInc` | `++` punctuation | |
| `OpDec` | `--` punctuation | |
| `OpAddAssign` | `+=` punctuation | |
| `OpSubAssign` | `-=` punctuation | |
| `OpMulAssign` | `*=` punctuation | |
| `OpDivAssign` | `/=` punctuation | |
| `OpModAssign` | `%=` punctuation | |
| `OpShlAssign` | `<<=` punctuation | |
| `OpShrAssign` | `>>=` punctuation | |
| `OpAndAssign` | `&=` punctuation | |
| `OpOrAssign` | `\|=` punctuation | |
| `OpXorAssign` | `^=` punctuation | |

## Token data layout

The `Token` struct in
[slang-token.h](../../../../source/compiler-core/slang-token.h) carries:

```cpp
class Token
{
public:
    TokenType type = TokenType::Unknown;
    TokenFlags flags = 0;
    SourceLoc loc;
    uint32_t charsCount = 0;
    union CharsNameUnion
    {
        const char* chars;
        Name* name;
    };
    CharsNameUnion charsNameUnion;
    // ...
};
```

The `charsNameUnion` is a tagged union: when the `Name` flag bit is
set, the token text is interned as a `Name*` (used for identifiers
and keywords); otherwise the token holds a raw pointer plus length
into the original source buffer.

## Token flags

`TokenFlag` (declared in
[slang-token.h](../../../../source/compiler-core/slang-token.h)) is a
bitmask that records lexical properties:

| Flag | Meaning |
| --- | --- |
| `AtStartOfLine` | Token is the first token on a logical line — set after an emitted `NewLine` token and preserved across intervening whitespace and comments, so escaped newlines (which emit no `NewLine`) do not start a new one (used by the preprocessor for directive recognition) |
| `AfterWhitespace` | Token was preceded by whitespace; the preprocessor reads it to preserve spacing when stringizing and to tell a function-like macro definition (`NAME(` with no gap) from an object-like one |
| `ScrubbingNeeded` | A line continuation was folded while lexing this token; `Lexer::lexToken` uses the flag to scrub the continuation out of the stored content |
| `Name` | Discriminates the `chars` / `name` union |

## Special-case lexing rules

The lexer in
[slang-lexer.cpp](../../../../source/compiler-core/slang-lexer.cpp)
implements several context-sensitive rules:

- **Backslash line continuation.** A `\` immediately before a newline
  is consumed and folded out by `_advance`, but the resulting token's
  source location still refers to the original physical line. The
  `ScrubbingNeeded` flag is set so that `Lexer::lexToken` strips the
  continuation from the content it stores on the token.
- **`<...>` after `#include`.** The lexer has no include-header mode:
  it emits ordinary `OpLess`, path, and `OpGreater` tokens.
  `HandleIncludeDirective` in
  [slang-preprocessor.cpp](../../../../source/slang/slang-preprocessor.cpp)
  reassembles the path by concatenating the contents of the tokens
  between `<` and `>`; the quoted form is a single `StringLiteral`.
- **Raw string literals.** A string opened with `R"delimiter(` is
  closed only by `)delimiter"` for an arbitrary `delimiter`. Inside,
  newlines and backslashes are taken literally — no escape processing
  is performed. Implementation lives in `_lexRawStringLiteralBody`
  (`slang-lexer.cpp` lines 1782-1829, with the closing-delimiter
  termination check around lines 1802-1812), invoked from the `R"`
  arm of the string-literal dispatch at line 2166. A bare `"` as the
  delimiter is rejected with `LexerDiagnostics::quoteCannotBeDelimiter`.
- **Character literals.** A `'`-quoted body is lexed by the same
  `_lexStringLiteralBody` helper as strings; the helper takes the
  closing quote character as its second argument (`'\''` for character
  literals at line 2177, `'"'` for strings at line 2172) rather than a
  separate single-character flag. While lexing, the helper does only the
  minimum escape handling needed to find the real closing quote — it
  steps over `\'`, `\"`, and `\\` so an escaped quote does not end the
  token — and reports only the two failures that prevent finding an end
  at all: `LexerDiagnostics::endOfFileInLiteral` and
  `LexerDiagnostics::newlineInLiteral`.

  Everything about the literal's *value* is deferred. The
  one-character rule is enforced by `getCharLiteralValue`
  (`slang-lexer.cpp` lines 1660-1725), which emits
  `LexerDiagnostics::illegalCharacterLiteral` both for a body too short
  to hold a character (`''`) and for a body whose decode leaves
  unconsumed input (a multi-character body), returning `-1` in either
  case. Escape sequences — including the `\u` / `\U` Unicode forms,
  which diagnose `invalidUnicodeStringEscape` on a wrong digit count —
  are decoded by the shared `_decodeStringEscape`, and a malformed UTF-8
  body reports `invalidUtf8ByteSequence`. A `CharLiteral` token is still
  produced in all of these cases; the failure surfaces only when a
  consumer asks for the value.
- **Numeric literal suffixes.** Suffix characters (`u`, `l`, `f`,
  `h`, ...) are kept as part of the literal token's raw text, so the
  token itself does not record which type was requested. Decoding is
  the consumer's job and happens later.
- **Leading-zero floating-point continuations.** A bare `0` followed
  by a base-10 exponent (`0e10`, `0E5`, `0e+1`, `0e-3`) or by the
  legacy MSVC infinity form (`0#INF`) is lexed as a
  `FloatingPointLiteral`, matching the `1e10` / `1#INF` forms. The
  `default:` arm of the `0` branch in `_lexTokenImpl`
  ([slang-lexer.cpp](../../../../source/compiler-core/slang-lexer.cpp)
  line 2059) consults `_maybeLexNumberExponent` (line 608) before
  falling back to an `IntegerLiteral`; without it the exponent would be
  swallowed as an integer suffix. The sibling arms of the same branch
  handle the other bases (`0x`/`0X` hex, `0b`/`0B` binary, and a leading
  digit run that is lexed base-8 after a `LexerDiagnostics::octalLiteral`
  diagnostic).
- **Block-comment handling.** `BlockComment` tokens cover the entire
  `/* ... */` range; nested block comments are not supported.
- **Identifier / keyword classification.** Every keyword arrives at
  the parser as `TokenType::Identifier`. Keyword status is determined
  by lookup in the parser's syntax-decl table; see
  [keywords-and-builtins.md](keywords-and-builtins.md).

## Source location

Every token's `SourceLoc` is a 32-bit integer decoded by
`SourceManager`
([slang-source-loc.h](../../../../source/compiler-core/slang-source-loc.h),
[slang-source-loc.cpp](../../../../source/compiler-core/slang-source-loc.cpp)).
The integer is a key into one `SourceView`, not a pair of locations:
the interpretation is chosen by the `SourceLocType` argument
(`Nominal`, which honours `#line` directives and source maps;
`Actual`, which ignores them; and `Emit`, which honours `#line` but
ignores source maps). Where a macro-expanded or otherwise derived
view came from is recorded separately, on the `SourceView` itself, and
retrieved with `getInitiatingSourceLoc`.

## What this catalog does not cover

- Keywords. Every keyword arrives as `TokenType::Identifier`; the
  classification and inventory live in
  [keywords-and-builtins.md](keywords-and-builtins.md).
- Grammar productions. See [grammar.md](grammar.md).
