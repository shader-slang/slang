---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-20T17:50:00Z
source_commit: 2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
watched_paths_digest: 4efe93afbd22f4572d6d334ca82947cebf8058c7572291261103fd18aa04f6bd
source_doc: docs/llm-generated/ir-reference/metadata.md
source_doc_digest: 3216251c72624b474409742713e67c3562882361cc1f104f8f3fda2ef68cc6c0
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for ir-reference/metadata

## Intent

Tests verify the per-opcode catalog of the IR *metadata* families
described in
[`docs/llm-generated/ir-reference/metadata.md`](../../../docs/llm-generated/ir-reference/metadata.md):
that the Layout, Attr, Debug-info, and SPIRVAsmOperand opcodes
surface in `-dump-ir` output with the schema-described operand
shape when their natural surface conditions are met.

The doc itself notes that "most opcodes documented here are
`(synthesized)` — they are introduced by IR passes (layout,
debug-info, SPIR-V asm lowering) rather than by
`slang-lower-to-ir.cpp` visitors." As a result, none of these
opcodes are visible at the `LOWER-TO-IR:` section of the dump.
They appear in the *post-LOWER-TO-IR* sections, starting with
`### AFTER validateAndRemoveAssumeAddress:` (which is the first
post-LOWER dump and is sufficient to observe every layout/attr
opcode in this bundle). For that reason `pipeline_stage=ir-pass`
is used throughout (per `_common.md`'s allowed stages); the doc
itself attributes Layout-family opcodes to the layout pass and
Debug-info opcodes to the debug-info insertion pass.

The bundle samples one to three representative opcodes per
family that have a natural Slang surface:

- **Layout family**: `varLayout`, `EntryPointLayout`,
  `structTypeLayout`, `parameterGroupTypeLayout`,
  `arrayTypeLayout`, `matrixTypeLayout`,
  `structuredBufferTypeLayout`.
- **Attr family**: `stage`, `systemValueSemantic`, `no_diff`,
  `unorm`, `size`, `offset`, `structFieldLayout`.
- **Debug-info family** (requires `-g`): `DebugSource`,
  `DebugFunction`, `DebugLine`, `DebugVar` + `DebugValue`,
  `DebugCompilationUnit`.
- **SPIR-V inline asm**: `SPIRVAsm` + `SPIRVAsmInst` (observed
  through a `spirv_asm{...}` block).

The primary observation mechanism is
`-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute`
followed by a FileCheck against the dumped IR. Debug-info tests
additionally pass `-g`. Tests use FileCheck wildcards for
operand IDs and resource-kind integers that are unstable across
versions.

This bundle is adjacent to:

- [`ir-reference/decorations`](../decorations/) — the much larger
  sibling metadata family of decorations. `layout(%N)` decoration
  in that bundle references a `Layout` opcode that this bundle
  observes directly.
- [`cross-cutting/ir-instructions`](../../cross-cutting/ir-instructions/)
  — the category-level view that samples each IR family once.

## Claims enumerated

| Claim ID | Anchor | Claim (one line) | Tests |
| --- | --- | --- | --- |
| C-01 | [#layout-family](../../../docs/llm-generated/ir-reference/metadata.md#layout-family) | The layout pass synthesizes a `varLayout` opcode for each global parameter. | [`varlayout-on-global-param.slang`](varlayout-on-global-param.slang) |
| C-02 | [#varlayout-and-entrypointlayout](../../../docs/llm-generated/ir-reference/metadata.md#varlayout-and-entrypointlayout) | The layout pass synthesizes an `EntryPointLayout` opcode for each entry point. | [`entry-point-layout.slang`](entry-point-layout.slang) |
| C-03 | [#layout-family](../../../docs/llm-generated/ir-reference/metadata.md#layout-family) | A laid-out struct (including the synthetic struct backing a `cbuffer`) produces a `structTypeLayout` opcode. | [`struct-type-layout-on-cbuffer.slang`](struct-type-layout-on-cbuffer.slang) |
| C-04 | [#layout-family](../../../docs/llm-generated/ir-reference/metadata.md#layout-family) | A `cbuffer` (parameter-group type) produces a `parameterGroupTypeLayout` opcode. | [`parameter-group-type-layout-on-cbuffer.slang`](parameter-group-type-layout-on-cbuffer.slang) |
| C-05 | [#layout-family](../../../docs/llm-generated/ir-reference/metadata.md#layout-family) | An array-typed cbuffer field induces an `arrayTypeLayout` opcode. | [`array-type-layout-on-cbuffer-array.slang`](array-type-layout-on-cbuffer-array.slang) |
| C-06 | [#layout-family](../../../docs/llm-generated/ir-reference/metadata.md#layout-family) | A matrix-typed cbuffer field induces a `matrixTypeLayout` opcode whose first operand encodes row-major vs column-major. | [`matrix-type-layout-on-cbuffer.slang`](matrix-type-layout-on-cbuffer.slang) |
| C-07 | [#layout-family](../../../docs/llm-generated/ir-reference/metadata.md#layout-family) | An `RWStructuredBuffer` global parameter lowers to a `structuredBufferTypeLayout` opcode. | [`structured-buffer-type-layout.slang`](structured-buffer-type-layout.slang) |
| C-08 | [#attr-family](../../../docs/llm-generated/ir-reference/metadata.md#attr-family) | The `stage` Attr opcode tags the EntryPointLayout with its pipeline-stage integer; compute is 6. | [`stage-attr-on-entry-point.slang`](stage-attr-on-entry-point.slang) |
| C-09 | [#usersemantic-vs-systemvaluesemantic](../../../docs/llm-generated/ir-reference/metadata.md#usersemantic-vs-systemvaluesemantic) | An `SV_*` system value lowers to a `systemValueSemantic` Attr opcode with (name, index) operands. | [`system-value-semantic-attr.slang`](system-value-semantic-attr.slang) |
| C-10 | [#attr-family](../../../docs/llm-generated/ir-reference/metadata.md#attr-family) | The `no_diff` Attr opcode is created from `NoDiffModifier` on a function parameter. | [`no-diff-attr-on-param.slang`](no-diff-attr-on-param.slang) |
| C-11 | [#attr-family](../../../docs/llm-generated/ir-reference/metadata.md#attr-family) | The `unorm` Attr opcode is created from `UNormModifier` on a buffer element type. | [`unorm-attr-on-buffer-element.slang`](unorm-attr-on-buffer-element.slang) |
| C-12 | [#attr-family](../../../docs/llm-generated/ir-reference/metadata.md#attr-family) | The `size` Attr opcode records per-resource-kind size on a typeLayout. | [`size-attr-in-layout.slang`](size-attr-in-layout.slang) |
| C-13 | [#attr-family](../../../docs/llm-generated/ir-reference/metadata.md#attr-family) | The `offset` Attr opcode records per-resource-kind offset on a varLayout. | [`offset-attr-in-varlayout.slang`](offset-attr-in-varlayout.slang) |
| C-14 | [#attr-family](../../../docs/llm-generated/ir-reference/metadata.md#attr-family) | A `structFieldLayout` Attr opcode is emitted for each field inside a `structTypeLayout`. | [`struct-field-layout-attr.slang`](struct-field-layout-attr.slang) |
| C-15 | [#debug-info-family](../../../docs/llm-generated/ir-reference/metadata.md#debug-info-family) | With `-g`, the debug-info pass emits a `DebugSource` opcode at module top level. | [`debug-source-with-g.slang`](debug-source-with-g.slang) |
| C-16 | [#debug-info-family](../../../docs/llm-generated/ir-reference/metadata.md#debug-info-family) | With `-g`, the debug-info pass emits a `DebugFunction` for each function. | [`debug-function-with-g.slang`](debug-function-with-g.slang) |
| C-17 | [#debugline](../../../docs/llm-generated/ir-reference/metadata.md#debugline) | `DebugLine` pins an instruction to a `(file, startLine, startCol, endLine, endCol)` range under `-g`. | [`debug-line-with-g.slang`](debug-line-with-g.slang) |
| C-18 | [#debug-info-family](../../../docs/llm-generated/ir-reference/metadata.md#debug-info-family) | `DebugVar` declares a user-visible local; `DebugValue` reports its current value. | [`debug-var-and-value-with-g.slang`](debug-var-and-value-with-g.slang) |
| C-19 | [#debug-info-family](../../../docs/llm-generated/ir-reference/metadata.md#debug-info-family) | `DebugCompilationUnit` declares the compilation unit and references a `DebugSource`. | [`debug-compilation-unit-with-g.slang`](debug-compilation-unit-with-g.slang) |
| C-20 | [#spir-v-inline-asm](../../../docs/llm-generated/ir-reference/metadata.md#spir-v-inline-asm) | A `spirv_asm{...}` block lowers to a `SPIRVAsm` parent opcode owning `SPIRVAsmInst` children. | [`spirv-asm-block.slang`](spirv-asm-block.slang) |
| C-21 | [#layout-family](../../../docs/llm-generated/ir-reference/metadata.md#layout-family) | Multiple `varLayout` opcodes coexist in one module: one per global plus EntryPointLayout-owned ones. | [`varlayout-uniform-and-entry-point.slang`](varlayout-uniform-and-entry-point.slang) |
| C-22 | [#layout-family](../../../docs/llm-generated/ir-reference/metadata.md#layout-family) | The base `typeLayout` opcode (TypeLayoutBase wrapper) carries size/alignment attrs and roots the type-layout hierarchy. | [`type-layout-base.slang`](type-layout-base.slang) |

## Tests in this bundle

| File | Intent | Doc anchor |
| --- | --- | --- |
| [`varlayout-on-global-param.slang`](varlayout-on-global-param.slang) | functional | `#layout-family` |
| [`entry-point-layout.slang`](entry-point-layout.slang) | functional | `#varlayout-and-entrypointlayout` |
| [`struct-type-layout-on-cbuffer.slang`](struct-type-layout-on-cbuffer.slang) | functional | `#layout-family` |
| [`parameter-group-type-layout-on-cbuffer.slang`](parameter-group-type-layout-on-cbuffer.slang) | functional | `#layout-family` |
| [`array-type-layout-on-cbuffer-array.slang`](array-type-layout-on-cbuffer-array.slang) | functional | `#layout-family` |
| [`matrix-type-layout-on-cbuffer.slang`](matrix-type-layout-on-cbuffer.slang) | functional | `#layout-family` |
| [`structured-buffer-type-layout.slang`](structured-buffer-type-layout.slang) | functional | `#layout-family` |
| [`stage-attr-on-entry-point.slang`](stage-attr-on-entry-point.slang) | functional | `#attr-family` |
| [`system-value-semantic-attr.slang`](system-value-semantic-attr.slang) | functional | `#usersemantic-vs-systemvaluesemantic` |
| [`no-diff-attr-on-param.slang`](no-diff-attr-on-param.slang) | functional | `#attr-family` |
| [`unorm-attr-on-buffer-element.slang`](unorm-attr-on-buffer-element.slang) | functional | `#attr-family` |
| [`size-attr-in-layout.slang`](size-attr-in-layout.slang) | functional | `#attr-family` |
| [`offset-attr-in-varlayout.slang`](offset-attr-in-varlayout.slang) | functional | `#attr-family` |
| [`struct-field-layout-attr.slang`](struct-field-layout-attr.slang) | functional | `#attr-family` |
| [`debug-source-with-g.slang`](debug-source-with-g.slang) | functional | `#debug-info-family` |
| [`debug-function-with-g.slang`](debug-function-with-g.slang) | functional | `#debug-info-family` |
| [`debug-line-with-g.slang`](debug-line-with-g.slang) | functional | `#debugline` |
| [`debug-var-and-value-with-g.slang`](debug-var-and-value-with-g.slang) | functional | `#debug-info-family` |
| [`debug-compilation-unit-with-g.slang`](debug-compilation-unit-with-g.slang) | functional | `#debug-info-family` |
| [`spirv-asm-block.slang`](spirv-asm-block.slang) | functional | `#spir-v-inline-asm` |
| [`varlayout-uniform-and-entry-point.slang`](varlayout-uniform-and-entry-point.slang) | functional | `#layout-family` |
| [`type-layout-base.slang`](type-layout-base.slang) | functional | `#layout-family` |

## Out of scope (no-GPU runner)

The doc's tables list many opcodes whose AST origin is
`(synthesized)` and whose surface conditions are too narrow or
too target-specific to make stable per-opcode reference tests
in this representative bundle:

- **Layout family — niche layout kinds**: `streamOutputTypeLayout`
  (requires a geometry-shader stream-output entry-point),
  `existentialTypeLayout` (requires existential-typed values
  visible to layout), `tupleTypeLayout` (tuples are lowered
  before layout in normal Slang surface), `ptrTypeLayout`
  (requires explicit pointer-typed laid-out variables). Each
  is observable in principle with the right surface but
  requires pipeline-specific scaffolding distinct from the
  per-opcode reference goal.

- **Attr family — niche attrs**: `snorm` (mirror of `unorm`;
  same AST origin mechanism, one test is sufficient),
  `nonuniform` (the `NonuniformResourceIndex(...)` intrinsic
  lowers to a `nonUniformResourceIndex` *instruction* call,
  not the `nonuniform` Attr opcode, at the dump points
  observable here; the Attr surface is attached deeper in the
  pipeline), `Aligned` (synthesized; no stable AST surface
  observed), `MemoryScope` (synthesized on atomic/barrier
  operations after later passes), `caseLayout` (existential
  / enum-style layout; requires existential surface),
  `tupleFieldLayout` (tuple-layout-specific), `FuncThrowType`
  (synthesized — observed not to surface on `throws` functions
  in the dump points sampled by this bundle), `userSemantic`
  (the surface `: TEXCOORD0` lowers to `semantic("TEXCOORD0",
  ...)` decoration rather than to a separate `userSemantic`
  Attr opcode at the observation points sampled here).

- **Debug-info family — niche debug opcodes**:
  `DebugInlinedAt`, `DebugInlinedVariable` (require an actual
  inlined call to be observable; would need a `[ForceInline]`
  helper plus a non-trivial scope), `DebugScope`, `DebugNoScope`
  (typically pinned to compound statements rather than top-level
  insts and harder to anchor in a small test), `DebugBuildIdentifier`
  (synthesized at SPIR-V emission with build-tooling-specific
  contents and pollutes the test with non-deterministic strings),
  `EmbeddedDownstreamIR` (requires a precompiled-library workflow,
  which is out of scope).

- **SPIRVAsmOperand opcode catalog**: the inline-asm operand
  opcodes (`SPIRVAsmOperandLiteral`, `SPIRVAsmOperandInst`,
  `SPIRVAsmOperandEnum`, `SPIRVAsmOperandBuiltinVar`,
  `SPIRVAsmOperandGLSL450Set`, `SPIRVAsmOperandDebugPrintfSet`,
  `SPIRVAsmOperandId`, `SPIRVAsmOperandResult`,
  `SPIRVAsmOperandConvertTexel`,
  `SPIRVAsmOperandRayPayloadFromLocation`,
  `SPIRVAsmOperandRayAttributeFromLocation`,
  `SPIRVAsmOperandRayCallableFromLocation`, `__truncate`,
  `__entryPoint`, `__sampledType`, `__imageType`,
  `__sampledImageType`) are typed operands of a
  `SPIRVAsmInst`. The dump prints them inline as the
  arguments of `SPIRVAsmInst(...)` (e.g. `result`, `param`),
  not as separately-named opcodes; observing each per-opcode
  spelling requires an inline-asm fragment exercising that
  operand kind and a parser of the printed operand syntax
  that is not stable across compiler versions. One representative
  `SPIRVAsm` / `SPIRVAsmInst` block is sampled instead.

- **Debug info on inlined call sites**: `DebugInlinedAt` and
  `DebugInlinedVariable` require the inliner to actually
  inline a call under `-g`, which depends on inlining
  decisions and the order of passes — fragile per-test.

## Doc gaps observed

- The Layout-family table lists every opcode's `AST origin` as
  `(synthesized)`, which is true at the LOWER-TO-IR boundary but
  is a one-shot loss of information: the layout opcodes *do*
  have a stable user-side AST surface that triggers them
  (e.g. `cbuffer X { ... }` → `parameterGroupTypeLayout`,
  `float arr[4]` field → `arrayTypeLayout`,
  `float4x4 m` field → `matrixTypeLayout`,
  `RWStructuredBuffer<T>` global → `structuredBufferTypeLayout`).
  Documenting the surface→layout-opcode mapping would make this
  table testable without exploration.

- The Layout-family note "every child is hoistable so identical
  layouts dedupe to a single IR value" is testable but the doc
  does not document at which dump point identical layouts have
  already deduplicated. In practice they dedupe before the first
  post-LOWER dump (`AFTER validateAndRemoveAssumeAddress`), but
  the doc could state this explicitly so test authors know
  which dump section to anchor on.

- The `Attr` table for `size` and `offset` describes them as
  "Type-size record on a layout, keyed by resource kind" and
  "Var-offset record on a `varLayout`, keyed by resource kind"
  respectively, but does not enumerate the resource-kind enum
  values that appear as the first operand (`size(9 : Int, ...)`
  vs `size(8 : Int, ...)`). The integer-to-kind mapping is
  visible in the source but the doc could include a table.

- The `stage` Attr opcode's operand is described as
  `stageOperand: IRIntLit` but the integer-to-stage mapping is
  not enumerated. Observed values: compute = 6. A note pointing
  at the `Stage` enum that produces these tags would let tests
  pin the exact integer.

- The `systemValueSemantic` row says "(`SemanticAttr` is the
  grouping parent of `userSemantic` and `systemValueSemantic`;
  it is not itself an opcode)", which is helpful, but does not
  state which dump form the index operand takes. Observed:
  `systemValueSemantic("SV_DispatchThreadID", 0 : Int)` — the
  index operand is the resolved semantic-index integer, not the
  `-1` sentinel used in the `semantic` decoration. Documenting
  the difference between the `semantic` decoration's index
  (`-1` for "no explicit index") and the `systemValueSemantic`
  Attr's index (the resolved value) would help.

- The Debug-info-family table notes "the opcodes mirror the
  SPIR-V NonSemantic.Shader debug-info extension, but the page
  intentionally avoids citing specific external instruction
  numbers". This is fine but leaves the dump's exact operand
  ordering implicit. For `DebugFunction` the table says
  "(variadic, min=5)" but does not show what the five operands
  are; observed: `("name", line : UInt, line : UInt,
  source, funcType)`. Documenting the canonical operand order
  would let tests pin every operand instead of just the name.

- The `DebugSource` opcode embeds the *full* source file
  contents as its second string operand. That makes the dump's
  `DebugSource(...)` line very long and the literal-string
  match brittle if a test author tries to pin source contents.
  A note that "the second operand is the file contents — match
  only the path or the leading prefix" would prevent surprise.

- The `SPIRVAsm` row lists operands as "(variadic)" and notes
  it is the parent container, but does not document the printed
  dump form. Observed: `SPIRVAsm %N : <result-type>` on one line
  followed by indented `SPIRVAsmInst(opcode, type, operand...)`
  child lines. A worked example would help test authors anchor
  on either the parent or a specific child instruction.

- The `SPIRVAsmInst` row says "(variadic, min=1)" but does not
  document that the first operand is the SPIR-V opcode integer
  (observed: `132 : UInt` for `OpIMul`). Documenting that the
  first operand is the SPIR-V opcode number would let tests
  pin the exact instruction by opcode.
