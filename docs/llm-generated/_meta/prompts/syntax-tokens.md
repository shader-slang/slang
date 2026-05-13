# Prompt: syntax-reference/tokens.md

See [_common.md](_common.md) for universal rules.

## Target

Produce `docs/llm-generated/syntax-reference/tokens.md` — an
exhaustive catalog of token kinds emitted by the lexer.

Audience: a developer extending the lexer or writing a tool that
consumes Slang source.

## Required structure

1. `# Token Reference` (title)
2. `## Source` — name the watched paths
   ([slang-token.h](../../../source/compiler-core/slang-token.h),
   [slang-lexer.cpp](../../../source/compiler-core/slang-lexer.cpp))
   and state that this catalog is reverse-engineered from those files.
3. `## Token-kind taxonomy` — a table with one row per `TokenKind`
   value, grouped by category:
   - End markers (`EndOfFile`, `Invalid`, ...)
   - Literals (integer, float, string, character, identifier)
   - Punctuation and operators (parentheses, brackets, braces,
     `::`, `->`, `==`, `<=`, ...)
   - Whitespace and trivia (`WhiteSpace`, `NewLine`, `LineComment`,
     `BlockComment`)
   - Preprocessor markers (`Pound`, `PoundPound`, ...)
   - Special / pseudo-kinds the lexer emits internally.

   Each row:

   ```markdown
   | TokenKind | Lexer source range | Notes |
   | --- | --- | --- |
   | `Identifier` | identifier rule in slang-lexer.cpp | Includes keywords; classification deferred to parser/semantic-check |
   ```

4. `## Token flags` — describe the bit flags on `Token` that record
   leading/trailing whitespace, suppression flags, etc., as defined in
   [slang-token.h](../../../source/compiler-core/slang-token.h).
5. `## Special-case lexing rules` — `<...>` after `#include`, line
   continuations with backslash, raw-string-like constructs (only if
   they are present in the watched files), numeric-literal suffix
   handling (only the structural fact that suffixes are part of the
   token text and parsed later).
6. `## Source location` — one paragraph: every token carries a single
   integer source location decoded by
   [slang-source-loc.h](../../../source/compiler-core/slang-source-loc.h).

## Quality checklist (in addition to the universal one)

- [ ] Token-kind list is anchored to enumerators that actually appear
      in the watched paths. Do not invent kinds.
- [ ] Identifier classification: explicitly note that keywords are
      not distinguished here; defer to
      [keywords-and-builtins.md](keywords-and-builtins.md).
- [ ] Document length under 24 KB.
