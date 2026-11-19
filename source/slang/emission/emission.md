# Emission

Emission produces target code from the program representations prepared by the compiler.
It provides concrete emitters for textual languages and binary forms, together with shared services for writing code, expanding intrinsic definitions, and recording required target extensions.

## Concepts

A _target emitter_ translates the representation accepted by that emitter into the requested output form.
A _source writer_ manages emitted text and its mapping to input source locations.
An _intrinsic expansion_ supplies target code for an operation using its target-specific definition.
The [IR design](../../../docs/design/ir.md) describes the principal program representation consumed by target emitters.

## Dependencies

- [IR](../ir/ir.md) provides the instructions and decorations inspected by most emitters.
- [AST](../ast/ast.md) provides shared support types, including names paired with source locations.
- [IR passes](../ir-passes/ir-passes.md) provide control-flow restructuring and other services used during emission.
- [Layout and binding](../layout-and-binding/layout-and-binding.md) supplies layout information used to emit parameter locations and storage declarations.
- Compiler targets, profiles, capabilities, and options control the output form and target requirements.
- `source/compiler-core/` supplies artifacts, source maps, downstream compiler interfaces, and target grammar information.
- `source/core/` provides text, binary storage, writers, and platform utilities.

## Decomposition and Ownership

The `slang-emit-*` files in this directory implement the concrete emitters and their shared support, including base emitters, precedence, and source writing.
`slang-intrinsic-expand.*` and `slang-extension-tracker.*` provide intrinsic expansion and target-extension tracking used by those emitters.
`slang-ir-spirv-snippet.*` parses and represents the embedded SPIR-V intrinsic templates used by SPIR-V emission.
`slang-lookup-spirv.h` declares lookup of SPIR-V extended-instruction names and includes shared SPIR-V definitions.
The parser and semantic checker also use that header; its location here preserves an existing coupling between recognition of SPIR-V operations and their emission.
The directory retains the existing file boundaries without adding a directory for each target.

`../slang-emit.cpp` and `../slang-code-gen.cpp` remain in the compiler directory because they coordinate compilation and pass pipelines as well as emission.
`../slang-emit-dependency-file.*` also remains there because it reports compilation input dependencies for build tools rather than producing target code.
Transformations that prepare IR for a target remain with [IR passes](../ir-passes/ir-passes.md).
This separation records primary responsibilities; it does not change existing dependencies between emitters and those transformations.
