# The Parser

The parser constructs declarations, statements, and expressions from a stream of preprocessed tokens.
It also parses deferred bodies when semantic checking requests them and sets up the callbacks used to recognize syntax declarations.

The [parsing design](../../../docs/design/parsing.md) explains the parsing approach in more detail.

## Concepts

A _syntax declaration_ associates a name with a parsing callback, allowing the parser to look up syntax in a scope.
A _deferred body_ is a statement body whose tokens have been retained for parsing during semantic checking.
The parser constructs the [AST](../ast/ast.md), which also represents the program after checking.

## Dependencies

- `source/compiler-core/` provides token streams, token reading, source locations, and lexing utilities.
- [AST](../ast/ast.md) provides the objects and builders used to construct the parsed program.
- [Semantic checking](../semantic-checking/semantic-checking.md) provides lookup and semantic context used by context-sensitive parsing.
- [Emission](../emission/emission.md) currently supplies the helper header through which the parser includes SPIR-V definitions for inline assembly.
- Compiler requests, language modes, and diagnostics in the parent directory supply the surrounding compilation state and error reporting.

## Decomposition and Ownership

`slang-parser.h` declares the parsing entry points, and `slang-parser.cpp` implements the recursive descent parser and syntax callbacks.
The implementation remains in one file; the directory identifies a concern that can be decomposed further when that work is warranted.

Preprocessing belongs to the [preprocessor](../preprocessor/preprocessor.md), and tokenization belongs to `source/compiler-core/`.
Parsing is not independent of checking: the parser uses semantic lookup, and checking calls `parseUnparsedStmt` to parse deferred bodies with the appropriate scopes and semantic context.
This directory boundary preserves that interaction.
