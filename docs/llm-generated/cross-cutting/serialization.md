---
generated: true
model: claude-opus-4.7
generated_at: 2026-05-07T14:35:56+00:00
source_commit: 3da83a82d83ad1b0fbd58465ed3a89d2880533dd
watched_paths_digest: 5b25292bf14e2a45191187f721db5ff6afa45b617dfab12381292b32174d4546
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Serialization

This document describes the AST and IR serialization machinery: what
gets serialized, in which format, and how round-tripping works. The
intended reader is a developer adding a serialized field, debugging a
deserialization failure, or working on cross-version stability of IR
modules.

## What is serialized

Three flavors of payload exist, each with its own driver file:

- **AST modules** — handled by
  [slang-serialize-ast.h](../../../source/slang/slang-serialize-ast.h)
  /
  [slang-serialize-ast.cpp](../../../source/slang/slang-serialize-ast.cpp).
  The serialized form preserves the checked AST that backs an
  `import`-able module.
- **IR modules** — handled by
  [slang-serialize-ir.h](../../../source/slang/slang-serialize-ir.h)
  /
  [slang-serialize-ir.cpp](../../../source/slang/slang-serialize-ir.cpp),
  with the IR-specific type vocabulary in
  [slang-serialize-ir-types.h](../../../source/slang/slang-serialize-ir-types.h)
  /
  [slang-serialize-ir-types.cpp](../../../source/slang/slang-serialize-ir-types.cpp).
- **Containers** — the on-disk packaging that bundles AST + IR + side
  data into a single file (the `.slang-module` format and other
  artefacts). Driver:
  [slang-serialize-container.h](../../../source/slang/slang-serialize-container.h)
  /
  [slang-serialize-container.cpp](../../../source/slang/slang-serialize-container.cpp);
  underlying chunked format in
  [slang-serialize-riff.h](../../../source/slang/slang-serialize-riff.h)
  /
  [slang-serialize-riff.cpp](../../../source/slang/slang-serialize-riff.cpp).

Source-location streams have their own helper file
([slang-serialize-source-loc.h](../../../source/slang/slang-serialize-source-loc.h)
/
[slang-serialize-source-loc.cpp](../../../source/slang/slang-serialize-source-loc.cpp)),
because preserving readable diagnostics across deserialization
requires re-establishing the `SourceManager`'s view of files and
expansions.

## The serialize() pattern

The generic interface is
[slang-serialize.h](../../../source/slang/slang-serialize.h). The
preamble of that header captures the central design choice: a single
`serialize(serializer, value)` function handles both reading and
writing, distinguished by a `SerializerMode` carried on the
`serializer` argument. Per the file's own example:

```cpp
struct MyThing
{
    float a;
    List<OtherThing> otherThings;
    SomeObject* object;
};

template<typename S>
void serialize(S const& serializer, MyThing& value)
{
    SLANG_SCOPED_SERIALIZER_STRUCT(serializer);
    serialize(serializer, value.a);
    serialize(serializer, value.otherThings);
    serialize(serializer, value.object);
}
```

The same template runs in both directions. This rules out
"serialize" / "deserialize" code drift by construction. The
`SLANG_SCOPED_SERIALIZER_STRUCT` macro brackets a structural scope so
the format can record nesting; the field-by-field calls do the work.

`serialize(serializer, value.object)` — pointer fields — work because
the serializer maintains a graph of objects it has already visited
and emits / resolves identifiers for shared and circular references.

## Backends

The generic `serialize(...)` pattern is implemented against multiple
`ISerializerImpl` backends; the backends visible in the watched paths
are:

### Generic / structural backend

[slang-serialize.cpp](../../../source/slang/slang-serialize.cpp)
implements the core `ISerializerImpl` infrastructure and the dispatch
between reader and writer modes. Concrete encoders for primitive
types live in
[slang-serialize-types.cpp](../../../source/slang/slang-serialize-types.cpp).

### Fossil backend

[slang-serialize-fossil.h](../../../source/slang/slang-serialize-fossil.h)
/
[slang-serialize-fossil.cpp](../../../source/slang/slang-serialize-fossil.cpp).
"Fossil" is a memory-mappable binary format defined in `slang-fossil.h`
(referenced from the fossil header). Per the comments in
[slang-serialize-fossil.h](../../../source/slang/slang-serialize-fossil.h):

> Deserializing data is an important place where security issues can
> arise, so it is usually important to perform validation checks
> throughout the process, and fail fast rather than risk reading
> mal-formed data.

The validation cost is configurable via the macro
`SLANG_SERIALIZE_FOSSIL_ENABLE_VALIDATION_CHECKS` (default 1). When
enabled, validation failures call `SLANG_UNEXPECTED("invalid format
encountered in serialized data")`. When disabled — used for
performance-critical paths such as loading the embedded core module —
the same conditions become assertions.

The format is designed for **memory-mapped** deserialization: pointers
in the serialized data are relative offsets resolved against the start
of the mapped buffer (`slang-relative-ptr.h`), so loading a module is
nearly free if the file is already mapped.

## RIFF container format

[slang-serialize-riff.h](../../../source/slang/slang-serialize-riff.h)
/
[slang-serialize-riff.cpp](../../../source/slang/slang-serialize-riff.cpp)
implement a chunked, tagged container based on the RIFF idiom: each
chunk has a four-character code, a length, and a payload. Container-
level orchestration in
[slang-serialize-container.cpp](../../../source/slang/slang-serialize-container.cpp)
composes a serialized AST module, a serialized IR module, and the
auxiliary data (source-location stream, dependency list, etc.) into
one RIFF file with well-known chunk codes per payload.

The RIFF wrapping is what allows tools to inspect partial structure
of a `.slang-module` file (chunk types, sizes) without parsing the
inner serialized content — useful for sanity checks and recovery.

## Source-location serialization

Source locations are tricky to round-trip because the integer encoding
in
[slang-source-loc.h](../../../source/compiler-core/slang-source-loc.h)
is meaningful only relative to the live `SourceManager` of the
session that produced it. The serializer therefore captures the
contributing `SourceFile` records (path, content, expansion stack)
alongside the integer locations and reconstructs them into a fresh
`SourceManager` on load.

Driver: [slang-serialize-source-loc.cpp](../../../source/slang/slang-serialize-source-loc.cpp).

## Versioning and backwards compatibility

Modules are expected to remain loadable by newer compilers, even
though new IR opcodes are added over time. The constraint that makes
this possible is at the top of
[slang-ir-insts.lua](../../../source/slang/slang-ir-insts.lua):

> Please make sure to update the supported module versions in
> Slang::IRModule accordingly when modifying this file.

When the Lua file is edited, the C++ side carries a versioning gate
for the deserializer so that older modules continue to load against
a (well-defined) prefix of the opcode set, while newer opcodes
become available only when the module declares the matching version.
The full design is described in
[../../design/backwards-compat-for-ir-modules.md](../../design/backwards-compat-for-ir-modules.md).

The `Unrecognized` opcode that appears at the head of
[slang-ir-insts.lua](../../../source/slang/slang-ir-insts.lua) plays
a role here: it is the placeholder a deserializer substitutes for an
opcode it does not know, and the comment requires that
`Unrecognized` never survive past deserialization.

## Round-trip and repro files

Historical `-dump-repro` / `-load-repro` machinery for capturing a
single `slangc` invocation as a serialized blob is documented in
[CLAUDE.md](../../../CLAUDE.md) as **deprecated**: it should not be
relied on. The watched-paths set for this document does not include
the repro implementation, so any newer alternative workflow is out
of scope for this page.

## Adding a new serialized field

1. Add the field to its host C++ type as usual.
2. Update the corresponding `serialize(...)` function (typically in a
   `slang-ast-*.cpp` or `slang-ir-*.cpp` file) so that it visits the
   new field with a `serialize(serializer, value.newField);` call.
3. Decide whether the new field needs to be readable by older
   compilers. If not, place the call after a version gate (see
   [../../design/backwards-compat-for-ir-modules.md](../../design/backwards-compat-for-ir-modules.md)).
4. If the field is a pointer to an externally-owned object, ensure
   the target type is itself serializable.
5. Add tests under [tests/](../../../tests/) that exercise the
   round-trip — typically a `COMPARE_COMPUTE` or `INTERPRET` test
   plus a separate test that loads a pre-built module file.

## What is not in this document

- The format-level layout of a fossil chunk or a RIFF chunk. The
  authoritative descriptions are in `slang-fossil.h` (referenced
  from
  [slang-serialize-fossil.h](../../../source/slang/slang-serialize-fossil.h))
  and the chunk-code constants near the top of
  [slang-serialize-container.cpp](../../../source/slang/slang-serialize-container.cpp).
- The full backwards-compatibility policy, which lives in
  [../../design/backwards-compat-for-ir-modules.md](../../design/backwards-compat-for-ir-modules.md).
- The historical repro format. Treat it as removed.
