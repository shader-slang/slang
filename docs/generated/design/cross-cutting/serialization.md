---
generated: true
model: claude-sonnet-5-5
generated_at: 2026-11-03T08:34:07Z
source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
watched_paths_digest: e47a6f43c0eb2a5586546cd7f7a5b275cfa12cbb91caecfcd280aa2045cbc433
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

From the command line, `slangc <source> -o <output>.slang-module`
writes a container, and `-dump-module` and `-get-module-info` read one
back: the first prints the deserialized IR, the second the module's
name, its module version, and the version of the compiler that wrote
it. Those three fields are what `readSerializedModuleInfo` returns,
without deserializing the instruction graph; `compilerVersion` and
`name` are optional out-parameters, and an optional fourth
(`serializationVersion`) reports the payload's format version.
`-get-supported-module-versions` prints the module-version window
described under
[Versioning](#versioning-and-backwards-compatibility).

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

The same template runs in both directions, which rules out
"serialize" / "deserialize" drift by construction.
`SLANG_SCOPED_SERIALIZER_STRUCT` brackets a structural scope.

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
(referenced from the fossil header). The writer in
[slang-serialize-fossil.cpp](../../../../source/slang/slang-serialize-fossil.cpp)
emits the header magic, a total-blob-size field that is filled in when the
blob is laid out (`writeTotalBlobSize`), and the flags.
The validation cost is configurable via the macro
`SLANG_ENABLE_VALIDATION_FOSSIL`, defined with a fallback of 0 in
`slang-fossil.h` (not a watched path) and set from the CMake option of
the same name (`SLANG_ENABLE_VALIDATION_FOSSIL`, declared with no
default, so off unless set, in the top-level `CMakeLists.txt`).
When enabled, `SLANG_SERIALIZE_FOSSIL_VALIDATE` failures
call `SLANG_UNEXPECTED("invalid format encountered in serialized
data")`; when disabled the same conditions become plain
`SLANG_ASSERT`s. The toggle exists because validation costs
performance when loading the trusted core module.

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

Not every payload a `.slang-module` can carry is a chunk of its own.
The precompiled downstream artefact that `-embed-downstream-ir`
requests is not added to the RIFF hierarchy; it rides inside the IR
payload as an ordinary instruction. `EmbeddedDownstreamIR` carries the
target code as an integer literal and the artefact as a blob literal,
and it has a stable name like any other opcode
([slang-ir-insts-stable-names.lua](../../../../source/slang/slang-ir-insts-stable-names.lua)),
so it is written by the same `kIROp_StringLit` / `kIROp_BlobLit` case
of `serializeAsFlatModule` that handles string literals — the blob's
bytes land in `stringLengths` / `stringChars`. Reading the file back
with `-dump-module` therefore prints it inside the IR listing, as
`EmbeddedDownstreamIR(<targetCode> : Int, ...)`.

A former separate *RIFF serializer backend* (`slang-serialize-riff.{h,cpp}`)
was deleted; Fossil is the encoding for values, and RIFF survives only
as the container described above.

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

A worked example makes the shape of `operandIndices` concrete. Take a
module whose only content is the integer constant `42` of type `Int`.
Preorder traversal numbers the module instruction 0 and its children
after it — literals are moved to the end of the module's child list by
`kReorderInstructionsForSerialization` — giving:

| index | `instAllocInfo`            | `childCounts` | `operandIndices` slice |
| ----- | -------------------------- | ------------- | ---------------------- |
| 0     | `ModuleInst`, 0 operands   | 2             | `-1`                   |
| 1     | `IntType`, 0 operands      | 0             | `-1`                   |
| 2     | `IntLit`, 0 operands       | 0             | `1`                    |

`operandIndices` is the concatenation of those slices, `[-1, -1, 1]`.
Every instruction contributes its type-use slot, then its operands; the
reader recovers each run's end from `instAllocInfo[i].operandCount`.
The two `-1` entries are missing *types* (of the module instruction and
of `IntType`), encoded like a null operand. The value `42` is the single
entry of `literals`.

Because the flat tables come from a file that may be malformed,
`deserializeFromFlatModule` treats their relationships as
serialized invariants and validates them with `SLANG_RELEASE_ASSERT`
before dereferencing — for example that `childCounts` and `sourceLocs`
each have one entry per instruction, that every `operandIndices` read
stays in range and resolves to a valid instruction index
(`-1 <= index < numInsts`), that the running total of
`operandCount + 1` over the allocated instructions never exceeds the
size of `operandIndices` (checked before each `_allocateInst`, so an
inflated `operandCount` cannot size an operand array the file has no
data for), and that string-literal lengths are
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

The recursive rebuild and the write-side traversal share a recursion
budget, `kMaxIRSerializationDepth` (512) in
[slang-serialize-ir.h](../../../../source/slang/slang-serialize-ir.h),
asserted on each level so an adversarial module fails fast instead of
overflowing the C++ stack.

## Source-location serialization

Source locations are tricky to round-trip because the integer encoding
in
[slang-source-loc.h](../../../../source/compiler-core/slang-source-loc.h)
is meaningful only relative to the live `SourceManager` of the
session that produced it. The serializer therefore captures, per
contributing source file, its path, its source-location range, its
total line count, and one line-start record for each line a serialized
location actually reaches, alongside the integer locations. The reader
rebuilds the full line-break array from those records, and
reconstructs each file into a fresh
`SourceManager` on load with `createSourceFileWithSize` (a
placeholder-sized file plus a single view — the file's content is not
serialized).

A reached line lands in exactly one of two lists, chosen by whether a
`#line` directive was in effect there: `SerialSourceLocWriter::addSourceLoc`
adds a plain `LineInfo` (physical line index plus start offset) when no
view entry covers the location, and an `AdjustedLineInfo` (that physical
`LineInfo` plus the remapped line index and overridden path) when one
does. The reader takes the *physical* `LineInfo` out of both lists to
rebuild the line-break array, then turns each `AdjustedLineInfo` back
into a `SourceView::Entry` whose `m_lineAdjust` is the remapped index
minus the physical one, so a reloaded location resolves exactly as in
the writing session; the remapping is never baked into the line table.

A source-manager query takes a `SourceLocType` that selects the
physical or the `#line`-adjusted view; that parameter, not the
serialized data, decides which a consumer sees. See
[../syntax-reference/tokens.md#source-location](../syntax-reference/tokens.md#source-location).

What a consumer does with the location is outside this document. Note
that whether a `#line` directive appears in emitted text, and in which
spelling, is a compile option rather than a property of the serialized
locations: `-line-directive-mode`
([slang-options.cpp](../../../../source/slang/slang-options.cpp) line
879) takes `none`, `source-map`, `default`, `standard`, or `glsl`. The
per-target rendering is documented in
[../pipeline/06-emit.md](../pipeline/06-emit.md) and the
[../target-pipelines/](../target-pipelines) pages.

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
it against `kSupportedSerializationVersion` (currently `2`) and returns
`SLANG_FAIL` on a mismatch, with a comment marking that comparison as
the place a future multi-version reader would branch. The full design is
described in
[../../../design/backwards-compat-for-ir-modules.md](../../../design/backwards-compat-for-ir-modules.md).

Two different version numbers are in play. `IRModuleInfo::serializationVersion`
versions the *fossil schema* of the payload. `IRModule::m_version` is
serialized next to the module's name in `handleIRModule` and versions the
module's IR semantics rather than its encoding; it is what
`-get-module-info` prints as `Module Version`. The window
`-get-supported-module-versions` prints is
`IRModule::k_minSupportedModuleVersion` to `k_maxSupportedModuleVersion`
(both `37` at this commit), declared in `slang-ir.h` together with
`IRModule::isModuleVersionSupported`, an inclusive range test. The
comment there says adding an IR instruction requires a new writer
version, and that earlier modules stay supported only while their AST and
IR representations remain compatible with the reader.

Both numbers are now checked on the read path, in different places:

- `readSerializedModuleInfo` (metadata only) returns
  `SLANG_E_NOT_AVAILABLE` when `serializationVersion` is not the
  supported value, after writing it to the optional
  `serializationVersion` out-parameter so the caller can report it; a null
  module pointer returns `SLANG_FAIL`. It does not check `m_version`.
- `readSerializedModuleIR_` returns `SLANG_FAIL` for an unsupported
  `serializationVersion`, a null module pointer, or an `m_version` outside
  the window (`isModuleVersionSupported`).
- The module-loading paths in `slang-session.cpp` (outside the watched
  paths) call `readSerializedModuleInfo` first, diagnose
  `UnsupportedSerializedModuleFormatVersion` on `SLANG_E_NOT_AVAILABLE`
  and `UnsupportedSerializedModuleVersion` when `isModuleVersionSupported`
  fails, then deserialize.

The `Unrecognized` opcode that appears at the head of
[slang-ir-insts.lua](../../../../source/slang/slang-ir-insts.lua) plays
a role here: when `getStableNameOpcode` yields `kIROp_Invalid` — the
module was written by a newer compiler that knows a stable name this
one does not — the reader substitutes `kIROp_Unrecognized` and sets
`_foundUnrecognizedInstructions`, which turns into the recoverable
`SLANG_FAIL` described above. 
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
   the generic `FlatInstTable`, which records each instruction's opcode,
   operand count, and operands uniformly. Ordinary new operands,
   decorations, and instructions fit that schema with no serializer edit.
2. The reader accepts exactly one serialization version and fails
   otherwise, so there are no per-field version gates. Only a change to
   the encoding itself or to a special payload representation (the flat
   tables, literals, strings) means bumping
   `kSupportedSerializationVersion`. A new IR instruction instead means
   moving `IRModule::k_maxSupportedModuleVersion`
   (and the minimum, if older modules would decode differently), plus
   updating the compatibility story (see
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
- Repro format details are outside this page's watched paths.

## Manifest coverage

The serialization drivers, the types/chunk-code files, the opcode
stable-name files, and `source/core/slang-riff.{h,cpp}` are in the
manifest's `watched_paths`, so a change to them marks this page stale.
Some behavior described here lives in unwatched files, so changes
confined to them do not trigger freshness tracking: `slang-fossil.h`
(Fossil validation and format layout), `slang-relative-ptr.h`, and
`slang-session.cpp` (module-loading version checks). The deleted
`slang-serialize-riff.{h,cpp}` are no longer listed.
