# Prompt: syntax-reference/keywords-and-builtins.md

See [_common.md](_common.md) for universal rules.

## Target

Produce
`docs/llm-generated/syntax-reference/keywords-and-builtins.md` —
an inventory of the syntactic keywords and built-in syntax declarations
that constitute Slang's surface vocabulary.

The non-obvious fact this document must convey: most Slang "keywords"
are not lexer keywords, they are identifiers bound to syntax in the
default environment. Reverse-engineering them therefore requires
reading the parser's syntax-decl registration and the meta-modules
that populate the environment.

Audience: a developer adding a new keyword or trying to understand why
a specific identifier is special.

## Required structure

1. `# Keywords and Built-in Syntax` (title)
2. `## Where keywords come from` — short paragraph explaining the
   syntax-as-declaration model, citing
   [slang-parser.cpp](../../../source/slang/slang-parser.cpp) and the
   `meta.slang` files in the watched paths.
3. `## Lexer-recognized keywords` — list any tokens that the lexer
   does treat specially (operators, punctuators that have dedicated
   `TokenKind` values; verify in
   [slang-token.h](../../../source/compiler-core/slang-token.h)). Do
   not invent entries.
4. `## Parser-registered syntax keywords` — keywords whose meaning is
   determined by `addBuiltinSyntax` / `add*Syntax` calls in
   [slang-parser.cpp](../../../source/slang/slang-parser.cpp). Group
   them by what they introduce:
   - Declaration keywords (`struct`, `class`, `enum`, `interface`,
     `extension`, `func`, `var`, `let`, `typedef`, `import`,
     `module`, `namespace`, ...)
   - Statement keywords (`if`, `for`, `while`, `do`, `break`,
     `continue`, `return`, `switch`, `case`, `default`, `discard`,
     ...)
   - Expression keywords (`new`, `sizeof`, `__target_intrinsic`,
     `unsafeBitCast`, `as`, ...)
   - Modifier keywords (`in`, `out`, `inout`, `ref`, `const`,
     `static`, `uniform`, `nointerpolation`, `precise`, ...)
   Verify each keyword by searching for it in the watched files.
5. `## Core-module syntax declarations` — what the `*.meta.slang`
   files in `source/slang/core.meta.slang`,
   `source/slang/hlsl.meta.slang`,
   `source/slang/glsl.meta.slang`,
   `source/slang/diff.meta.slang` contribute to the active
   environment. State that these are processed at compiler startup
   and that they introduce types, functions, and intrinsic operators.
6. `## Reserved identifier prefixes` — note any conventions
   (`__intrinsic`, `__builtin`, etc.) used in the meta-modules and
   by the parser.

## Quality checklist (in addition to the universal one)

- [ ] Every keyword listed is grep-able in at least one of the watched
      files; no fabricated entries.
- [ ] Do not duplicate the operator table that belongs in
      [grammar.md](grammar.md).
- [ ] Document length under 32 KB.
