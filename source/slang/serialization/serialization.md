# Serialization

Serialization reads and writes the compiler's program representations and module containers.
It supplies shared serialization machinery and the encoding of AST objects, IR, source locations, and related compiler data.
Loading serialized declarations also supports the compiler's on-demand access to previously compiled modules.

The [serialization design](../../../docs/design/serialization.md) describes the Fossil format.
The [IR module compatibility design](../../../docs/design/backwards-compat-for-ir-modules.md) discusses compatibility requirements for serialized IR.

## Concepts

A _container_ groups the serialized data needed for a compiled module or library.
_Fossil_ is the serialization format that uses relative offsets to represent object graphs in a memory-mappable buffer.
_On-demand deserialization_ reconstructs declarations when the compiler requests them rather than loading every declaration immediately.

## Dependencies

- [AST](../ast/ast.md) and [IR](../ir/ir.md) define the program objects encoded by the serializers.
- `source/compiler-core/` supplies source locations, artifact infrastructure, and related shared compiler data.
- Compiler modules, sessions, and linkage supply the context needed to reconstruct usable program objects.
- [Semantic checking](../semantic-checking/semantic-checking.md) and [parser](../parser/parser.md) services participate in reconstructing AST data and syntax callbacks.
- `source/core/` supplies buffers, streams, containers, and allocation.

## Decomposition and Ownership

The `slang-serialize*` files provide serialization operations, compiler-specific encodings, and container support.
`slang-fossil.*` and `slang-serialize-fossil.*` implement the Fossil representation and its connection to the shared serializer interface.
Generated AST serialization support remains associated with the AST definitions and the serializer implementation that uses it.

The directory groups the existing formats and mechanisms without consolidating them or changing their encodings.
Repro capture, reflection JSON, and module-loading policy remain in the compiler directory; they use or accompany serialization but do not become part of this concern solely because they read or write data.
