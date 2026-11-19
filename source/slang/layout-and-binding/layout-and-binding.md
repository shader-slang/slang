# Layout and Binding

This concern provides two related services: laying out types and binding program parameters for a target.
Type layout determines the storage and binding resources required by a type, including the relative offsets of its fields and elements.
Parameter binding assigns locations or binding ranges to particular parameters using their type layouts, explicit binding declarations, and the ranges already assigned in the program.

These are distinct tasks.
For example, computing the byte offsets of fields in a constant-buffer type lays out the type; assigning a declared constant-buffer parameter to a descriptor binding binds the parameter.
Keeping the services together reflects how parameter binding uses type layouts without treating the two operations as synonyms.

## Concepts

A _type layout_ describes the resources consumed by a type under a set of layout rules.
A _variable layout_ combines a type layout with information about a particular variable or parameter, including its assigned offsets or bindings.
A _program layout_ collects the layout and binding information associated with a compiled program and its entry points.
The [compiler overview](../../../docs/design/overview.md#parameter-binding-and-type-layout) explains the relationship to reflection and target selection.

## Dependencies

- [AST](../ast/ast.md) provides checked types, declarations, and modifiers inspected when computing layouts and assigning bindings.
- [Semantic checking](../semantic-checking/semantic-checking.md) provides type and declaration queries used by layout computation and lookup used during binding.
- [IR](../ir/ir.md) provides the representation used when layout information is recorded in IR.
- [IR passes](../ir-passes/ir-passes.md) collect hashed string literals used when constructing program layout information.
- Compiler targets, profiles, options, and component types supply layout rules and the program whose parameters receive bindings.
- Compiler diagnostics and `source/compiler-core/` provide error reporting and target/artifact classification.

## Decomposition and Ownership

`slang-type-layout.*` defines layout objects and computes layouts for types and program interfaces.
`slang-parameter-binding.*` assigns parameter bindings and builds program layout information.
Reflection API implementation remains in the compiler directory and exposes information computed by these services.

Natural-layout helpers specific to AST types remain in [AST](../ast/ast.md), and natural-layout helpers specific to IR types remain in [IR](../ir/ir.md).
IR transformations that use or change layout information remain in [IR passes](../ir-passes/ir-passes.md).
The directory therefore identifies the front-end layout and binding services rather than collecting every operation that computes a size or uses an offset.
