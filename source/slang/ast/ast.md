# The AST

The AST represents unchecked and checked Slang programs.
It provides declarations, expressions, statements, types, compile-time values, and the relationships needed to represent the meaning of a checked program.
Its services include construction, traversal, cloning, substitution, printing, and inspection of those objects.

## Concepts

- A _syntax node_ represents a construct such as a declaration, expression, statement, or modifier.
- A _compile-time value_, represented by `Val`, includes types and other semantic values used while checking and specializing programs.
- A [declaration reference](../../../docs/design/decl-refs.md) identifies a declaration together with its substitution context.
- An _AST builder_ owns and constructs AST objects and canonical semantic values.

The [semantic checking design](../../../docs/design/semantic-checking.md) describes how checking enriches and transforms the program represented by these objects.

## Dependencies

- `source/core/` supplies storage, collections, memory arenas, and other basic services.
- `source/compiler-core/` supplies names, tokens, and source locations retained in the program representation.
- Compiler capability definitions describe capability information represented by AST values and modifiers.
- [Semantic checking](../semantic-checking/semantic-checking.md) supplies services still used by some AST operations, including type-related queries and lookup during cloning.
- [IR](../ir/ir.md) supplies IR definitions referenced by the AST builder and related representation support.
- [Serialization](../serialization/serialization.md) supplies on-demand reading of declaration members exposed through AST operations.
- [Emission](../emission/emission.md) supplies the text writer used for AST dumps.

## Decomposition and Ownership

The `slang-ast-*` files define the node families, builders, declaration references, semantic values, and operations over them.
`slang-syntax.*` provides shared queries and operations over this representation, and `slang-visitor.h` supplies traversal support.
`slang-capability-val.h` defines the AST value form of capability information.
Generated Fiddle metadata supports dispatch, construction, traversal, and other operations on the node hierarchy.

`slang-ast-natural-layout.*` computes natural sizes and alignments for AST types and remains with the representation it inspects.
The broader service of laying out types and binding program parameters belongs to [layout and binding](../layout-and-binding/layout-and-binding.md).
AST serialization belongs to [serialization](../serialization/serialization.md), and translating checked programs into IR belongs to [lowering to IR](../lower-to-ir/lower-to-ir.md).
