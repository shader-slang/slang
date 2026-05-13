# Prompt: syntax-reference/grammar.md

See [_common.md](_common.md) for universal rules.

## Target

Produce `docs/llm-generated/syntax-reference/grammar.md` — an
EBNF-style grammar for Slang surface syntax, reverse-engineered from
the parser implementation.

This document is intentionally informal: Slang has no formal grammar,
and the parser's heuristic disambiguation cannot be expressed in a
context-free grammar. The goal is to capture the **structural shape**
of programs the parser accepts, with explicit notes where heuristics
or context-sensitivity intrude.

Audience: a tooling developer (syntax highlighter, formatter, language
server) who needs an approximate description of what the parser
recognizes.

## Required structure

1. `# Slang Grammar (Reverse-Engineered)` (title)
2. `## Caveats` — explicit list:
   - This grammar is reverse-engineered from
     [slang-parser.cpp](../../../source/slang/slang-parser.cpp), not
     designed; mismatches with the implementation are bugs in the
     grammar.
   - It is context-sensitive: identifiers are classified at lookup
     time, not by grammar.
   - The two-stage parser means function bodies parse with a different
     ambiguity strategy than declarations; see
     [../pipeline/02-parse-ast.md](../pipeline/02-parse-ast.md).
3. `## Notation` — describe the EBNF dialect used:
   - `RULE ::= ALTERNATIVES`
   - `?` optional, `*` zero-or-more, `+` one-or-more
   - `(...)` grouping
   - terminals in `'single quotes'` or as named token kinds
     (`IDENT`, `INT_LIT`, `FLOAT_LIT`, `STRING_LIT`, ...) cross-
     referenced to [tokens.md](tokens.md).
4. `## Top-level structure` — productions for `module`,
   `import-decl`, `using-decl`, namespace blocks.
5. `## Declarations` — productions for `var-decl`, `func-decl`,
   `struct-decl`, `class-decl`, `enum-decl`, `interface-decl`,
   `extension-decl`, `typedef-decl`, generic-parameter syntax.
6. `## Statements` — productions for the structured statements
   (`block`, `if`, `for`, `while`, `do-while`, `switch`, `return`,
   `break`, `continue`, `discard`, `defer` if present).
7. `## Expressions` — productions for the expression grammar
   following the parser's precedence ladder. Cite
   [slang-parser.cpp](../../../source/slang/slang-parser.cpp) and
   note the heuristic for `<` (generic application vs. comparison).
8. `## Modifiers` — production for the modifier syntax that prefixes
   declarations.
9. `## Attributes and decorations` — `[attribute]` syntax and how it
   parses.
10. `## Generics and where-clauses` — productions; note the
    deferred-body interaction.

Every production must be verifiable by reading the named parsing
functions in [slang-parser.cpp](../../../source/slang/slang-parser.cpp).
When you state a production, parenthetically cite the parser function
that implements it (e.g. "implemented in `parseDecl`").

## Quality checklist (in addition to the universal one)

- [ ] Every terminal is either an explicit string literal or one of
      the token kinds in [tokens.md](tokens.md).
- [ ] Every non-terminal cites at least one parser function in
      [slang-parser.cpp](../../../source/slang/slang-parser.cpp).
- [ ] Productions for ambiguous constructs (`<` in expressions, type
      vs. value identifiers) include a note explaining how the parser
      disambiguates.
- [ ] Document length under 48 KB. If larger, summarize sub-grammars
      and link out to dedicated sub-pages (and propose adding them to
      the manifest).
