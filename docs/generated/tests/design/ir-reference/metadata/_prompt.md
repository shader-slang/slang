# Prompt: docs/generated/tests/design/ir-reference/metadata/

See [`_common.md`](../../../_meta/prompts/_common.md) for universal rules and
[`_claims.md`](../../../_meta/prompts/_claims.md) for the claim methodology.
Those rules apply to this bundle and override nothing here unless explicitly
noted.

## Target

Produce the test bundle at `docs/generated/tests/design/ir-reference/metadata/`,
anchored to
[`docs/generated/design/ir-reference/metadata.md`](../../../../design/ir-reference/metadata.md).

Audience: nightly CI. The bundle exercises the four IR families whose
instructions carry metadata _about_ other instructions rather than computing a
value: the `Layout` opcodes, the `Attr` opcodes, the `Debug*` opcodes, and the
`SPIRVAsmOperand` opcodes.

The doc's own framing is the organizing principle of the bundle: these opcodes
are **records, so operand position is the entire meaning**. A test that merely
asserts "the mnemonic appears somewhere in the dump" cannot detect the failure
mode the doc warns about — an operand that moves, gains a neighbour, or changes
kind. Write tests that capture the operand ids of one record and then follow
each captured id to its defining instruction.

## The translation rule: claims to observations

Every opcode on this page is created either by AST-to-IR lowering (the layout,
attribute and inline-asm families) or by an early IR pass (the remaining
`Debug*` records). All of them are therefore present in the platform-neutral
IR snapshot, and the observation point for nearly every test is:

```
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute
```

`-o /dev/null` keeps target code off stdout so the dump is the only output;
`-target` is required or compilation stops before the dump is produced.

Records that a pass creates rather than lowering are read from that pass's own
snapshot instead, e.g. `-dump-ir-after performForceInlining` for the inlining
records, so the text being checked is the one the producing pass just wrote.

### Debug level selects which records exist

This is the single most important operational fact for this bundle, and it is
now stated in the doc:

- `-g1` emits no `DebugCompilationUnit`, `DebugVar` or `DebugValue` at all.
- `-g` / `-g2` / `-g3` emit all of them.
- `DebugBuildIdentifier` appears **only** when separate debug information is
  requested (`-separate-debug-info`), not on an ordinary `-g` compile.

`DebugSource` embeds the **entire text of the compiled file** as its second
operand. At full `-g` the dump therefore contains a verbatim copy of the test's
own `CHECK` lines, and a pattern can silently match itself. Any test whose
patterns would be ambiguous under that echo must use `-g1`, which keeps every
record it needs while leaving the source-text operand empty.

### Boundary pairs for optional operands

The doc names three optional operands. An optional operand is only
distinguishable from a fixed one by the _pair_ of observations, so each gets one
test for the present form and one for the absent form:

| Optional operand              | Present form          | Absent form    |
| ----------------------------- | --------------------- | -------------- |
| `offset`'s register space     | non-zero space        | space zero     |
| `TypeAlignment`'s layout unit | non-byte unit         | byte default   |
| `DebugVar`'s argument index   | entry-point parameter | ordinary local |

### Observable claims (write tests for these)

Grouped by the anchor each `doc_ref` must resolve to.

**`#layout` / `#layout-family`**

- A laid-out instruction reaches its layout through a layout decoration whose
  operand is a concrete `Layout` child.
- `cbuffer X { … }` lowers to a `parameterGroupTypeLayout` whose first three
  operands are the container var layout, the element var layout, and the offset
  element type layout.
- A structured-buffer parameter lowers to a `structuredBufferTypeLayout` whose
  operand 0 is the element type layout.
- An array lowers to an `arrayTypeLayout` storing the element type layout in
  operand 0, **deriving** the element stride rather than storing it.
- A `structTypeLayout` carries one `structFieldLayout` attribute per field, in
  field declaration order.
- An interface-typed field of a laid-out struct produces an
  `existentialTypeLayout`.
- A pointer-typed field lowers to a `ptrTypeLayout` that stores only attributes
  — deliberately **not** a pointee layout.
- A geometry entry point with an `inout TriangleStream` parameter produces a
  `streamOutputTypeLayout` whose operand 0 is the element type layout.
- `matrixTypeLayout` stores a `MatrixLayoutMode` integer in operand 0 that
  changes with `-matrix-layout-row-major` / `-matrix-layout-column-major`.
- Layout records are hoistable: two distinct layouts needing the same attribute
  value share one attribute instruction.
- The generic `typeLayout` opcode carries only attribute operands, with no fixed
  layout operand ahead of them.

**`#varlayout-and-entrypointlayout`**

- A `varLayout` has one fixed operand (the type layout) followed by a
  variable-length tail of attributes.
- An `EntryPointLayout` has exactly two fixed operands and **no** attribute
  tail: the parameters layout and the result layout, both `varLayout`s.

**`#size-and-offset`**

- A `size` attribute puts the `LayoutResourceKind` in operand 0 and the size in
  operand 1, so one type layout can carry a separate size per kind.
- An `offset` attribute gains a third operand holding the register space when
  that space is non-zero, and omits it when the space is zero.
- An unbounded resource array records a non-finite extent in the raw
  `LayoutSize` operand rather than a finite count.

**`#typealignment--the-operand-order-exception`**

- `TypeAlignment` reads the alignment from operand 0 and omits the layout-unit
  operand for the byte default — the exception to the kind-first order that
  `size` and `offset` follow.
- A type layout emits every `size` attribute before any `TypeAlignment`, so each
  attribute kind forms one contiguous run of operands.
- An absent `TypeAlignment` encodes alignment 1: the identity alignment is never
  emitted.

**`#attr-family`**

- `structFieldLayout` stores the field key in operand 0 and the field's
  `varLayout` in operand 1.
- `Aligned` records the access alignment of a load or store, appearing in the
  **operand tail of the access** rather than in a type layout. Reach it through
  the public core-module `loadAligned` / `storeAligned` wrappers.
- `MemoryScope` records the memory scope of a coherent store, sitting after the
  `Aligned` attribute in the store's operand tail. Reach it through
  `loadCoherent` / `storeCoherent`; the coherent pair additionally requires the
  Vulkan memory-model capability.
- `no_diff` on a parameter type produces a no-operand `no_diff` attribute.
- `snorm` / `unorm` on a texture or structured-buffer element type produce
  no-operand attributes inside an `Attributed` type wrapper.
- The `stage` attribute tags an entry-point `varLayout` with a single
  pipeline-stage integer indexing the `Stage` enumeration.

**`#usersemantic-vs-systemvaluesemantic`**

- A user-written semantic lowers to a `userSemantic` whose operand 0 is the name
  string and operand 1 the semantic index.
- A `varLayout` carries at most one semantic attribute: an `SV_` parameter gets a
  `systemValueSemantic` and **no** `userSemantic` is created for it (negative).

**`#debug-info-family` / `#debugline` / `#debugscope` / `#debugvar`**

- `DebugCompilationUnit` declares the unit with a single operand referencing a
  `DebugSource`.
- `DebugFunction` declares a function with its name, line, column, source file
  and function type; the owning function links to it by decoration. At full `-g`
  it carries a trailing `DebugCompilationUnit` operand; the five-operand form
  appears only at `-g1`, where no compilation unit exists.
- `DebugSource` records path, embedded text, and an included-file flag.
- `DebugLine` is an ordinary instruction in the block's stream rather than a
  decoration, so the location travels with position in the block; it pins an
  instruction to a source range with five operands (file, start/end line,
  start/end column).
- `DebugScope` operand 0 references the enclosing scope (a `DebugFunction` for a
  function-level scope) and operand 1 records the inlining context.
  `DebugNoScope` is emitted with zero operands.
- `DebugVar` for an entry-point parameter carries the optional argument-index
  operand after source, line and column; for an ordinary local it omits that
  operand, and the variable's own type is the **pointee of the instruction's
  pointer result type**, not an operand.
- `DebugValue` reports the current value of a `DebugVar`: declaration in operand
  0, value in operand 1.
- `DebugInlinedAt` records one frame of an inlining chain (line, column, source
  file, the debug function inlined into), with the outer-frame operand absent at
  the outermost frame.
- No `DebugInlinedVariable` opcode is produced anywhere, even when a call is
  inlined under debug info (negative).
- `DebugBuildIdentifier` records the build identifier together with a flags
  operand, only under separate debug information.

**`#spir-v-inline-asm` / `#spirvasmoperand` / `#sampledtype--imagetype--sampledimagetype`**

- A `spirv_asm` block lowers to a `SPIRVAsm` parent owning `SPIRVAsmInst`
  children, each taking its SPIR-V opcode as operand 0.
- Each token becomes a typed operand instruction: the result marker and a named
  id are distinct operand kinds carried in the `SPIRVAsmInst` operand list.
- A `builtin(...)` token becomes `SPIRVAsmOperandBuiltinVar` carrying the
  built-in kind; `glsl450` becomes a dedicated operand referencing the
  GLSL.std.450 instruction set.
- `__truncate` is a pseudo-opcode accepted in the opcode operand;
  `__sampledType` computes the result type of sampling an image of a given
  component type.
- `__imageType` / `__sampledImageType` are type functions whose operand is a
  value in scope, so the SPIR-V type is computed at emit time rather than stored
  in the IR.

**`#tuplefieldlayout-and-caselayout`**

- The dormant tuple and union layout opcodes (`tupleTypeLayout`,
  `tupleFieldLayout`, `caseLayout`) have no producer in the compiler, so none
  appears in any IR dump (negative).

### Not testable here (record under `## Untested claims`)

- **`nonuniform`** — built only as part of a specialization cache key and never
  present in a dumped module. `NonUniformResourceIndex(i)` over a bindless array
  yields a `nonUniformResourceIndex` _instruction_, not the attribute.
- **`FuncThrowType`** — a `throws` function already shows a `Result(T, E)` return
  type in the first available dump section, so the attribute is consumed before
  any observable snapshot.
- **`EmbeddedDownstreamIR`** — needs a second file precompiled to a
  `.slang-module` with embedded downstream IR plus a two-step `slangc`
  invocation, which one `//TEST` directive cannot express.
- **`DebugSource`'s include flag in its `true` form** — needs a second file to
  `#include` / `__include`. The `false` form is covered.
- **`SPIRVAsmOperandInst` non-hoistability** — hoistability is only observable as
  deduplication, and the dump prints these operands inline inside their
  `SPIRVAsmInst` line rather than as separately numbered instructions.
- **The remaining inline-asm operand kinds** — `SPIRVAsmOperandLiteral` and
  `SPIRVAsmOperandEnum` print as bare integers indistinguishable from each
  other; the ray-payload / hit-attribute / callable kinds need ray-tracing entry
  points with the matching Vulkan capability set.
- **The abstract grouping entries** (`Layout`, `TypeLayout`, `Attr`,
  `SemanticAttr`, `LayoutResourceInfoAttr`, `SPIRVAsmOperand`) — the claim is
  about the shape of a C++ range check; the user-visible part is implied by every
  test naming a concrete opcode.
- **Hand-written vs FIDDLE-generated wrapper structs** — a property of the
  compiler's own headers with no consequence a compiled shader can reveal.
- **The `## Manifest coverage` watched-path statement** — about this
  documentation set's staleness tracking, not compiler behavior.

## Required structure

1. `README.md` with the structure named in `_common.md`.
2. 40 to 60 `.slang` files. The bundle sits at its manifest `size_cap_files`;
   raise the cap in `_meta/manifest.yaml` in the same change if new claims push
   past it.

## Doc sources

Primary (every `doc_ref` resolves into this):

- `docs/generated/design/ir-reference/metadata.md`

Secondary (allowed citations; only where the primary doc hands off):

- `docs/generated/design/cross-cutting/ir-instructions.md`
- `docs/generated/design/ir-reference/decorations.md`
- `docs/generated/design/pipeline/04-ast-to-ir.md`

If you would cite anything else, stop and record a doc-gap finding in
`README.md` instead.

## Source files you may consult for _verification only_

Use these to confirm a printed operand order or mnemonic. Do **not** mine them
for claims the doc does not state.

- `source/slang/slang-ir-insts.lua`
- `source/slang/slang-ir-insts.h`
- `source/slang/slang-ir-insts-info.cpp`
- `source/slang/slang-ir.h`, `source/slang/slang-ir.cpp`
- `source/slang/slang-lower-to-ir.cpp`

## Test directives

Default:

```
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute
```

Variants this bundle legitimately needs:

| Need                                                            | Directive suffix                                           |
| --------------------------------------------------------------- | ---------------------------------------------------------- |
| Debug records without the source-text echo                      | `-g1`                                                      |
| Full debug records (`DebugVar`, `DebugValue`, compilation unit) | `-g`                                                       |
| `DebugBuildIdentifier`                                          | `-g -separate-debug-info`                                  |
| Records created by inlining                                     | `-dump-ir-after performForceInlining`                      |
| Matrix layout mode                                              | `-matrix-layout-row-major` / `-matrix-layout-column-major` |
| Stream-output layout                                            | `-stage geometry`                                          |
| Semantics on a rasterizer stage                                 | `-stage fragment`                                          |

## Lessons captured for this bundle

- **Follow the id, don't match the mnemonic.** Capture the operand id with a
  FileCheck variable and match its defining instruction on a later line. A bare
  mnemonic match passes even when the record is malformed.
- **`-g` echoes the source file into the dump.** `DebugSource` operand 1 is the
  whole file, so `CHECK` lines can match themselves. Use `-g1` whenever the
  pattern would otherwise be ambiguous.
- **A non-finite extent prints as `-1`.** An unbounded array's `size` attribute
  reads `size(3 : Int, -1 : Int)`; that is the encoding, not a negative count.
- **`matrixTypeLayout` mode is 1 for row-major, 2 for column-major.**
- **Inline-asm operands print inline.** `SPIRVAsmInst(132 : UInt, Int, result,
param, param)` — the operand instructions are not separately numbered, so a
  literal and an enumerator are indistinguishable in dump text.
- **A named id prints as `%"name"`** inside an asm block.
- **Reach `Aligned` / `MemoryScope` through the public wrappers**
  (`loadAligned` / `storeAligned`, `loadCoherent` / `storeCoherent`), not the
  internal `__align_attr` / `__memoryscope_attr` intrinsics.
- **Keep values live.** DCE removes records whose owner is dead; write results
  to a buffer so the laid-out or annotated instruction survives to the dump.

## Quality checklist (in addition to `_common.md`'s)

- [ ] Every `doc_ref` resolves to an anchor in `ir-reference/metadata.md` (or a
      listed secondary doc), and every `doc_section_digest` is current.
- [ ] Every test follows at least one captured operand id to its definition,
      rather than asserting a mnemonic appears.
- [ ] Each optional operand has both a present-form and an absent-form test.
- [ ] Tests using full `-g` are checked for self-matching against the echoed
      source text; prefer `-g1` where the pattern allows.
- [ ] No test depends on a GPU: compute (or geometry/fragment) entry points
      compiled to text, never executed.
- [ ] `## Untested claims` enumerates every opcode on the page with no
      dump-observable form, each with a reason.
- [ ] `## Doc gaps observed` records claims lacking a checkable marker.
