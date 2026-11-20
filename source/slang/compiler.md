# The Compiler

The compiler implements the Slang language and its compilation services.
It preprocesses and parses programs, checks their meaning, translates them into IR, specializes and transforms that IR, and emits code for the requested targets.
It also provides type layout, parameter binding, reflection, serialization, and the language server's understanding of Slang programs.

`source/slang/` contains the compiler implementation rather than the entire Slang project.
The [source organization strategy](../../docs/design/source-organization.md) describes the conventions used by this document and its children.
The [compiler overview](../../docs/design/overview.md) provides a longer explanation of the compilation model and pipeline.

## Concepts

- A _translation unit_ groups the files checked together as a program module.
- A _component type_ represents a composable unit of compilation, such as a module, an entry point, or a specialization.
- A _target_ identifies the requested output format and the capabilities available for that output.
- The [AST](ast/ast.md) represents both unchecked and checked Slang programs, including their types and compile-time values.
- The [IR](ir/ir.md) represents programs in the form used for linking, specialization, optimization, and target legalization.

## Dependencies

- `source/core/` supplies containers, memory management, files, strings, and platform services shared by the implementation.
- `source/compiler-core/` supplies lexing, tokens, source locations and source management, diagnostic sinks, artifacts, downstream compiler integration, and protocol infrastructure.
  These services retain their existing ownership outside this directory.
- `include/` defines the public interfaces implemented by the compiler.
- The standard modules and preludes supply built-in declarations and target support used during compilation.

## Decomposition

The children below describe responsibilities, rather than independent libraries or enforced layers.
All are directly under `source/slang/`; this initial organization does not introduce deeper directories.

| Concern                                                        | Services                                                                |
| -------------------------------------------------------------- | ----------------------------------------------------------------------- |
| [Parser](parser/parser.md)                                     | Construct the program AST from tokens and parse deferred bodies.        |
| [Preprocessor](preprocessor/preprocessor.md)                   | Expand macros and process includes and conditional directives.          |
| [AST](ast/ast.md)                                              | Represent and manipulate unchecked and checked programs.                |
| [Semantic Checking](semantic-checking/semantic-checking.md)    | Resolve names and establish the meaning and validity of programs.       |
| [Lowering to IR](lower-to-ir/lower-to-ir.md)                   | Translate checked programs and specialization information into IR.      |
| [IR](ir/ir.md)                                                 | Define, build, inspect, and manipulate the intermediate representation. |
| [IR Passes](ir-passes/ir-passes.md)                            | Analyze, validate, link, specialize, and transform IR programs.         |
| [Emission](emission/emission.md)                               | Produce target code from the representations prepared for emission.     |
| [Layout and Binding](layout-and-binding/layout-and-binding.md) | Lay out types and bind program parameters for a target.                 |
| [Serialization](serialization/serialization.md)                | Read and write compiler representations and module containers.          |
| [Language Server](language-server/language-server.md)          | Answer editor requests using program and workspace information.         |

## Ownership Notes

The files directly in this directory retain compiler services whose decomposition has not been introduced here.
They include public API implementation, compilation requests and sessions, module and component composition, compiler options, capabilities and profiles, reflection, diagnostics, built-in module construction, documentation generation, and repro support.
Their location does not make those responsibilities a single additional concern.

Some files at this level also coordinate several children.
For example, `slang-code-gen.cpp` and `slang-emit.cpp` arrange processing and emission rather than implementing only one concrete emitter.
`slang-check.cpp` contains both semantic entry points and session methods for loading downstream compilers.
Those files remain at this level rather than being split to complete the directory map.

The dependency graph still contains cycles and dependencies on implementation details in neighboring directories.
For example, the parser uses semantic lookup, and semantic checking invokes the parser for deferred bodies.
The AST representation also uses checking services in some operations.
The child descriptions record these relationships; moving the files does not change them or impose a new visibility policy.
