---
generated: true
model: claude-opus-4.7
generated_at: 2026-05-07T14:35:56+00:00
source_commit: 3da83a82d83ad1b0fbd58465ed3a89d2880533dd
watched_paths_digest: e2c6f1441dbe013ee44a514220f358519fb7666c14bf549fa51c11558ff1dd3e
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Token Reference

This document catalogs the tokens emitted by the Slang lexer. The
intended reader is a developer extending the lexer or writing tooling
that consumes Slang source.

## Source

The catalog is reverse-engineered from:

- [slang-token.h](../../../source/compiler-core/slang-token.h) —
  `Token`, `TokenType`, `TokenFlags`.
- [slang-token-defs.h](../../../source/compiler-core/slang-token-defs.h)
  — the X-macro list of every `TokenType` value, included by
  [slang-token.h](../../../source/compiler-core/slang-token.h) and
  several other places.
- [slang-lexer.h](../../../source/compiler-core/slang-lexer.h) /
  [slang-lexer.cpp](../../../source/compiler-core/slang-lexer.cpp)
  — the tokenizer that produces them.

## Token-kind taxonomy

Tokens come in five groups: end markers, content tokens (literals
and identifiers), trivia (whitespace and comments), preprocessor
markers, and punctuation / operators. The `PUNCTUATION(id, text)`
macro in
[slang-token-defs.h](../../../source/compiler-core/slang-token-defs.h)
expands to a `TOKEN(id, "'<text>'")` so every punctuation kind is
both a `TokenType` enumerator and a string used in diagnostics. The
"Lexer source range" column points at the lexer code that emits the
kind; in `slang-lexer.cpp` the bulk of the dispatch lives in
`Lexer::lexToken` / `Lexer::lexTokenImpl`.

### End markers and special

| TokenKind | Lexer source range | Notes |
| --- | --- | --- |
| `Unknown` | default-constructed `Token` | Should not appear in valid input |
| `EndOfFile` | end-of-buffer branch of `lexTokenImpl` | Returned when the lexer reaches end of input |
| `Invalid` | error branch of `lexTokenImpl` (`slang-lexer.cpp` around line 1739) | Lexer hit a character it cannot classify; emits a diagnostic unless `kLexerFlag_SuppressDiagnostics` is set, but the `Invalid` token is still produced |

### Content tokens

| TokenKind | Lexer source range | Notes |
| --- | --- | --- |
| `Identifier` | identifier rule in `slang-lexer.cpp` | Includes every keyword; classification deferred to the parser via syntax-decl lookup |
| `IntegerLiteral` | integer-literal rule in `slang-lexer.cpp` | Suffixes (`u`, `l`, `ul`, ...) are part of the token's raw text |
| `FloatingPointLiteral` | float-literal rule in `slang-lexer.cpp` | Suffixes (`f`, `lf`, ...) are part of the token's raw text |
| `StringLiteral` | string-literal rule in `slang-lexer.cpp` (regular, raw-string, and include-header forms) | Includes the opening / closing quotes; escape sequences are not yet decoded |
| `CharLiteral` | char-literal rule in `slang-lexer.cpp` | Single-quoted character literal |

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
| `CompletionRequest` | synthesized by the language-server pipeline | `#?`; emitted at the cursor position to request completion (`slang-completion-token.cpp`) |

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
[slang-token.h](../../../source/compiler-core/slang-token.h) carries:

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
[slang-token.h](../../../source/compiler-core/slang-token.h)) is a
bitmask that records lexical properties:

| Flag | Meaning |
| --- | --- |
| `AtStartOfLine` | Token is the first non-whitespace token on its physical line (used by the preprocessor for directive recognition) |
| `AfterWhitespace` | Token was preceded by whitespace (relevant to macro pasting) |
| `ScrubbingNeeded` | Token text contains line-continuation characters that must be removed before use |
| `Name` | Discriminates the `chars` / `name` union |

## Special-case lexing rules

The lexer in
[slang-lexer.cpp](../../../source/compiler-core/slang-lexer.cpp)
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
  is performed. Implementation lives in `slang-lexer.cpp` (raw-string
  prefix detection around line 1025 and termination around line
  1427).
- **Numeric literal suffixes.** Suffix characters (`u`, `l`, `f`,
  `h`, ...) are kept as part of the literal token's raw text. The
  parser / checker decodes them when interpreting the value.
- **Block-comment handling.** `BlockComment` tokens cover the entire
  `/* ... */` range; nested block comments are not supported.
- **Identifier / keyword classification.** Every keyword arrives at
  the parser as `TokenType::Identifier`. Keyword status is determined
  by lookup in the parser's syntax-decl table; see
  [keywords-and-builtins.md](keywords-and-builtins.md).

## Source location

Every token's `SourceLoc` is a 32-bit integer decoded by
`SourceManager`
([slang-source-loc.h](../../../source/compiler-core/slang-source-loc.h),
[slang-source-loc.cpp](../../../source/compiler-core/slang-source-loc.cpp)).
The encoding distinguishes "spelling" (where the text physically
lives) from "expansion" (where the macro-expanded use occurred); both
are reachable through the source manager when formatting diagnostics.

## What this catalog does not cover

- Keywords. Every keyword arrives as `TokenType::Identifier`; the
  classification and inventory live in
  [keywords-and-builtins.md](keywords-and-builtins.md).
- Grammar productions. See [grammar.md](grammar.md).
