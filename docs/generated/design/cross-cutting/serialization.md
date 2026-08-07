---
generated: true
model: claude-opus-5
generated_at: 2026-08-03T13:18:21Z
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
watched_paths_digest: f1411119c2984cf871fda3e87109caf5abb8a34836f05a251561d7792998a19a
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
  [slang-serialize-ast.h](../../../../source/slang/slang-serialize-ast.h)
  /
  [slang-serialize-ast.cpp](../../../../source/slang/slang-serialize-ast.cpp).
  The serialized form preserves the checked AST that backs an
  `import`-able module. Most enum-valued AST fields are handled by a
  FIDDLE template in
  [slang-serialize-ast.cpp](../../../../source/slang/slang-serialize-ast.cpp)
  that generates a `serialize(...)` overload (delegating to
  `serializeEnum`, which encodes the value as a `FossilUInt`) for each
  name in a single `enumTypeNames` list. Serializing a new AST enum is
  usually a matter of appending its type name to that list rather than
  writing a bespoke `serialize` function.
- **IR modules** — handled by
  [slang-serialize-ir.h](../../../../source/slang/slang-serialize-ir.h)
  /
  [slang-serialize-ir.cpp](../../../../source/slang/slang-serialize-ir.cpp),
  with the IR-specific type vocabulary in
  [slang-serialize-ir-types.h](../../../../source/slang/slang-serialize-ir-types.h)
  /
  [slang-serialize-ir-types.cpp](../../../../source/slang/slang-serialize-ir-types.cpp).
- **Containers** — the on-disk packaging that bundles AST + IR + side
  data into a single file (the `.slang-module` format and other
  artefacts). Driver:
  [slang-serialize-container.h](../../../../source/slang/slang-serialize-container.h)
  /
  [slang-serialize-container.cpp](../../../../source/slang/slang-serialize-container.cpp).
  The underlying chunked file format is the general-purpose RIFF
  reader/writer in
  [slang-riff.h](../../../../source/core/slang-riff.h) /
  [slang-riff.cpp](../../../../source/core/slang-riff.cpp), which is a
  `source/core` facility rather than a serialization-specific one.

Source-location streams have their own helper file
([slang-serialize-source-loc.h](../../../../source/slang/slang-serialize-source-loc.h)
/
[slang-serialize-source-loc.cpp](../../../../source/slang/slang-serialize-source-loc.cpp)),
because preserving readable diagnostics across deserialization
requires re-establishing the `SourceManager`'s view of files and
expansions.

## Backends

### Generic serialize

The generic interface is
[slang-serialize.h](../../../../source/slang/slang-serialize.h). The
preamble of that header captures the central design choice: a single
`serialize(serializer, value)` function handles both reading and
writing, distinguished by a `SerializationMode` carried on the
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

A backend is described by the `ISerializerImpl` interface declared in
[slang-serialize.h](../../../../source/slang/slang-serialize.h). That
interface lists the `handleBool` / `handleInt32` / `handleString` /
structural-scope operations a backend must provide, but its own comment
is explicit that it is a specification rather than a required base
class: implementations "do *not* need to inherit from this type; it
currently serves only to define the requirements". Callers pair a
concrete backend with a context type through the `Serializer<Backend,
Context>` template, so the common case dispatches statically.

The generic machinery is header-only: the companion
[slang-serialize.cpp](../../../../source/slang/slang-serialize.cpp)
holds no definitions, and the sibling
[slang-serialize-types.h](../../../../source/slang/slang-serialize-types.h)
/
[slang-serialize-types.cpp](../../../../source/slang/slang-serialize-types.cpp)
pair carries the shared container vocabulary rather than value
encoders: the RIFF chunk codes (`SerialBinary`, `PropertyKeys<Module>`,
`PropertyKeys<IRModule>`) and the `SerialStringTableUtil` string-table
encoder/decoder.

Fossil is the sole concrete backend at this commit.

### Fossil backend

[slang-serialize-fossil.h](../../../../source/slang/slang-serialize-fossil.h)
/
[slang-serialize-fossil.cpp](../../../../source/slang/slang-serialize-fossil.cpp).
"Fossil" is a memory-mappable binary format defined in `slang-fossil.h`
(referenced from the fossil header). Per the comments in
[slang-fossil.h](../../../../source/slang/slang-fossil.h):

> Deserializing data is an important place where security issues can
> arise, so it is usually important to perform validation checks
> throughout the process, and fail fast rather than risk reading
> mal-formed data.

The validation cost is configurable via the macro
`SLANG_ENABLE_VALIDATION_FOSSIL` (a compile-time define, default 0),
which is set by the CMake option of the same name. When enabled,
validation failures call
`SLANG_UNEXPECTED("invalid format encountered in serialized data")`;
when the define is 0 the same conditions become plain `SLANG_ASSERT`s,
and the walk that checks the whole object graph on load
([slang-fossil-validate.cpp](../../../../source/slang/slang-fossil-validate.cpp))
is compiled out entirely. Validation is off by default because it is
expensive, and because a key serialization path — loading the core
module from the `slang.dll` binary — reads data that is already
trusted; validating the core module costs roughly two seconds per
process in a release build. Builds that load untrusted
`.slang-module` data should turn the option on.

The format is designed for **memory-mapped** deserialization: a pointer
in the serialized data is a 32-bit offset relative to the address of
the pointer itself (`slang-relative-ptr.h`), so a fossilized object
graph can be traversed directly once the file is mapped, with no
pointer-fixup pass.

Both module payloads go through this backend. `writeSerializedModuleAST`
in
[slang-serialize-ast.cpp](../../../../source/slang/slang-serialize-ast.cpp)
and `writeSerializedModuleIR` in
[slang-serialize-ir.cpp](../../../../source/slang/slang-serialize-ir.cpp)
each build a `Fossil::SerialWriter` over a `BlobBuilder` and then hand
the finished blob to `RIFF::BuildCursor::addDataChunk`, so a fossil blob
is what a RIFF data chunk contains. The IR side names its serializer
types `Serializer<Fossil::SerialWriter, IRSerialWriteContext>` and
`Serializer<Fossil::SerialReader, IRSerialReadContext>` — bound to the
concrete backend rather than to `ISerializerImpl` — with the comment
that this is done "to avoid some virtual function calls".

## RIFF container format

The chunked, tagged container is the general-purpose RIFF
implementation in
[slang-riff.h](../../../../source/core/slang-riff.h) /
[slang-riff.cpp](../../../../source/core/slang-riff.cpp): each chunk
has a `FourCC` four-character code, a length, and a payload, and
`RIFF::ListChunk` chunks nest. Writers build chunks with
`RIFF::Builder` / `RIFF::BuildCursor`; readers navigate with
`RIFF::RootChunk::getFromBlob` and the `findListChunk` /
`findDataChunk` helpers.

Container-level orchestration in
[slang-serialize-container.cpp](../../../../source/slang/slang-serialize-container.cpp)
composes a serialized AST module, a serialized IR module, and the
auxiliary data (source-location stream, file-dependency list, entry
points, digest) into one RIFF hierarchy. The chunk codes are the
constants in
[slang-serialize-types.h](../../../../source/slang/slang-serialize-types.h)
— for example `SerialBinary::kModuleFourCC` (`'smod'`) for a module
list chunk, `PropertyKeys<IRModule>::IRModule` (`'ir  '`) and
`PropertyKeys<Module>::ASTModule` (`'ast '`) for the two payloads, and
`PropertyKeys<Module>::FileDependencies` (`'fdep'`). The reader side of
those codes is the `ModuleChunk` / `ContainerChunk` / `DebugChunk`
accessor types in
[slang-serialize-container.h](../../../../source/slang/slang-serialize-container.h).

The RIFF wrapping is what allows tools to inspect partial structure
of a `.slang-module` file (chunk types, sizes) without parsing the
inner serialized content — useful for sanity checks and recovery.

A separate *RIFF serializer backend* — an `ISerializerImpl` that wrote
each value as its own chunk, in files named `slang-serialize-riff.h` /
`slang-serialize-riff.cpp` — used to sit alongside the Fossil backend.
It was deleted (commit `52cb4e12e`) because its only remaining callers
were branches of a hard-coded-off `USE_RIFF` switch in
[slang-serialize-ir.cpp](../../../../source/slang/slang-serialize-ir.cpp),
so no build ever compiled it. Nothing replaced it: Fossil is the
encoding for values, and RIFF survives only as the container described
above. Earlier revisions of this page linked those two files; they no
longer exist.

## IR flat-module read path

[slang-serialize-ir.cpp](../../../../source/slang/slang-serialize-ir.cpp)
does not serialize the IR object graph directly. `serializeAsFlatModule`
first flattens a module into the parallel arrays of `FlatInstTable` —
one entry per instruction, walked in preorder by
`traverseInstsInSerializationOrder` in
[slang-serialize-ir.h](../../../../source/slang/slang-serialize-ir.h):
`instAllocInfo`, `childCounts`, `sourceLocs`, plus a single
`operandIndices` list (type-use slot followed by each operand, with a
`nullptr` operand encoded as `-1`), `stringLengths`, the concatenated
`stringChars`, and a `literals` list holding the raw bits of every
bool / integer / float / pointer constant. On load,
`deserializeFromFlatModule`
allocates every instruction up front and then a recursive lambda
rebuilds the parent/child links and resolves operand pointers by
indexing back into the allocated `insts` array.

Because the flat tables come from a file that may be malformed,
`deserializeFromFlatModule` treats their relationships as
serialized invariants and validates them with `SLANG_RELEASE_ASSERT`
before dereferencing — for example that `childCounts` and `sourceLocs`
each have one entry per instruction, that every `operandIndices` read
stays in range and resolves to a valid instruction index
(`-1 <= index < numInsts`), and that string-literal lengths are
non-negative, fit in `uint32_t`, and do not run past the end of
`stringChars`. After the walk it asserts that every flat table was
fully consumed (`instIndex == numInsts`,
`operandIndex == operandIndicesCount`, and the literal/string cursors
reach the end of their lists) and that the root is an `IRModuleInst`.
The literal/string-consumption check is skipped when
`readContext._foundUnrecognizedInstructions` is set, because the reader
cannot know whether a future unknown opcode (mapped to `Unrecognized`,
see [Versioning](#versioning-and-backwards-compatibility)) would have
consumed literal or string payloads. That case instead becomes a
recoverable failure one level up: `readSerializedModuleIR_` checks the
same flag after deserializing and returns `SLANG_FAIL`, so the caller in
[slang-serialize-container.cpp](../../../../source/slang/slang-serialize-container.cpp)
sees an unloadable module rather than a crash.

The recursive rebuild and the matching write-side traversal share a
fixed recursion budget, `kMaxIRSerializationDepth` (512) declared in
[slang-serialize-ir.h](../../../../source/slang/slang-serialize-ir.h),
asserted on each level of `go`/`traverseInstsInSerializationOrder` so a
deeply-nested or adversarial module fails fast instead of overflowing
the C++ stack. Keeping the bound on both sides keeps round-trips
symmetric.

## Source-location serialization

Source locations are tricky to round-trip because the integer encoding
in
[slang-source-loc.h](../../../../source/compiler-core/slang-source-loc.h)
is meaningful only relative to the live `SourceManager` of the
session that produced it. The serializer therefore captures, per
contributing source file, its path, its source-location range, and
line-start records (both unadjusted and `#line`-adjusted) for just the
lines a serialized location actually reaches, plus the file's total
line count, alongside the integer locations. The reader rebuilds the
full line-break array from those records, and reconstructs each file
into a fresh
`SourceManager` on load with `createSourceFileWithSize` (a
placeholder-sized file plus a single view — the file's content is not
serialized).

Driver: [slang-serialize-source-loc.cpp](../../../../source/slang/slang-serialize-source-loc.cpp).

## Versioning and backwards compatibility

Modules are expected to remain loadable by newer compilers, even
though new IR opcodes are added over time. The constraint that makes
this possible is at the top of
[slang-ir-insts.lua](../../../../source/slang/slang-ir-insts.lua):

> Please make sure to update the supported module versions in
> Slang::IRModule accordingly when modifying this file.

Two mechanisms in the watched paths implement that. First, an opcode is
never written as its `kIROp_*` enum value. The `serialize(S const&,
IROp&)` overload in
[slang-serialize-ir.cpp](../../../../source/slang/slang-serialize-ir.cpp)
converts through `getOpcodeStableName` on write and
`getStableNameOpcode` on read
(declared in
[slang-ir-insts-stable-names.h](../../../../source/slang/slang-ir-insts-stable-names.h)
and implemented in
[slang-ir-insts-stable-names.cpp](../../../../source/slang/slang-ir-insts-stable-names.cpp),
which owns both the opcode-to-stable-name and stable-name-to-opcode
tables; those tables are generated from
[slang-ir-insts-stable-names.lua](../../../../source/slang/slang-ir-insts-stable-names.lua)),
so inserting or reordering entries in
[slang-ir-insts.lua](../../../../source/slang/slang-ir-insts.lua) does
not renumber what is already on disk. Second, the whole payload carries
`IRModuleInfo::serializationVersion`; `readSerializedModuleIR_` compares
it against `kSupportedSerializationVersion` (currently `1`) and returns
`SLANG_FAIL` on a mismatch, with a comment marking that comparison as
the place a future multi-version reader would branch. The full design is
described in
[../../../design/backwards-compat-for-ir-modules.md](../../../design/backwards-compat-for-ir-modules.md).

The `Unrecognized` opcode that appears at the head of
[slang-ir-insts.lua](../../../../source/slang/slang-ir-insts.lua) plays
a role here: when `getStableNameOpcode` yields `kIROp_Invalid` — the
module was written by a newer compiler that knows a stable name this
one does not — the reader substitutes `kIROp_Unrecognized` and sets
`_foundUnrecognizedInstructions`, which turns into the recoverable
`SLANG_FAIL` described above. The Lua comment requires that
`Unrecognized` never survive past deserialization.

## Round-trip and repro files

The repro machinery captures a single `slangc` invocation as a
serialized blob. [CLAUDE.md](../../../../CLAUDE.md) lists
`-dump-repro` among the options not to use, but keeps `-load-repro`
and `-extract-repro` as specialized tools for work on repro handling
itself; neither is a general-purpose round-trip workflow. The
watched-paths set for this document does not include the repro
implementation, so the format itself is out of scope for this page.

## Adding a new serialized field

The AST and IR paths differ, and neither is a hand-edited per-field
`serialize(...)` call with a version gate.

For an **AST** field:

1. Add the field to its host node type as usual. FIDDLE generates both
   the `Fossilized_<T>` layout and the `serialize(S const&, T&)`
   function by iterating the type's `directFields`, so a field that
   participates in FIDDLE metadata is picked up with no serializer
   edit at all.
2. Only a genuinely hand-written special-type serializer (a type whose
   serialization is not FIDDLE-generated) needs a manual visit.
3. If the field is a pointer to an externally-owned object, ensure the
   target type is itself serializable.

For an **IR** change:

1. IR serialization does not walk per-instruction C++ fields; it writes
   the fixed `FlatInstTable` schema. Adding information therefore means
   changing that schema, not adding a field visit.
2. The reader accepts exactly one serialization version and fails
   otherwise, so there are no per-field version gates; a schema change
   means bumping the serialization version and updating the
   compatibility story (see
   [../../../design/backwards-compat-for-ir-modules.md](../../../design/backwards-compat-for-ir-modules.md)).

Either way, exercise the round-trip with the serialization unit tests,
such as
[tools/slang-unit-test/unit-test-ir-blob.cpp](../../../../tools/slang-unit-test/unit-test-ir-blob.cpp).

## What is not in this document

- The format-level layout of a fossil value or a RIFF chunk. The
  authoritative descriptions are in `slang-fossil.h` (referenced
  from
  [slang-serialize-fossil.h](../../../../source/slang/slang-serialize-fossil.h))
  and [slang-riff.h](../../../../source/core/slang-riff.h); the
  chunk-code constants live in
  [slang-serialize-types.h](../../../../source/slang/slang-serialize-types.h),
  not in the container `.cpp` that uses them.
- The full backwards-compatibility policy, which lives in
  [../../../design/backwards-compat-for-ir-modules.md](../../../design/backwards-compat-for-ir-modules.md).
- The historical repro format. Treat it as removed.

## Manifest coverage

Every source file this page cites for serialization behavior is inside
the manifest's `watched_paths` for it, so a change to any of them marks
this page stale. That covers:

- [source/slang/slang-serialize-types.h](../../../../source/slang/slang-serialize-types.h)
  and
  [source/slang/slang-serialize-types.cpp](../../../../source/slang/slang-serialize-types.cpp)
  — the chunk codes and the string-table encoder.
- [source/slang/slang-ir-insts-stable-names.h](../../../../source/slang/slang-ir-insts-stable-names.h),
  [source/slang/slang-ir-insts-stable-names.cpp](../../../../source/slang/slang-ir-insts-stable-names.cpp),
  and
  [source/slang/slang-ir-insts-stable-names.lua](../../../../source/slang/slang-ir-insts-stable-names.lua)
  — the opcode↔stable-name mapping the IR reader depends on, its
  implementation, and its generator input.
- [source/core/slang-riff.h](../../../../source/core/slang-riff.h) and
  [source/core/slang-riff.cpp](../../../../source/core/slang-riff.cpp)
  — the container implementation. The manifest no longer lists the
  deleted `slang-serialize-riff.{h,cpp}`.
