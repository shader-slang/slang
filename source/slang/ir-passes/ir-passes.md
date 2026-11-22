# IR Passes

IR passes analyze, validate, link, specialize, and transform programs represented in Slang IR.
They implement work ranging from general optimizations to automatic differentiation and transformations required by particular targets.
This concern groups those operations separately from the definition and shared manipulation of the representation itself.

## Concepts

A _pass_ performs a particular analysis, validation, or transformation over IR.
_Legalization_ transforms a program into a form supported by a target or a later compilation step.
An analysis can supply information to several transformations without itself rewriting the program.
The [IR design](../../../docs/design/ir.md) defines the representation on which these operations work.

## Dependencies

- [IR](../ir/ir.md) supplies instructions, builders, and shared representation utilities.
- Compiler targets, options, capabilities, and profiles determine which transformations apply and which forms are supported.
- Compiler diagnostics and `source/compiler-core/` supply error reporting and downstream target information.
- [AST](../ast/ast.md) supplies shared representation support used by type legalization and differentiation implementations.
- [Emission](../emission/emission.md) supplies common emitter services used by SPIR-V legalization.
- [Layout and binding](../layout-and-binding/layout-and-binding.md) supplies layout objects and rules used by type and parameter legalization.
- Other passes provide analyses and transformations composed into larger operations.

## Decomposition and Ownership

The `slang-ir-*` files here implement analyses, validation of program properties, linking, specialization, optimizations, automatic differentiation, and target legalization.
Their existing file groups remain intact within this directory.
`slang-legalize-types.*` supplies shared type-legalization machinery, and `slang-pass-wrapper.*` supplies pass instrumentation such as dumping, validation, and timing.
`slang-check-out-of-bound-access.*` checks accesses in IR despite its name lacking the `slang-ir-` prefix.
The [automatic differentiation design](../../../docs/design/autodiff.md) describes one substantial group of related transformations.

The compiler's pass orchestration remains in files such as `../slang-emit.cpp`, `../slang-code-gen.cpp`, and the [lowering implementation](../lower-to-ir/lower-to-ir.md).
A pass's location does not establish when it runs; its callers still select and order the operation.
This decomposition does not introduce separate target, optimization, or differentiation directories, or require changes to the current pass pipeline.
