# The Intermediate Representation

The IR represents programs in the form used for linking, specialization, optimization, and target legalization.
This concern defines the representation and provides the construction and manipulation services shared by its producers and consumers.

The [IR design](../../../docs/design/ir.md) explains the representation.
The [instruction definition guide](../../../docs/design/ir-instruction-definition.md) describes the instruction definitions and generated support.

## Concepts

An _instruction_ represents a value, operation, type, or other IR construct.
Instructions have operands and may contain child instructions, allowing the representation to describe modules, functions, blocks, and their contents.
A _decoration_ attaches additional information to an IR construct.
An _IR builder_ constructs instructions and maintains the representation's construction rules.

## Dependencies

- `source/core/` supplies storage, collections, allocation, and basic utilities.
- `source/compiler-core/` supplies names and source information retained in IR and its diagnostics.
- [AST](../ast/ast.md) supplies shared type and declaration vocabulary used by instruction definitions.
- [Layout and binding](../layout-and-binding/layout-and-binding.md) supplies resource-kind and size definitions used by IR layout objects.
- Compiler capability, profile, and target definitions describe information encoded in or queried from IR.
- [IR passes](../ir-passes/ir-passes.md) supply dominator information used by representation validation and other shared IR operations.

## Decomposition and Ownership

`slang-ir.*` and the `slang-ir-insts*` files define the instruction hierarchy, builders, instruction information, and generated definitions.
The directory also contains shared utilities for operations such as cloning, inspection, and validation of the representation.
The Lua instruction definitions and templates live with the code they generate.
`slang-ir-validate.*` checks representation invariants but also contains some target-specific validation of program properties.
We keep that existing file together rather than split it to make the boundary exact.

`slang-ir-layout.*` computes layout information for IR types, including their natural sizes and alignments, and remains with the IR services that inspect those types.
[Layout and binding](../layout-and-binding/layout-and-binding.md) owns the front-end services for laying out types and assigning program parameter bindings.
The [IR passes](../ir-passes/ir-passes.md) concern owns program analyses and transformations, and [serialization](../serialization/serialization.md) owns reading and writing IR in compiler containers.

The distinction is between providing the representation and performing compilation work over a program in that representation.
Existing utility and pass dependencies cross this boundary, and the directory layout does not impose a new layering rule.
