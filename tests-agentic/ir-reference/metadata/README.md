---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-22T00:00:00+00:00
source_commit: 6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
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

## Coverage

| Claim | Intent | Anchor | Tests |
| --- | --- | --- | --- |
| A structFieldLayout Attr opcode is emitted for each field inside a structTypeLayout; operands are the field key and a varLayout. | functional | [#attr-family](../../../docs/llm-generated/ir-reference/metadata.md#attr-family) | [`struct-field-layout-attr.slang`](struct-field-layout-attr.slang) |
| The no_diff Attr opcode is created from the NoDiffModifier on a function parameter and surfaces as a bare let-binding in the IR dump. | functional | [#attr-family](../../../docs/llm-generated/ir-reference/metadata.md#attr-family) | [`no-diff-attr-on-param.slang`](no-diff-attr-on-param.slang) |
| The NonUniformResourceIndex intrinsic on a bindless Texture2D[] index lowers to a nonUniformResourceIndex IR instruction visible in the IR dump. | expansion | [#attr-family](../../../docs/llm-generated/ir-reference/metadata.md#attr-family) | [`nonuniform-resource-index-bindless.slang`](nonuniform-resource-index-bindless.slang) |
| The offset Attr opcode records per-resource-kind offset on a varLayout; it appears as offset(kind, N) in the layout dump. | functional | [#attr-family](../../../docs/llm-generated/ir-reference/metadata.md#attr-family) | [`offset-attr-in-varlayout.slang`](offset-attr-in-varlayout.slang) |
| The size Attr opcode records per-resource-kind size on a typeLayout; it appears as size(kind, N) in the layout dump. | functional | [#attr-family](../../../docs/llm-generated/ir-reference/metadata.md#attr-family) | [`size-attr-in-layout.slang`](size-attr-in-layout.slang) |
| The stage attribute tags an EntryPointLayout with its pipeline-stage operand; compute is stage 6. | functional | [#attr-family](../../../docs/llm-generated/ir-reference/metadata.md#attr-family) | [`stage-attr-on-entry-point.slang`](stage-attr-on-entry-point.slang) |
| The unorm Attr opcode originates from the UNormModifier on a buffer element type and is recorded as a bare no-operand attribute. | functional | [#attr-family](../../../docs/llm-generated/ir-reference/metadata.md#attr-family) | [`unorm-attr-on-buffer-element.slang`](unorm-attr-on-buffer-element.slang) |
| DebugCompilationUnit declares the compilation unit and references a DebugSource; emitted at module top level under -g. | functional | [#debug-info-family](../../../docs/llm-generated/ir-reference/metadata.md#debug-info-family) | [`debug-compilation-unit-with-g.slang`](debug-compilation-unit-with-g.slang) |
| DebugVar declares a user-visible local for the debugger; DebugValue reports the current value of a DebugVar. | functional | [#debug-info-family](../../../docs/llm-generated/ir-reference/metadata.md#debug-info-family) | [`debug-var-and-value-with-g.slang`](debug-var-and-value-with-g.slang) |
| With -g, the debug-info pass emits a DebugFunction opcode for each function, declaring its name, scope, and file for the debugger. | functional | [#debug-info-family](../../../docs/llm-generated/ir-reference/metadata.md#debug-info-family) | [`debug-function-with-g.slang`](debug-function-with-g.slang) |
| With -g, the debug-info pass emits a DebugSource opcode at module top level that records the source-file path and contents. | functional | [#debug-info-family](../../../docs/llm-generated/ir-reference/metadata.md#debug-info-family) | [`debug-source-with-g.slang`](debug-source-with-g.slang) |
| DebugLine pins an instruction to a (file, startLine, startCol, endLine, endCol) range and is emitted in source order under -g. | functional | [#debugline](../../../docs/llm-generated/ir-reference/metadata.md#debugline) | [`debug-line-with-g.slang`](debug-line-with-g.slang) |
| A cbuffer (a parameter-group type) lowers to a parameterGroupTypeLayout opcode in the layout dump. | functional | [#layout-family](../../../docs/llm-generated/ir-reference/metadata.md#layout-family) | [`parameter-group-type-layout-on-cbuffer.slang`](parameter-group-type-layout-on-cbuffer.slang) |
| A geometry entry point with inout TriangleStream<T> lowers to a streamOutputTypeLayout opcode in the layout dump. | expansion | [#layout-family](../../../docs/llm-generated/ir-reference/metadata.md#layout-family) | [`stream-output-type-layout-geometry.slang`](stream-output-type-layout-geometry.slang) |
| A matrix-typed cbuffer field induces a matrixTypeLayout opcode whose first operand encodes row-major vs column-major. | functional | [#layout-family](../../../docs/llm-generated/ir-reference/metadata.md#layout-family) | [`matrix-type-layout-on-cbuffer.slang`](matrix-type-layout-on-cbuffer.slang) |
| An RWStructuredBuffer global parameter lowers to a structuredBufferTypeLayout opcode in the layout dump. | functional | [#layout-family](../../../docs/llm-generated/ir-reference/metadata.md#layout-family) | [`structured-buffer-type-layout.slang`](structured-buffer-type-layout.slang) |
| An array-typed cbuffer field induces an arrayTypeLayout opcode in the layout dump. | functional | [#layout-family](../../../docs/llm-generated/ir-reference/metadata.md#layout-family) | [`array-type-layout-on-cbuffer-array.slang`](array-type-layout-on-cbuffer-array.slang) |
| The base typeLayout opcode (TypeLayoutBase wrapper) records size, alignment, and resource-usage attrs and is the root of the type-layout hierarchy. | functional | [#layout-family](../../../docs/llm-generated/ir-reference/metadata.md#layout-family) | [`type-layout-base.slang`](type-layout-base.slang) |
| The layout pass synthesizes a structTypeLayout opcode for each laid-out struct (including the synthetic struct backing a cbuffer). | functional | [#layout-family](../../../docs/llm-generated/ir-reference/metadata.md#layout-family) | [`struct-type-layout-on-cbuffer.slang`](struct-type-layout-on-cbuffer.slang) |
| The layout pass synthesizes a varLayout opcode (per-variable layout record) referenced by the global parameter's [layout(...)] decoration. | functional | [#layout-family](../../../docs/llm-generated/ir-reference/metadata.md#layout-family) | [`varlayout-on-global-param.slang`](varlayout-on-global-param.slang) |
| A spirv_asm{...} block lowers to a SPIRVAsm parent opcode that owns SPIRVAsmInst children inside the function body. | functional | [#spir-v-inline-asm](../../../docs/llm-generated/ir-reference/metadata.md#spir-v-inline-asm) | [`spirv-asm-block.slang`](spirv-asm-block.slang) |
| The systemValueSemantic Attr opcode lowers a parameter with an SV_* semantic and tags it inside the layout tree. | functional | [#usersemantic-vs-systemvaluesemantic](../../../docs/llm-generated/ir-reference/metadata.md#usersemantic-vs-systemvaluesemantic) | [`system-value-semantic-attr.slang`](system-value-semantic-attr.slang) |
| Multiple varLayout opcodes coexist in one module: one for each uniform global, plus the EntryPointLayout's owned varLayouts. | functional | [#varlayout-and-entrypointlayout](../../../docs/llm-generated/ir-reference/metadata.md#varlayout-and-entrypointlayout) | [`varlayout-uniform-and-entry-point.slang`](varlayout-uniform-and-entry-point.slang) |
| The layout pass synthesizes an EntryPointLayout opcode for each entry point; it owns a varLayout for the parameter group and a stage attribute. | functional | [#varlayout-and-entrypointlayout](../../../docs/llm-generated/ir-reference/metadata.md#varlayout-and-entrypointlayout) | [`entry-point-layout.slang`](entry-point-layout.slang) |

## Untested coverable claims

| Anchor | Backend | Claim | Why untested |
| --- | --- | --- | --- |
| [#snorm](../../../docs/llm-generated/ir-reference/metadata.md#snorm) | gpu-bindless | **Attr family — remaining niche attrs**: `snorm` (mirror of `unorm`; same AST origin mechanism, one test is sufficient), `Aligned` (synthesized; no stable AST surface observed), `MemoryScope` (synthesized on atomic/barrier operations after later passes), `caseLayout` (existential / enum-style layout; requires existential surface), `tupleFieldLayout` (tuple-layout-specific), `FuncThrowType` (synthesized — observed not to surface on `throws` functions in the dump points sampled by this bundle), `userSemantic` (the surface `: TEXCOORD0` lowers to `semantic("TEXCOORD0", ...)` decoration rather than to a separate `userSemantic` Attr opcode at the observation points sampled here). | Agent runtime has no GPU; CI / local machine does. |
| [#streamoutputtypelayout](../../../docs/llm-generated/ir-reference/metadata.md#streamoutputtypelayout) | gpu-non-compute | **Layout family — remaining niche layout kinds**: `existentialTypeLayout` (requires existential-typed values visible to layout), `tupleTypeLayout` (tuples are lowered before layout in normal Slang surface), `ptrTypeLayout` (requires explicit pointer-typed laid-out variables). Each is observable in principle with the right surface but requires pipeline-specific scaffolding distinct from the per-opcode reference goal. | Agent runtime has no GPU; CI / local machine does. |

## Out of scope

| Anchor | Reason | Claim | Why it's terminal |
| --- | --- | --- | --- |
| [#debuginlinedat](../../../docs/llm-generated/ir-reference/metadata.md#debuginlinedat) | (unclassified) | **Debug-info family — niche debug opcodes**: `DebugInlinedAt`, `DebugInlinedVariable` (require an actual inlined call to be observable; would need a `[ForceInline]` helper plus a non-trivial scope), `DebugScope`, `DebugNoScope` (typically pinned to compound statements rather than top-level insts and harder to anchor in a small test), `DebugBuildIdentifier` (synthesized at SPIR-V emission with build-tooling-specific contents and pollutes the test with non-deterministic strings), `EmbeddedDownstreamIR` (requires a precompiled-library workflow, which is out of scope). | Not reachable via any allowed test directive. |
| [#debuginlinedat](../../../docs/llm-generated/ir-reference/metadata.md#debuginlinedat) | (unclassified) | **Debug info on inlined call sites**: `DebugInlinedAt` and `DebugInlinedVariable` require the inliner to actually inline a call under `-g`, which depends on inlining decisions and the order of passes — fragile per-test. | Not reachable via any allowed test directive. |
| [#spirvasmoperandliteral](../../../docs/llm-generated/ir-reference/metadata.md#spirvasmoperandliteral) | (unclassified) | **SPIRVAsmOperand opcode catalog**: the inline-asm operand opcodes (`SPIRVAsmOperandLiteral`, `SPIRVAsmOperandInst`, `SPIRVAsmOperandEnum`, `SPIRVAsmOperandBuiltinVar`, `SPIRVAsmOperandGLSL450Set`, `SPIRVAsmOperandDebugPrintfSet`, `SPIRVAsmOperandId`, `SPIRVAsmOperandResult`, `SPIRVAsmOperandConvertTexel`, `SPIRVAsmOperandRayPayloadFromLocation`, `SPIRVAsmOperandRayAttributeFromLocation`, `SPIRVAsmOperandRayCallableFromLocation`, `__truncate`, `__entryPoint`, `__sampledType`, `__imageType`, `__sampledImageType`) are typed operands of a `SPIRVAsmInst`. The dump prints them inline as the arguments of `SPIRVAsmInst(...)` (e.g. `result`, `param`), not as separately-named opcodes; observing each per-opcode spelling requires an inline-asm fragment exercising that operand kind and a parser of the printed operand syntax that is not stable across compiler versions. One representative `SPIRVAsm` / `SPIRVAsmInst` block is sampled instead. | Not reachable via any allowed test directive. |

## Doc gaps observed

| Anchor | Kind | Gap | Suggested addition |
| --- | --- | --- | --- |
| (unspecified) | undocumented-behavior | The Layout-family table lists every opcode's `AST origin` as `(synthesized)`, which is true at the LOWER-TO-IR boundary but is a one-shot loss of information: the layout opcodes *do* have a stable user-side AST surface that triggers them (e.g. `cbuffer X { ... }` → `parameterGroupTypeLayout`, `float arr[4]` field → `arrayTypeLayout`, `float4x4 m` field → `matrixTypeLayout`, `RWStructuredBuffer<T>` global → `structuredBufferTypeLayout`). Documenting the surface→layout-opcode mapping would make this table testable without exploration. |  |
| [#every-child-is-hoistable-so-identical-layouts-dedupe-to-a-single-ir-value](../../../docs/llm-generated/ir-reference/metadata.md#every-child-is-hoistable-so-identical-layouts-dedupe-to-a-single-ir-value) | undocumented-behavior | The Layout-family note "every child is hoistable so identical layouts dedupe to a single IR value" is testable but the doc does not document at which dump point identical layouts have already deduplicated. In practice they dedupe before the first post-LOWER dump (`AFTER validateAndRemoveAssumeAddress`), but the doc could state this explicitly so test authors know which dump section to anchor on. |  |
| [#type-size-record-on-a-layout-keyed-by-resource-kind](../../../docs/llm-generated/ir-reference/metadata.md#type-size-record-on-a-layout-keyed-by-resource-kind) | undocumented-behavior | The `Attr` table for `size` and `offset` describes them as "Type-size record on a layout, keyed by resource kind" and "Var-offset record on a `varLayout`, keyed by resource kind" respectively, but does not enumerate the resource-kind enum values that appear as the first operand (`size(9 : Int, ...)` vs `size(8 : Int, ...)`). The integer-to-kind mapping is visible in the source but the doc could include a table. |  |
| [#stage](../../../docs/llm-generated/ir-reference/metadata.md#stage) | undocumented-behavior | The `stage` Attr opcode's operand is described as `stageOperand: IRIntLit` but the integer-to-stage mapping is not enumerated. Observed values: compute = 6. | A note pointing at the `Stage` enum that produces these tags would let tests pin the exact integer. |
| [#svdispatchthreadid](../../../docs/llm-generated/ir-reference/metadata.md#svdispatchthreadid) | undocumented-behavior | The `systemValueSemantic` row says "(`SemanticAttr` is the grouping parent of `userSemantic` and `systemValueSemantic`; it is not itself an opcode)", which is helpful, but does not state which dump form the index operand takes. Observed: `systemValueSemantic("SV_DispatchThreadID", 0 : Int)` — the index operand is the resolved semantic-index integer, not the `-1` sentinel used in the `semantic` decoration. Documenting the difference between the `semantic` decoration's index (`-1` for "no explicit index") and the `systemValueSemantic` Attr's index (the resolved value) would help. |  |
| [#variadic-min5](../../../docs/llm-generated/ir-reference/metadata.md#variadic-min5) | undocumented-behavior | The Debug-info-family table notes "the opcodes mirror the SPIR-V NonSemantic.Shader debug-info extension, but the page intentionally avoids citing specific external instruction numbers". This is fine but leaves the dump's exact operand ordering implicit. For `DebugFunction` the table says "(variadic, min=5)" but does not show what the five operands are; observed: `("name", line : UInt, line : UInt, source, funcType)`. Documenting the canonical operand order would let tests pin every operand instead of just the name. |  |
| [#debugsource](../../../docs/llm-generated/ir-reference/metadata.md#debugsource) | undocumented-behavior | The `DebugSource` opcode embeds the *full* source file contents as its second string operand. That makes the dump's `DebugSource(...)` line very long and the literal-string match brittle if a test author tries to pin source contents. | A note that "the second operand is the file contents — match only the path or the leading prefix" would prevent surprise. |
| [#variadic](../../../docs/llm-generated/ir-reference/metadata.md#variadic) | missing-example | The `SPIRVAsm` row lists operands as "(variadic)" and notes it is the parent container, but does not document the printed dump form. Observed: `SPIRVAsm %N : <result-type>` on one line followed by indented `SPIRVAsmInst(opcode, type, operand...)` child lines. | A worked example would help test authors anchor on either the parent or a specific child instruction. |
| [#variadic-min1](../../../docs/llm-generated/ir-reference/metadata.md#variadic-min1) | undocumented-behavior | The `SPIRVAsmInst` row says "(variadic, min=1)" but does not document that the first operand is the SPIR-V opcode integer (observed: `132 : UInt` for `OpIMul`). Documenting that the first operand is the SPIR-V opcode number would let tests pin the exact instruction by opcode. |  |
