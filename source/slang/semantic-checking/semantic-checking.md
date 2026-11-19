# Semantic Checking

Semantic checking establishes the meaning and validity of a program represented by the AST.
It resolves names, checks declarations and expressions, resolves overloads and generic constraints, and verifies conformance to interface requirements.
Checking also constructs the semantic information needed by lowering, reflection, and editor services.

The [semantic checking design](../../../docs/design/semantic-checking.md) describes checking phases and the handling of declaration dependencies.
The [interfaces design](../../../docs/design/interfaces.md) explains the generic and interface model used by conformance checking.

## Concepts

A _declaration check state_ records how far checking has progressed for a declaration.
Checking can request information from other declarations before their bodies have been checked.
A _witness_ represents evidence used to satisfy a constraint or interface requirement.
Declaration references and semantic values use the representation defined by the [AST](../ast/ast.md).

## Dependencies

- [AST](../ast/ast.md) provides the program being checked, its builders, semantic values, and substitutions.
- [Parser](../parser/parser.md) parses deferred statement bodies when checking reaches them.
- [Emission](../emission/emission.md) supplies SPIR-V extended-instruction name lookup used to check inline assembly.
- [Language server](../language-server/language-server.md) defines content-assist records that checking uses to report completion suggestions.
- [IR](../ir/ir.md) supplies operation identifiers used to recognize intrinsic types and analyze loops during checking.
- [Layout and binding](../layout-and-binding/layout-and-binding.md) supplies shared parsing of semantic names such as `COLOR0` into a name and index.
- Compiler sessions, modules, capability definitions, and profiles supply the compilation context and the rules checked for a program.
- `source/compiler-core/` and compiler diagnostics supply names, source information, and diagnostic reporting.

## Decomposition and Ownership

The specialized `slang-check-*` files divide checking by tasks such as declarations, expressions, statements, conversions, overloads, constraints, inheritance, and conformance.
`slang-check-impl.h` shares the checking context and visitor implementation across those files.
`slang-lookup.*` implements the name and member lookup used by checking and by context-sensitive parsing.
`slang-check.h` declares shared checking services.

`../slang-check.cpp` remains in the compiler directory because it also implements session services for loading downstream compilers.
Its semantic entry points still belong conceptually to checking.
Lookup, overload resolution, and conformance are not separate child directories in this decomposition; their shared implementation context remains intact.
