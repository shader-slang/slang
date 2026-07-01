---
generated: true
model: claude-opus-4.8
generated_at: 2026-06-29T13:22:37Z
source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
watched_paths_digest: 835ca8ae0e597a608474aaa8d581c75de3d392cc320259f50ca47f254b43433d
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
- [slang-lexer.h](../../../../source/compiler-core/slang-lexer.h) /
  [slang-lexer.cpp](../../../../source/compiler-core/slang-lexer.cpp)
  — the tokenizer that produces them.

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
free function `_lexTokenImpl` (around line 1736), wrapped by the
`Lexer::lexToken` member (around line 2243), which loops to fold out
line continuations and attach flags.

### End markers and special

| TokenKind | Lexer source range | Notes |
| --- | --- | --- |
| `Unknown` | default-constructed `Token` | Should not appear in valid input |
| `EndOfFile` | end-of-buffer branch of `_lexTokenImpl` (`slang-lexer.cpp` around line 1745) | Returned when the lexer reaches end of input |
| `Invalid` | error branch of `_lexTokenImpl` (`slang-lexer.cpp` around line 2239) | Lexer hit a character it cannot classify; emits a diagnostic unless `kLexerFlag_SuppressDiagnostics` is set (see `Lexer::getDiagnosticSink` in [slang-lexer.h](../../../../source/compiler-core/slang-lexer.h)), but the `Invalid` token is still produced |

### Content tokens

| TokenKind | Lexer source range | Notes |
| --- | --- | --- |
| `Identifier` | identifier rule in `slang-lexer.cpp` | Includes every keyword; classification deferred to the parser via syntax-decl lookup |
| `IntegerLiteral` | integer-literal rule in `slang-lexer.cpp` | Suffixes (`u`, `l`, `ul`, ...) are part of the token's raw text |
| `FloatingPointLiteral` | float-literal rule in `slang-lexer.cpp` | Suffixes (`f`, `lf`, ...) are part of the token's raw text |
| `StringLiteral` | `_lexStringLiteralBody` / `_lexRawStringLiteralBody` in `slang-lexer.cpp` (regular, raw-string, and include-header forms) | Raw token text includes the opening / closing quotes; escape sequences are decoded later by `getStringLiteralTokenValue` |
| `CharLiteral` | `_lexStringLiteralBody` in `slang-lexer.cpp`, invoked with `singleChar = true` | Single-quoted character literal; the lexer diagnoses empty (`''`) and multi-character bodies, and `getCharLiteralValue` decodes the code point |

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
| `CompletionRequest` | `#?` arm of the `#` branch in `_lexTokenImpl` (`slang-lexer.cpp` around line 2137) | `#?`; emitted at the cursor position to request completion |

### Punctuation and structural symbols

Listed by spelling; the lexer routes each through the punctuation
dispatch table in `slang-lexer.cpp`.

| TokenKind | Lexer source range | Notes |
| --- | --- | --- |
| `Semicolon` | `;` punctuation | |
| `Comma` | `,` punctuation | |
| `Dot` | `.` punctuation | |
| `DotDot` | `..` punctuation | Range / inclusive-range syntax in some contexts |
| `Ellipsis` | `...` punctuation | Variadic / pack expansion |
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
| `DoubleRightArrow` | `=>` punctuation | Lambda / requirement-binding |
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
| `AtStartOfLine` | Token is the first non-whitespace token on its physical line (used by the preprocessor for directive recognition) |
| `AfterWhitespace` | Token was preceded by whitespace (relevant to macro pasting) |
| `ScrubbingNeeded` | Token text contains line-continuation characters that must be removed before use |
| `Name` | Discriminates the `chars` / `name` union |

## Special-case lexing rules

The lexer in
[slang-lexer.cpp](../../../../source/compiler-core/slang-lexer.cpp)
implements several context-sensitive rules:

- **Backslash line continuation.** A `\` immediately before a newline
  is consumed and folded out, but the resulting token's source
  location still refers to the original physical line. The
  `ScrubbingNeeded` flag is set so consumers know to strip the
  continuation when reading raw text.
- **`<...>` after `#include`.** When the lexer is in include-header
  mode, `<foo/bar.h>` is tokenized as a single `StringLiteral` rather
  than as comparison operators.
- **Raw string literals.** A string opened with `R"delimiter(` is
  closed only by `)delimiter"` for an arbitrary `delimiter`. Inside,
  newlines and backslashes are taken literally — no escape processing
  is performed. Implementation lives in `_lexRawStringLiteralBody`
  (`slang-lexer.cpp` lines 1545-1592, with the closing-delimiter
  termination check around lines 1564-1576), invoked from the `R"`
  arm of the string-literal dispatch at line 1936. A bare `"` as the
  delimiter is rejected with `LexerDiagnostics::quoteCannotBeDelimiter`.
- **Character literals.** A `'`-quoted body is lexed by the same
  `_lexStringLiteralBody` helper as strings, passing `singleChar = true`
  (`slang-lexer.cpp` around line 1351). The lexer enforces the
  one-character rule eagerly: an empty body or a body that reaches a
  second character emits `LexerDiagnostics::illegalCharacterLiteral`,
  and the resulting token is still returned as a `CharLiteral`.
- **Numeric literal suffixes.** Suffix characters (`u`, `l`, `f`,
  `h`, ...) are kept as part of the literal token's raw text. The
  parser / checker decodes them when interpreting the value.
- **Leading-zero floating-point continuations.** A bare `0` followed
  by a base-10 exponent (`0e10`, `0E5`, `0e+1`, `0e-3`) or by the
  legacy MSVC infinity form (`0#INF`) is lexed as a
  `FloatingPointLiteral`, matching the `1e10` / `1#INF` forms. The
  `default:` arm of the `0` branch in `_lexTokenImpl`
  ([slang-lexer.cpp](../../../../source/compiler-core/slang-lexer.cpp))
  consults `_maybeLexNumberExponent` before falling back to an
  `IntegerLiteral`; without it the exponent would be swallowed as an
  integer suffix.
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
The encoding distinguishes "spelling" (where the text physically
lives) from "expansion" (where the macro-expanded use occurred); both
are reachable through the source manager when formatting diagnostics.

## What this catalog does not cover

- Keywords. Every keyword arrives as `TokenType::Identifier`; the
  classification and inventory live in
  [keywords-and-builtins.md](keywords-and-builtins.md).
- Grammar productions. See [grammar.md](grammar.md).
