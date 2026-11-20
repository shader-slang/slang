# Lowering to IR

Lowering translates a checked program from the AST into Slang IR.
It also creates IR that records component specialization and explicit type-conformance requests.
The resulting modules provide input to linking, specialization, validation, and further transformation.

## Concepts

A _lowered value_ is the IR representation produced for a checked construct.
An _IR module_ can contain definitions and references to definitions supplied by other modules; generating one module does not imply that all imported definitions have been linked into it.
The [compiler overview](../../../docs/design/overview.md#lowering-and-mandatory-optimizations) describes the translation and the processing that follows it.

## Dependencies

- [AST](../ast/ast.md) and [semantic checking](../semantic-checking/semantic-checking.md) supply checked constructs, semantic queries, and conformance information.
- [IR](../ir/ir.md) supplies instructions, builders, and utilities for constructing the translated program.
- [IR passes](../ir-passes/ir-passes.md) provide validation and transformations invoked as part of generating a module.
- [Layout and binding](../layout-and-binding/layout-and-binding.md) supplies layout information used during translation.
- Compiler requests, component types, mangling, and diagnostics connect the generated module to the rest of compilation.

## Decomposition and Ownership

`slang-lower-to-ir.h` declares generation entry points for translation units, specialized component types, and type conformances.
`slang-lower-to-ir.cpp` implements their translation and the associated processing of generated IR.

The directory separates the AST-to-IR translation from transformations whose input and output are already IR.
Some of those transformations are invoked by the lowering implementation; they remain in the IR-pass directory rather than being moved according to their caller.
