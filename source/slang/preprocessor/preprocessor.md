# The Preprocessor

The preprocessor expands macros, processes conditional directives and includes, and returns the tokens that the parser will read.
It also records source-language directives and exposes callbacks for file dependencies and other compilation services.

## Concepts

A _macro environment_ associates names with the macro definitions used during expansion.
An _input stream_ supplies tokens from a file or macro expansion; the preprocessor manages multiple such streams as preprocessing proceeds.
Source locations and source views are represented by the shared infrastructure in `source/compiler-core/`.

## Dependencies

- `source/compiler-core/` supplies the lexer, tokens, source manager, include resolution, and diagnostic sink used to process files and expanded tokens.
- Compiler profiles and language-version definitions in the parent directory describe language selection recorded by directives.
- Compiler linkage supplies the input services and configuration associated with a compilation request.
- [Language server](../language-server/language-server.md) defines the content-assist records in which preprocessing reports macro definitions, macro invocations, and includes.

## Decomposition and Ownership

`slang-preprocessor.h` declares the preprocessing description, callbacks, and entry points.
`slang-preprocessor.cpp` implements directive handling, macro expansion, and the management of input streams.
The [compiler overview](../../../docs/design/overview.md#preprocessing) explains the relationship between preprocessing and parsing.

Lexing and source management remain in `source/compiler-core/`.
The preprocessor's compiler-specific overloads still depend on linkage and editor state; placing preprocessing in its own directory does not change those dependencies.
