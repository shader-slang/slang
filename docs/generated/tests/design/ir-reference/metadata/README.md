---
generated: true
model: claude-opus-5[1m]
generated_at: 2026-08-13T00:00:00+00:00
source_commit: c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
watched_paths_digest: d15d1888c1942e74bb5f0d1c587028144302fd5be5a0ac77096604764c68cf54
source_doc: docs/generated/design/ir-reference/metadata.md
source_doc_digest: bc653b5707641f7f1f73c6b6ebb22c7405c29f629b0c6633eef90523062888b8
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for design/ir-reference/metadata

## Intent

This bundle exercises
[`docs/generated/design/ir-reference/metadata.md`](../../../../design/ir-reference/metadata.md),
the per-opcode reference for the four IR families that carry metadata about
other instructions: the `Layout` opcodes, the `Attr` opcodes, the `Debug*`
opcodes, and the `SPIRVAsmOperand` opcodes. The doc's own framing is that these
opcodes are records rather than computations, so **operand position is the
entire meaning** — and that is what the bundle is built around. Almost every
test captures the operand ids of one record and then follows each captured id
to its defining instruction, so the test fails if an operand moves, gains a
neighbour, or changes kind. Smoke tests that merely assert "the mnemonic
appears somewhere" are deliberately avoided; they cannot detect the failure
mode the doc warns about.

The observation point throughout is `-dump-ir` paired with a text target and
`-o /dev/null`, because every opcode on this page is either created by
AST-to-IR lowering (the layout, attribute and inline-asm families) or by an
early IR pass (the remaining `Debug*` records) and is therefore present in the
platform-neutral snapshot. Three tests that observe records the inliner
creates use `-dump-ir-after performForceInlining` so the snapshot is the one
the producing pass just wrote.

Two deliberate choices are worth knowing before reading the tests:

- **Debug level 1 where a self-match would be possible.** `DebugSource`
  embeds the entire text of the compiled file as its second operand, so at full
  `-g` the dump contains a copy of the test's own `CHECK` lines and a pattern
  can silently match itself. Tests whose patterns would be ambiguous under that
  echo use `-g1`, which keeps every record they need while leaving the
  source-text operand empty.
- **Boundary pairs for every optional operand.** The doc names three optional
  operands (`offset`'s register space, `TypeAlignment`'s layout unit,
  `DebugVar`'s argument index); each gets one test for the present form and one
  for the absent form, since only the pair distinguishes an optional operand
  from a fixed one.

Sibling bundles: [`ir-reference/decorations`](../decorations/) covers
`LayoutDecoration` and `DebugFuncDecoration`, the decorations that point _at_
the records here; [`cross-cutting/ir-instructions`](../../cross-cutting/ir-instructions/)
covers the schema, op flags and hoistable/parent conventions the families rely
on; [`pipeline/04-ast-to-ir`](../../pipeline/04-ast-to-ir/) covers the lowering
stage that produces them.

Two boundary rows in the tables below are worth calling out because they are
the only claims whose *absent* half has no reachable Slang surface: an invalid
`LayoutSize` extent (the `-2` spelling beside the reachable `-1`), and the
include-flag's `true` form, which needs a second file. Both are recorded in
`## Untested claims` beside the halves that are covered.

## Claims

Enumerated per [`_claims.md` §1](../../../_meta/prompts/_claims.md). This page is
a per-opcode reference, so a large fraction of it describes the compiler's own
structure rather than anything a shader can reveal; the list is therefore split
into **user-observable claims** (C1–C114, each reachable from a compiled shader)
and **internal-source facts** (I1–I10, about C++ wrappers, producing functions
and this documentation set's own bookkeeping). Every C and I identifier below
appears in exactly one of the two tables that follow.

### Layout family

1. `Layout` is the parent group and sets `hoistable = true`, which every child inherits, so identical layouts dedupe to a single IR value.
2. A laid-out instruction is connected to its layout by the `LayoutDecoration` opcode, whose single operand is one of the concrete `Layout` children.
3. `varLayout` carries the type layout as its one fixed operand followed by an attribute tail of `offset`, semantic and `stage`; any shader parameter produces one.
4. `typeLayout` is the fallback used when no specialized sub-opcode applies and carries attribute operands only.
5. `parameterGroupTypeLayout` carries `containerVarLayout`, `elementVarLayout` and `offsetElementTypeLayout`; a `cbuffer` block or a `ConstantBuffer<T>` produces one.
6. `arrayTypeLayout` carries `elementTypeLayout` in operand 0 and derives the element stride rather than storing it; an array field such as `float arr[4]` produces one.
7. `streamOutputTypeLayout` carries `elementTypeLayout`; an `inout TriangleStream<T>` parameter of a geometry entry point produces one.
8. `matrixTypeLayout` operand 0 is a `MatrixLayoutMode` — `1` row-major, `2` column-major — selected by `-matrix-layout-row-major` / `-matrix-layout-column-major`.
9. `existentialTypeLayout` carries attributes only; an interface-typed field such as `ILight light` produces one.
10. `structTypeLayout` carries one `structFieldLayout` attribute per field, in declaration order.
11. `tupleTypeLayout` has no producer at HEAD.
12. `structuredBufferTypeLayout` carries `elementTypeLayout`; a `RWStructuredBuffer<T>` produces one, and its element type `T` gets the fallback `typeLayout`.
13. `ptrTypeLayout` carries attributes only — the pointee layout is deliberately not stored; a pointer field such as `float* p` produces one.
14. `EntryPointLayout` carries exactly two fixed operands, the parameters layout and the result layout, both `varLayout`s.

### Attr family

15. `stage` carries one `Stage` enumerator literal, so a compute entry point prints as `stage(6 : Int)`.
16. `structFieldLayout` carries the field key in operand 0 and the field's `varLayout` in operand 1.
17. `tupleFieldLayout` has no producer at HEAD.
18. `caseLayout` has no producer at HEAD.
19. `unorm` is a no-operand attribute marking a type as the UNORM-normalized form.
20. `snorm` is a no-operand attribute marking a type as the SNORM-normalized form.
21. `no_diff` is a no-operand attribute marking a type as not contributing to derivative computation.
22. `nonuniform` is a no-operand attribute produced by call specialization to mark a resource index as non-uniform.
23. `Aligned` carries the access alignment of a `load` / `store` — not of a type layout — and its public surface is the core-module `loadAligned` / `storeAligned`.
24. `MemoryScope` carries the memory scope of a coherent `load` / `store`, reached through `loadCoherent` / `storeCoherent`, which are declared `[require(SPV_KHR_vulkan_memory_model)]`.
25. `userSemantic` carries a name string in operand 0 and an index in operand 1.
26. `systemValueSemantic` carries the same two operands and tags an `SV_*` system value.
27. `size` carries the `LayoutResourceKind` in operand 0 and the size in operand 1.
28. `offset` carries the kind in operand 0, the offset in operand 1, and an optional register space in operand 2.
29. `TypeAlignment` carries the alignment in operand 0 and an optional layout unit in operand 1.
30. `FuncThrowType` records the error type of a function declared `throws`.

### Debug info family

31. No `Debug*` opcode exists unless debug information is requested, and the requested `DebugInfoLevel` selects which of them exist.
32. `-g` with no suffix is `Standard`, and `-g0`..`-g3` name the level explicitly.
33. At `None` (`-g0`) lowering emits nothing.
34. `Minimal` (`-g1`) produces `DebugSource` with an empty text operand, `DebugLine`, `DebugFunction`, and — once the inliner has run — `DebugScope`, `DebugNoScope` and `DebugInlinedAt`.
35. `Standard` (`-g`, `-g2`) and `Maximal` (`-g3`) additionally give `DebugSource` the file's text and give each non-included source file a `DebugCompilationUnit`.
36. `DebugVar` and `DebugValue` never appear at `-g1`.
37. `-debug-info-include-source` embeds the source text into `DebugSource` even at `Minimal`.
38. Only `DebugSource` and `DebugCompilationUnit` are hoistable among the debug records.
39. `DebugSource` carries the file name, the source text, and an included-file flag.
40. `DebugCompilationUnit` carries a single operand referencing a `DebugSource`.
41. `DebugLine` carries the source, the start and end line, and the start and end column.
42. `DebugVar` carries source, line, column and an optional argument index, and its result type is `Ptr<T>`.
43. `DebugValue` carries the `DebugVar` in operand 0 and the value in operand 1.
44. `DebugInlinedAt` carries line, column, file, the debug function inlined into, and an optional outer frame.
45. `DebugFunction` carries name, line, column, file, debug type and an optional parent scope, and the owning function links to it by `DebugFuncDecoration`.
46. `DebugInlinedVariable` has no producer at HEAD.
47. `DebugScope` carries a scope in operand 0 and an inlining context in operand 1.
48. `DebugNoScope` is emitted with zero operands.
49. `DebugBuildIdentifier` carries a build identifier and a flags operand, and is created only under `-separate-debug-info`.
50. `EmbeddedDownstreamIR` carries an integer `CodeGenTarget` operand and an `IRBlobLit` payload.

### SPIR-V inline asm

51. `SPIRVAsm` is a `parent` instruction owning `SPIRVAsmInst` children.
52. Each `SPIRVAsmInst` takes its SPIR-V opcode as operand 0 and its SPIR-V operands as the remaining operands.
53. A `SPIRVAsm` block is dumped as a typed one-line header followed by an indented brace block.
54. Every `SPIRVAsmOperand` is folded into its use site, so the operand instructions print inside their `SPIRVAsmInst` rather than as separately numbered definitions above it.
55. `SPIRVAsmOperandLiteral` wraps one literal string or 32-bit integer emitted directly as an operand.
56. `SPIRVAsmOperandInst` wraps a reference to a Slang value or type and is deliberately not hoistable.
57. `SPIRVAsmOperandConvertTexel` wraps a value for the implicit texel-format conversion of an image store.
58. `SPIRVAsmOperandRayPayloadFromLocation` wraps a ray payload referenced by location.
59. `SPIRVAsmOperandRayAttributeFromLocation` wraps a ray hit attribute referenced by location.
60. `SPIRVAsmOperandRayCallableFromLocation` wraps a callable-shader payload referenced by location.
61. `SPIRVAsmOperandEnum` wraps a named enumerator plus an optional second operand that requests a constant id instead of a literal.
62. `SPIRVAsmOperandBuiltinVar` carries the built-in kind of a `builtin(...)` token.
63. `SPIRVAsmOperandGLSL450Set` is nullary and references the GLSL.std.450 instruction set.
64. `SPIRVAsmOperandDebugPrintfSet` is nullary and references the NonSemantic.DebugPrintf instruction set.
65. `SPIRVAsmOperandId` carries the name of another instruction's result in the same block.
66. `SPIRVAsmOperandResult` is nullary and marks where the generated result operand is inserted.
67. `__truncate` is a nullary type-directed truncation pseudo-opcode valid as an instruction's opcode operand.
68. `__entryPoint` is nullary and yields the id of an entry point that references the current function.
69. `__sampledType` is a type function giving the result type of sampling an image of a given component type.
70. `__imageType` is a type function giving the equivalent `OpTypeImage` of a sampled-image value.
71. `__sampledImageType` is a type function giving the equivalent sampled-image type.

### Notable opcodes — `Layout`

72. Layout insts carry a `Void` result type and are never folded into their use sites, so the `%N` in a layout decoration is always a reference to a definition printed elsewhere.

### Notable opcodes — `varLayout` and `EntryPointLayout`

73. A `varLayout` carries at most one semantic attribute and at most one `stage`, with the system-value semantic checked first and the user semantic used only as a fallback.
74. An `EntryPointLayout`'s result layout covers only the return value, not `out` / `inout` parameters.

### Notable opcodes — `TypeAlignment`

75. `TypeAlignment` reads the alignment from operand 0 and the layout unit from an optional operand 1, the exception to the kind-first order that `size` and `offset` follow.
76. An absent layout-unit operand means `LayoutResourceKind::Uniform`, that is, bytes.
77. An absent `TypeAlignment` means alignment `1`, so the identity alignment is never emitted.
78. A type layout emits every `size` attribute before any `TypeAlignment`, so each attribute kind forms one contiguous run of operands.
79. Stride is not stored at all but derived by rounding the byte size up to the byte alignment, so a `float[N]` in a constant buffer strides by 16 because the array's alignment is 16 even though the element reports 4.

### Notable opcodes — `size` and `offset`

80. `LayoutResourceKind` 2 is constant buffer.
81. `LayoutResourceKind` 3 is shader resource.
82. `LayoutResourceKind` 4 is unordered access.
83. `LayoutResourceKind` 5 is varying input.
84. `LayoutResourceKind` 6 is varying output.
85. `LayoutResourceKind` 7 is sampler state.
86. `LayoutResourceKind` 8 is uniform, that is, plain bytes.
87. `LayoutResourceKind` 9 is a descriptor-table slot.
88. `LayoutResourceKind` 12 is a register space.
89. The second number changes unit with the first, so `size(8 : Int, 16 : Int)` is sixteen bytes while `size(9 : Int, 2 : Int)` beside it is two descriptor slots.
90. `size` stores a raw `LayoutSize` in operand 1, so an unsized extent surfaces as `-1` and an invalid one as `-2`.
91. `offset` stores a register space in operand 2 only when it is non-zero, and the accessor returns 0 when that operand is missing.

### Notable opcodes — `tupleFieldLayout` and `caseLayout`

92. Neither attribute has a producer in the tree at `source_commit`.

### Notable opcodes — `userSemantic` vs `systemValueSemantic`

93. The two semantic attributes share one operand shape defined once on their common base: operand 0 a name string, operand 1 an index.
94. A user-written `: FOO` semantic lowers to `userSemantic` and an `SV_*` system value to `systemValueSemantic`.

### Notable opcodes — `nonuniform`

95. `nonuniform` never reaches a dump: no pass dump of the worked-example compile contains either `nonuniform` or an `Attributed` type.
96. Its only visible effect is that `specializeResourceUsage` splits `f` into two specialized functions with the identical signature `Func(Vec(Float, 4 : Int), UInt)`.

### Notable opcodes — `DebugSource`

97. Operand 1 holds the entire text of the file, so a `-dump-ir` of a `-g` compile contains a verbatim copy of its own input.
98. Compiling at `-g1` leaves the text operand an empty string.

### Notable opcodes — `DebugFunction`

99. `DebugFunction` picks the five- or six-operand form rather than storing a null, so the trailing operand is absent at `Minimal`, where no compilation unit is built at all.
100. The trailing operand is the `DebugCompilationUnit` of the source file the function is *defined* in, so an imported function resolves to its own module's unit and an `#include`d or `#line`-remapped file never gets one.

### Notable opcodes — `DebugVar`

101. The variable's own type is not an operand at all — it is the pointee of the instruction's `Ptr<T>` result type.

### Notable opcodes — `DebugLine`

102. `DebugLine` is an ordinary instruction in the block's stream rather than a decoration, so the location travels with position in the block across CFG transformations.
103. One `IfStmt` gets a marker at both the predicate and its `afterLoc`, and a `for` gets separate condition and increment markers.
104. Marker emission skips Slang-synthesized constructors so a debugger cannot step into compiler-generated code.

### Notable opcodes — `DebugScope`

105. `DebugScope` operand 0 references the enclosing scope — a `DebugFunction` for a function-level scope, or another `DebugScope` for a nested block — and operand 1 records the inlining context.
106. `DebugNoScope` is declared with `min_operands = 1` but emitted with zero operands, so its scope accessor must not be called on an instruction from that emitter.

### Notable opcodes — `SPIRVAsmOperand` printed forms

107. `SPIRVAsmOperandLiteral`, `SPIRVAsmOperandEnum` and `SPIRVAsmOperandInst` print only their wrapped operand, so a literal and a named enumerator are not distinguishable from the dump alone.
108. `SPIRVAsmOperandId` prints `%"name"`, `SPIRVAsmOperandResult` prints `result`, and `__truncate` prints `__truncate`.
109. The three type functions print as calls, as do the three late-resolving location kinds.
110. Everything else prints by its own opcode mnemonic followed by its operands, with the nullary `SPIRVAsmOperandGLSL450Set`, `SPIRVAsmOperandDebugPrintfSet` and `__entryPoint` printing with no parentheses at all.

### Notable opcodes — `SPIRVAsmOperandInst` (non-hoistable)

111. `SPIRVAsmOperandInst`, `SPIRVAsmOperandConvertTexel` and the three `...FromLocation` kinds are non-hoistable because each wraps a value that is resolved late.

### Notable opcodes — the three type functions

112. The three type functions compute a SPIR-V type from their operand at emit time rather than holding a value present in the IR.
113. `__truncate` is accepted in operand 0 of a `SPIRVAsmInst` alongside a `SPIRVAsmOperandEnum` or `SPIRVAsmOperandLiteral`.

### Internal-source facts

- **I1.** The four families live in distinct Lua entry groups and comprise fifty-nine concrete opcodes.
- **I2.** Thirty-eight of the fifty-nine have hand-written C++ wrappers and twenty-one are emitted by the FIDDLE template, because an entry declaring only `min_operands` gets no generated accessors.
- **I3.** Layout and layout-attribute opcodes are produced by AST-to-IR lowering rather than by an IR pass.
- **I4.** The whole `SPIRVAsm*` group comes from one visitor, `visitSPIRVAsmExpr`.
- **I5.** The `Debug*` opcodes are split across lowering, the inliner, the debug-value-store pass, type legalization and `linkAndOptimizeIR`.
- **I6.** `Layout`, `TypeLayout`, `Attr`, `SemanticAttr`, `LayoutResourceInfoAttr` and `SPIRVAsmOperand` are abstract grouping entries defining a contiguous opcode range, so `as<IRAttr>()` is a range comparison and no instruction ever carries one of those opcodes.
- **I7.** The `Debug*` opcodes are eleven sibling top-level entries with no common parent, so there is no `as<IRDebugInst>()`.
- **I8.** `IRBuilder::getTupleFieldLayoutAttr` creates the attribute with the layout as its only operand while `IRTupleFieldLayoutAttr::getLayout()` reads operand 1.
- **I9.** The `DebugVar` Lua operand names are stale aliases for `(source, line, col, argIndex?)`, and the generated `getType()` shadows `IRInst::getType()`.
- **I10.** Three producers of opcodes on this page are outside the document's `watched_paths` and should be added.

## Functional coverage

| Claim | Intent | Anchor | Tests |
| --- | --- | --- | --- |
| C15: The stage attribute tags an entry-point varLayout with a single pipeline-stage integer literal indexing the Stage enumeration. | functional | [#attr-family](../../../../design/ir-reference/metadata.md#attr-family) | [`stage-attr-on-entry-point-varlayout.slang`](stage-attr-on-entry-point-varlayout.slang) |
| C16: A structFieldLayout attribute stores the field key in operand 0 and the field's varLayout in operand 1. | functional | [#attr-family](../../../../design/ir-reference/metadata.md#attr-family) | [`struct-field-layout-key-and-varlayout.slang`](struct-field-layout-key-and-varlayout.slang) |
| C19: The unorm modifier on a texture or structured-buffer element type produces a no-operand unorm attribute inside an Attributed type wrapper. | functional, boundary | [#attr-family](../../../../design/ir-reference/metadata.md#attr-family) | [`unorm-attr-on-rwtexture-element.slang`](unorm-attr-on-rwtexture-element.slang), [`unorm-attr-on-structured-buffer-element.slang`](unorm-attr-on-structured-buffer-element.slang) |
| C20: The snorm modifier on a texture element type produces a no-operand snorm attribute inside an Attributed type wrapper. | functional | [#attr-family](../../../../design/ir-reference/metadata.md#attr-family) | [`snorm-attr-on-rwtexture-element.slang`](snorm-attr-on-rwtexture-element.slang) |
| C21: The no_diff modifier on a parameter type produces a no-operand no_diff attribute that marks the type as not contributing to derivatives. | functional | [#attr-family](../../../../design/ir-reference/metadata.md#attr-family) | [`no-diff-attr-on-param.slang`](no-diff-attr-on-param.slang) |
| C23: The Aligned attribute records the access alignment of a load or a store, appearing in the operand tail of those instructions rather than in a type layout. | functional | [#attr-family](../../../../design/ir-reference/metadata.md#attr-family) | [`aligned-attr-on-load-and-store.slang`](aligned-attr-on-load-and-store.slang) |
| C24: The MemoryScope attribute records the memory scope of a coherent store, sitting after the Aligned attribute in the store's operand tail. | functional | [#attr-family](../../../../design/ir-reference/metadata.md#attr-family) | [`memory-scope-attr-on-coherent-store.slang`](memory-scope-attr-on-coherent-store.slang) |
| C31, C32, C33, C34: At debug level None (-g0) lowering emits no Debug records at all, while the same shader at -g1 produces the source, function and line records. | boundary | [#debug-info-family](../../../../design/ir-reference/metadata.md#debug-info-family) | [`debug-level-none-emits-no-records.slang`](debug-level-none-emits-no-records.slang) |
| C34, C44: DebugInlinedAt records one frame of an inlining chain as line, column, source file and the debug function it was inlined into, with the outer-frame operand absent at the outermost frame -- and it exists at -g1 once the inliner has run. | functional | [#debug-info-family](../../../../design/ir-reference/metadata.md#debug-info-family) | [`debug-inlined-at-frame-operands.slang`](debug-inlined-at-frame-operands.slang) |
| C35, C36: Debug level Minimal (-g1) produces no DebugCompilationUnit and no DebugVar or DebugValue, all three of which the Standard level adds. | boundary | [#debug-info-family](../../../../design/ir-reference/metadata.md#debug-info-family) | [`debug-level-minimal-omits-var-value-and-unit.slang`](debug-level-minimal-omits-var-value-and-unit.slang) |
| C37: The -debug-info-include-source flag embeds the source text into DebugSource even at Minimal, the one exception to the debug-level rule. | boundary | [#debug-info-family](../../../../design/ir-reference/metadata.md#debug-info-family) | [`debug-info-include-source-embeds-text-at-minimal.slang`](debug-info-include-source-embeds-text-at-minimal.slang) |
| C39, C97: DebugSource records a source file's path, its embedded text, and a flag saying whether the file was included. | functional | [#debug-info-family](../../../../design/ir-reference/metadata.md#debug-info-family) | [`debug-source-records-path-and-include-flag.slang`](debug-source-records-path-and-include-flag.slang) |
| C40: DebugCompilationUnit declares the compilation unit with a single operand referencing a DebugSource. | functional | [#debug-info-family](../../../../design/ir-reference/metadata.md#debug-info-family) | [`debug-compilation-unit-references-debug-source.slang`](debug-compilation-unit-references-debug-source.slang) |
| C43: DebugValue reports the current value of a DebugVar, taking the variable declaration in operand 0 and the value in operand 1. | functional | [#debug-info-family](../../../../design/ir-reference/metadata.md#debug-info-family) | [`debug-value-reports-debug-var.slang`](debug-value-reports-debug-var.slang) |
| C45, C99: DebugFunction carries the optional trailing parentScope operand at full debug level, where a DebugCompilationUnit exists for it to reference; the five-operand form appears only at -g1. | boundary | [#debug-info-family](../../../../design/ir-reference/metadata.md#debug-info-family) | [`debug-function-parent-scope-at-full-debug.slang`](debug-function-parent-scope-at-full-debug.slang) |
| C45: DebugFunction declares a function for the debugger with its name, line, column, source file and function type, and the owning function links to it by decoration. | functional | [#debug-info-family](../../../../design/ir-reference/metadata.md#debug-info-family) | [`debug-function-name-line-col-file-type.slang`](debug-function-name-line-col-file-type.slang) |
| C46: No DebugInlinedVariable opcode is produced anywhere, even when a call is inlined under debug info. | negative | [#debug-info-family](../../../../design/ir-reference/metadata.md#debug-info-family) | [`debug-inlined-variable-has-no-producer.slang`](debug-inlined-variable-has-no-producer.slang) |
| C49: DebugBuildIdentifier records the build identifier of the compilation together with a flags operand when separate debug information is requested. | functional | [#debug-info-family](../../../../design/ir-reference/metadata.md#debug-info-family) | [`debug-build-identifier-with-separate-debug-info.slang`](debug-build-identifier-with-separate-debug-info.slang) |
| C102: DebugLine is an ordinary instruction in the block's stream rather than a decoration, so the location travels with position in the block. | functional | [#debugline](../../../../design/ir-reference/metadata.md#debugline) | [`debug-line-is-an-instruction-not-a-decoration.slang`](debug-line-is-an-instruction-not-a-decoration.slang) |
| C103: One if statement is marked at both its predicate and its after location, and a for statement gets separate condition and increment markers. | functional | [#debugline](../../../../design/ir-reference/metadata.md#debugline) | [`debug-line-if-predicate-and-after-markers.slang`](debug-line-if-predicate-and-after-markers.slang), [`debug-line-separate-for-condition-and-increment.slang`](debug-line-separate-for-condition-and-increment.slang) |
| C104: Lowering emits no DebugLine markers inside a Slang-synthesized constructor, so a debugger cannot step into compiler-generated code. | negative | [#debugline](../../../../design/ir-reference/metadata.md#debugline) | [`debug-line-skips-synthesized-constructor.slang`](debug-line-skips-synthesized-constructor.slang) |
| C41: DebugLine pins an instruction to a source range with five operands: the source file, the start and end lines, and the start and end columns. | functional | [#debugline](../../../../design/ir-reference/metadata.md#debugline) | [`debug-line-five-operand-range.slang`](debug-line-five-operand-range.slang) |
| C47, C105: A DebugScope opens a lexical scope whose operand 0 references the enclosing scope, a DebugFunction for a function-level scope, and whose operand 1 records the inlining context. | functional | [#debugscope](../../../../design/ir-reference/metadata.md#debugscope) | [`debug-scope-references-debug-function.slang`](debug-scope-references-debug-function.slang) |
| C48, C106: DebugNoScope is emitted with zero operands, so its declared scope accessor must not be called on an instruction from that emitter. | boundary | [#debugscope](../../../../design/ir-reference/metadata.md#debugscope) | [`debug-no-scope-emitted-without-operands.slang`](debug-no-scope-emitted-without-operands.slang) |
| C98: Compiling at -g1 leaves the DebugSource text operand an empty string, so the record carries the path without a copy of the file. | boundary | [#debugsource](../../../../design/ir-reference/metadata.md#debugsource) | [`debug-source-text-operand-empty-at-minimal.slang`](debug-source-text-operand-empty-at-minimal.slang) |
| C42, C101: A DebugVar for an ordinary local omits the argument-index operand, and the variable's own type is the pointee of the instruction's pointer result type rather than an operand. | boundary | [#debugvar](../../../../design/ir-reference/metadata.md#debugvar) | [`debug-var-local-result-type-is-pointer.slang`](debug-var-local-result-type-is-pointer.slang) |
| C42: A DebugVar for an entry-point parameter carries the optional argument-index operand after source, line and column. | boundary | [#debugvar](../../../../design/ir-reference/metadata.md#debugvar) | [`debug-var-param-carries-arg-index.slang`](debug-var-param-carries-arg-index.slang) |
| C2, C72: A laid-out instruction is connected to its layout by a layout decoration whose operand is a concrete Layout child, printed as a separate Void-typed definition elsewhere in the dump. | functional | [#layout](../../../../design/ir-reference/metadata.md#layout) | [`layout-decoration-reaches-concrete-layout-child.slang`](layout-decoration-reaches-concrete-layout-child.slang) |
| C10: A structTypeLayout carries one structFieldLayout attribute per field, and those attributes appear in field declaration order. | functional | [#layout-family](../../../../design/ir-reference/metadata.md#layout-family) | [`struct-type-layout-field-attrs-in-declaration-order.slang`](struct-type-layout-field-attrs-in-declaration-order.slang) |
| C12: A structured-buffer parameter lowers to a structuredBufferTypeLayout whose operand 0 is the element type layout. | functional | [#layout-family](../../../../design/ir-reference/metadata.md#layout-family) | [`structured-buffer-type-layout-element-operand.slang`](structured-buffer-type-layout-element-operand.slang) |
| C13: A pointer-typed field lowers to a ptrTypeLayout that stores only attributes, deliberately not a pointee layout. | functional | [#layout-family](../../../../design/ir-reference/metadata.md#layout-family) | [`ptr-type-layout-omits-pointee-layout.slang`](ptr-type-layout-omits-pointee-layout.slang) |
| C1: Layout records are hoistable, so two distinct layouts that need the same attribute value share one attribute instruction. | functional | [#layout-family](../../../../design/ir-reference/metadata.md#layout-family) | [`layout-hoistable-attrs-dedupe.slang`](layout-hoistable-attrs-dedupe.slang) |
| C4, C12: The generic typeLayout opcode carries only attribute operands, with no fixed layout operand ahead of them. | functional | [#layout-family](../../../../design/ir-reference/metadata.md#layout-family) | [`type-layout-fallback-attrs-only.slang`](type-layout-fallback-attrs-only.slang) |
| C5: A cbuffer lowers to a parameterGroupTypeLayout whose first three operands are the container var layout, the element var layout, and the offset element type layout. | functional | [#layout-family](../../../../design/ir-reference/metadata.md#layout-family) | [`parameter-group-type-layout-cbuffer.slang`](parameter-group-type-layout-cbuffer.slang) |
| C6: An arrayTypeLayout stores the element type layout in operand 0 and derives the element stride rather than storing it. | functional | [#layout-family](../../../../design/ir-reference/metadata.md#layout-family) | [`array-type-layout-element-layout-first.slang`](array-type-layout-element-layout-first.slang) |
| C7: A geometry entry point with an inout TriangleStream parameter produces a streamOutputTypeLayout whose operand 0 is the element type layout. | functional | [#layout-family](../../../../design/ir-reference/metadata.md#layout-family) | [`stream-output-type-layout-geometry.slang`](stream-output-type-layout-geometry.slang) |
| C8: A matrixTypeLayout stores a MatrixLayoutMode integer literal in operand 0, which changes with the requested default matrix layout. | boundary | [#layout-family](../../../../design/ir-reference/metadata.md#layout-family) | [`matrix-type-layout-mode-operand.slang`](matrix-type-layout-mode-operand.slang) |
| C9: An interface-typed field of a laid-out struct produces an existentialTypeLayout record. | functional | [#layout-family](../../../../design/ir-reference/metadata.md#layout-family) | [`existential-type-layout-interface-field.slang`](existential-type-layout-interface-field.slang) |
| C22, C95, C96: The nonuniform attribute never reaches a dump; its only visible effect is that the call site splits into two specialized functions with an identical signature. | negative | [#nonuniform](../../../../design/ir-reference/metadata.md#nonuniform) | [`non-uniform-attr-absent-while-call-splits.slang`](non-uniform-attr-absent-while-call-splits.slang) |
| C67, C69, C113: \_\_truncate is a pseudo-opcode accepted in an inline-asm instruction's opcode operand, and \_\_sampledType computes the result type of sampling an image of a given component type. | functional | [#sampledtype--imagetype--sampledimagetype](../../../../design/ir-reference/metadata.md#sampledtype--imagetype--sampledimagetype) | [`spirv-asm-truncate-as-opcode-operand.slang`](spirv-asm-truncate-as-opcode-operand.slang) |
| C70, C71, C112: The \_\_imageType and \_\_sampledImageType tokens are type functions whose operand is a value in scope, so the SPIR-V type is computed at emit time rather than stored in the IR. | functional | [#sampledtype--imagetype--sampledimagetype](../../../../design/ir-reference/metadata.md#sampledtype--imagetype--sampledimagetype) | [`spirv-asm-image-type-functions.slang`](spirv-asm-image-type-functions.slang) |
| C27, C86, C87, C89: A size attribute puts the LayoutResourceKind in operand 0 and the size in operand 1, so one type layout can carry a separate size per kind -- kind 8 counting bytes beside kind 9 counting descriptor slots. | functional | [#size-and-offset](../../../../design/ir-reference/metadata.md#size-and-offset) | [`size-attr-resource-kind-first.slang`](size-attr-resource-kind-first.slang) |
| C28, C91: An offset attribute gains a third operand holding the register space when that space is non-zero. | boundary | [#size-and-offset](../../../../design/ir-reference/metadata.md#size-and-offset) | [`offset-attr-records-nonzero-space.slang`](offset-attr-records-nonzero-space.slang) |
| C28, C91: An offset attribute stores kind and offset only, omitting the register-space operand when the space is zero. | boundary | [#size-and-offset](../../../../design/ir-reference/metadata.md#size-and-offset) | [`offset-attr-omits-zero-space.slang`](offset-attr-omits-zero-space.slang) |
| C80: A cbuffer's own binding uses LayoutResourceKind 2, constant buffer, in operand 0 of its size and offset attributes. | functional | [#size-and-offset](../../../../design/ir-reference/metadata.md#size-and-offset) | [`resource-kind-constant-buffer-two.slang`](resource-kind-constant-buffer-two.slang) |
| C81: A read-only texture parameter uses LayoutResourceKind 3, shader resource, in operand 0 of its size and offset attributes. | functional | [#size-and-offset](../../../../design/ir-reference/metadata.md#size-and-offset) | [`resource-kind-shader-resource-three.slang`](resource-kind-shader-resource-three.slang) |
| C82: A read-write structured buffer uses LayoutResourceKind 4, unordered access, in operand 0 of its size and offset attributes. | functional | [#size-and-offset](../../../../design/ir-reference/metadata.md#size-and-offset) | [`resource-kind-unordered-access-four.slang`](resource-kind-unordered-access-four.slang) |
| C83: A rasterizer-stage input parameter uses LayoutResourceKind 5, varying input, in operand 0 of its size and offset attributes. | functional | [#size-and-offset](../../../../design/ir-reference/metadata.md#size-and-offset) | [`resource-kind-varying-input-five.slang`](resource-kind-varying-input-five.slang) |
| C84: A rasterizer-stage result uses LayoutResourceKind 6, varying output, in operand 0 of its size and offset attributes. | functional | [#size-and-offset](../../../../design/ir-reference/metadata.md#size-and-offset) | [`resource-kind-varying-output-six.slang`](resource-kind-varying-output-six.slang) |
| C85: A SamplerState parameter uses LayoutResourceKind 7, sampler state, in operand 0 of its size and offset attributes. | functional | [#size-and-offset](../../../../design/ir-reference/metadata.md#size-and-offset) | [`resource-kind-sampler-state-seven.slang`](resource-kind-sampler-state-seven.slang) |
| C88: A ParameterBlock consumes a whole register space, recorded as LayoutResourceKind 12 in operand 0 of an offset attribute on its container var layout. | functional | [#size-and-offset](../../../../design/ir-reference/metadata.md#size-and-offset) | [`resource-kind-register-space-twelve.slang`](resource-kind-register-space-twelve.slang) |
| C90: An unbounded resource array records a non-finite extent in the size attribute's raw LayoutSize operand rather than a finite count. | boundary | [#size-and-offset](../../../../design/ir-reference/metadata.md#size-and-offset) | [`size-attr-unsized-array-raw-value.slang`](size-attr-unsized-array-raw-value.slang) |
| C51, C52, C53, C54: A spirv_asm block lowers to a SPIRVAsm parent instruction owning SPIRVAsmInst children, each of which takes its SPIR-V opcode as operand 0 and prints its folded operands inline. | functional | [#spir-v-inline-asm](../../../../design/ir-reference/metadata.md#spir-v-inline-asm) | [`spirv-asm-parent-owns-inst-children.slang`](spirv-asm-parent-owns-inst-children.slang) |
| C62: A builtin(...) token in an inline-asm instruction becomes a SPIRVAsmOperandBuiltinVar operand carrying the built-in kind. | functional | [#spirvasmoperand](../../../../design/ir-reference/metadata.md#spirvasmoperand) | [`spirv-asm-operand-builtin-var.slang`](spirv-asm-operand-builtin-var.slang) |
| C63, C110: The glsl450 token in an inline-asm instruction becomes a dedicated nullary operand instruction referencing the GLSL.std.450 instruction set. | functional | [#spirvasmoperand](../../../../design/ir-reference/metadata.md#spirvasmoperand) | [`spirv-asm-operand-glsl450-set.slang`](spirv-asm-operand-glsl450-set.slang) |
| C65, C66, C108: Each token of an inline-asm instruction becomes a typed operand instruction: the result marker and a named id are distinct operand kinds carried in the SPIRVAsmInst operand list. | functional | [#spirvasmoperand](../../../../design/ir-reference/metadata.md#spirvasmoperand) | [`spirv-asm-operand-result-and-named-id.slang`](spirv-asm-operand-result-and-named-id.slang) |
| C68, C107, C110: The \_\_entryPoint token prints bare with no parentheses inside its SPIRVAsmInst, while a named enumerator beside it prints as only its wrapped integer. | functional | [#spirvasmoperand](../../../../design/ir-reference/metadata.md#spirvasmoperand) | [`spirv-asm-entry-point-and-enum-printed-forms.slang`](spirv-asm-entry-point-and-enum-printed-forms.slang) |
| C11, C17, C18, C92: The dormant tuple and union layout opcodes (tupleTypeLayout, tupleFieldLayout, caseLayout) have no producer in the compiler, so none of them appears in any IR dump. | negative | [#tuplefieldlayout-and-caselayout](../../../../design/ir-reference/metadata.md#tuplefieldlayout-and-caselayout) | [`tuple-field-and-case-layout-have-no-producer.slang`](tuple-field-and-case-layout-have-no-producer.slang) |
| C29, C75, C76: A TypeAlignment attribute reads the alignment from operand 0 and omits the layout-unit operand for the byte default, unlike the kind-first size and offset attributes. | functional | [#typealignment--the-operand-order-exception](../../../../design/ir-reference/metadata.md#typealignment--the-operand-order-exception) | [`type-alignment-attr-alignment-first.slang`](type-alignment-attr-alignment-first.slang) |
| C77: An absent TypeAlignment attribute encodes alignment 1, so the identity alignment is never emitted. | boundary | [#typealignment--the-operand-order-exception](../../../../design/ir-reference/metadata.md#typealignment--the-operand-order-exception) | [`type-alignment-absent-for-alignment-one.slang`](type-alignment-absent-for-alignment-one.slang) |
| C78: A type layout emits every size attribute before any TypeAlignment attribute, so each attribute kind forms one contiguous run of operands. | boundary | [#typealignment--the-operand-order-exception](../../../../design/ir-reference/metadata.md#typealignment--the-operand-order-exception) | [`type-alignment-attrs-follow-all-size-attrs.slang`](type-alignment-attrs-follow-all-size-attrs.slang) |
| C79: Stride is not stored at all: a float[4] in a constant buffer strides by 16 because the array's own alignment is 16 even though the element reports size and alignment 4. | boundary | [#typealignment--the-operand-order-exception](../../../../design/ir-reference/metadata.md#typealignment--the-operand-order-exception) | [`array-stride-derived-not-stored.slang`](array-stride-derived-not-stored.slang) |
| C25, C93: A user-written semantic lowers to a userSemantic attribute whose operand 0 is the name string and operand 1 the semantic index. | functional | [#usersemantic-vs-systemvaluesemantic](../../../../design/ir-reference/metadata.md#usersemantic-vs-systemvaluesemantic) | [`user-semantic-name-and-index-operands.slang`](user-semantic-name-and-index-operands.slang) |
| C26, C73, C94: A varLayout carries at most one semantic attribute: an SV\_ parameter gets a systemValueSemantic and no userSemantic is created for it. | negative | [#usersemantic-vs-systemvaluesemantic](../../../../design/ir-reference/metadata.md#usersemantic-vs-systemvaluesemantic) | [`system-value-semantic-suppresses-user-semantic.slang`](system-value-semantic-suppresses-user-semantic.slang) |
| C14: An EntryPointLayout has exactly two fixed operands and no attribute tail: the parameters layout and the result layout, both varLayouts. | functional | [#varlayout-and-entrypointlayout](../../../../design/ir-reference/metadata.md#varlayout-and-entrypointlayout) | [`entry-point-layout-two-varlayout-operands.slang`](entry-point-layout-two-varlayout-operands.slang) |
| C3: A varLayout has one fixed operand, the type layout, followed by a variable-length tail of attributes. | functional | [#varlayout-and-entrypointlayout](../../../../design/ir-reference/metadata.md#varlayout-and-entrypointlayout) | [`varlayout-type-layout-then-attr-tail.slang`](varlayout-type-layout-then-attr-tail.slang) |
| C74: An EntryPointLayout's result layout covers only the return value, so an out parameter is laid out in the parameters layout instead. | boundary | [#varlayout-and-entrypointlayout](../../../../design/ir-reference/metadata.md#varlayout-and-entrypointlayout) | [`entry-point-layout-result-excludes-out-param.slang`](entry-point-layout-result-excludes-out-param.slang) |

## Untested claims

| Claim | Reason | Anchor | Why untested |
| --- | --- | --- | --- |
| C30: `FuncThrowType` records the error type of a function declared `throws`. | implementation-detail | [#attr-family](../../../../design/ir-reference/metadata.md#attr-family) | A `throws` function already shows a `Result(T, E)` return type in the first available dump section, so the attribute has been consumed before any observable snapshot; no `-dump-ir-before` phase name exposes the intermediate form. |
| C38: Only `DebugSource` and `DebugCompilationUnit` are hoistable among the debug records. | implementation-detail | [#debug-info-family](../../../../design/ir-reference/metadata.md#debug-info-family) | Hoistability shows up only as deduplication, and a single translation unit produces one of each of those two records, so there is no second identical record whose collapse could be observed. |
| C39 (include-flag `true` half): `DebugSource`'s third operand is `true` for a file reached through an include rather than the primary translation unit. | needs-multi-file-test | [#debug-info-family](../../../../design/ir-reference/metadata.md#debug-info-family) | Observing the `true` form requires a second `.slang` file to `#include` or `__include`, which a single-file bundle test cannot express; the `false` form is covered by `debug-source-records-path-and-include-flag.slang`. |
| C50: `EmbeddedDownstreamIR` embeds a precompiled downstream blob for one CodeGenTarget, keyed by an integer target operand with the blob in operand 1. | needs-multi-file-test | [#embeddeddownstreamir](../../../../design/ir-reference/metadata.md#embeddeddownstreamir) | The opcode only exists when a translation unit is precompiled into a `.slang-module` with embedded downstream IR and then linked, which needs a second file plus a two-step `slangc` invocation rather than one `//TEST` directive. |
| C55, C61, C107: `SPIRVAsmOperandLiteral` and `SPIRVAsmOperandEnum` -- including the enum's optional constant-type operand -- print as only their wrapped operand, so a literal and a named enumerator are indistinguishable in a dump. | implementation-detail | [#spirvasmoperand](../../../../design/ir-reference/metadata.md#spirvasmoperand) | The kind is erased by the printer, so no CHECK can tell which of the two produced a bare `31 : UInt`; `spirv-asm-entry-point-and-enum-printed-forms.slang` pins the erasure itself, but not which kind was erased. |
| C56, C111: `SPIRVAsmOperandInst` is deliberately not hoistable so a pass can rewrite the Slang value it references without disturbing other asm blocks, and `SPIRVAsmOperandConvertTexel` plus the three `...FromLocation` kinds are non-hoistable for the same reason. | implementation-detail | [#spirvasmoperandinst-non-hoistable](../../../../design/ir-reference/metadata.md#spirvasmoperandinst-non-hoistable) | Hoistability is only observable as deduplication, and the IR dump prints these operands inline inside their `SPIRVAsmInst` line rather than as separately-numbered instructions, so two identical operands cannot be shown to be distinct values from the dump text. |
| C57, C58, C59, C60, C64, C109 (location-kind half): the texel-conversion operand, the three ray `...FromLocation` operand kinds, and the NonSemantic.DebugPrintf instruction-set operand. | gpu-vulkan-extension | [#spirvasmoperand](../../../../design/ir-reference/metadata.md#spirvasmoperand) | The texel-conversion kind needs an inline-asm image store, the three location kinds need ray-tracing entry points with the matching Vulkan capability set, and the debug-printf set needs that extended instruction set enabled -- none of which this bundle's compute entry points can request. |
| C90 (invalid-extent half): an invalid extent surfaces as `-2` in the raw `LayoutSize` operand. | implementation-detail | [#size-and-offset](../../../../design/ir-reference/metadata.md#size-and-offset) | The `-1` unsized sentinel is reachable from an unbounded resource array and is covered, but no Slang declaration was found that makes a layout report an *invalid* extent, so the `-2` spelling has no surface to drive it from. |
| C100: `DebugFunction`'s trailing operand is the `DebugCompilationUnit` of the file the function is defined in, so an imported function resolves to its own module's unit and an `#include`d or `#line`-remapped file never gets one. | needs-multi-file-test | [#debugfunction](../../../../design/ir-reference/metadata.md#debugfunction) | Distinguishing "the defining module's unit" from "the entry point's unit" needs a second module to import from, and the `#include` half needs a second file to include; a single-file test only ever has one compilation unit to point at. |
| I1, I2: The four families comprise fifty-nine concrete opcodes across distinct Lua entry groups, thirty-eight of which have hand-written C++ wrapper structs because their entries declare only `min_operands` and so get no generated accessors. | internal-source-fact | [#source](../../../../design/ir-reference/metadata.md#source) | Which wrappers are hand-written and which are generated is a property of the compiler's own headers with no consequence a compiled shader can reveal. |
| I3, I4, I5: The layout family is produced by AST-to-IR lowering rather than by an IR pass, the whole `SPIRVAsm*` group comes from one visitor, and the `Debug*` opcodes are split across lowering, the inliner, the debug-value-store pass, type legalization and `linkAndOptimizeIR`. | internal-source-fact | [#source](../../../../design/ir-reference/metadata.md#source) | Which C++ function creates a record is not observable from a dump, which shows only that the record exists in a given snapshot; the snapshot-ordering consequence that *is* observable is already exercised by the tests that read from `-dump-ir-after performForceInlining`. |
| I6, I7: `Layout`, `TypeLayout`, `Attr`, `SemanticAttr`, `LayoutResourceInfoAttr` and `SPIRVAsmOperand` are abstract grouping entries that define a contiguous opcode range so `as<IRAttr>()` is a range comparison, and the `Debug*` opcodes have no such grouping parent. | internal-source-fact | [#family-hierarchy](../../../../design/ir-reference/metadata.md#family-hierarchy) | The consequence the claim is about is the shape of a C++ range check; the only user-visible part -- that no instruction ever carries an abstract opcode -- is implied by every layout test in this bundle naming a concrete opcode instead. |
| I8: `IRBuilder::getTupleFieldLayoutAttr` creates the attribute with the layout as its only operand while `IRTupleFieldLayoutAttr::getLayout()` reads operand 1. | internal-source-fact | [#tuplefieldlayout-and-caselayout](../../../../design/ir-reference/metadata.md#tuplefieldlayout-and-caselayout) | Both the builder and the accessor are dormant -- nothing in the tree calls either -- so the disagreement cannot be reached from any shader; it is a note for whoever revives the opcode. |
| I9: The `DebugVar` Lua operand names are stale aliases for `(source, line, col, argIndex?)`, and the generated `getType()` shadows `IRInst::getType()`. | internal-source-fact | [#debugvar](../../../../design/ir-reference/metadata.md#debugvar) | Accessor naming is a C++-level hazard; the operand *meanings* it warns about are what the two `DebugVar` tests pin, and a dump cannot show which accessor a caller would have used. |
| I10: Three producers of opcodes on this page are outside the document's `watched_paths` and should be added. | process-doc | [#manifest-coverage](../../../../design/ir-reference/metadata.md#manifest-coverage) | A statement about this documentation set's own staleness tracking, not about compiler behavior. |

## Doc gaps observed

(none) — no new gaps were observed in this pass.

All fourteen gaps previously listed here were fixed on the documentation side
(`docs/generated/design/_meta/doc-gap-state.json`) and the answers are now in
`ir-reference/metadata.md`: the debug-level table, the `DebugSource` whole-file
operand, the `parentScope?` operand on `DebugFunction`, the `Stage` enumerator
value, the `MatrixLayoutMode` values, the raw `LayoutSize` spellings, the
`nonuniform` callout, the per-row Slang surfaces for the layout family, and the
printed-form mappings for the inline-asm operands.

This pass consumed those answers rather than re-reporting them. Every claim the
fill made concrete was run against the compiler before a CHECK was written, and
all of them reproduced: `-g0` emits no `Debug*` record, `-g1` omits
`DebugCompilationUnit` / `DebugVar` / `DebugValue` and leaves the source text
empty, `-debug-info-include-source` embeds the text at `-g1` anyway, the nine
`LayoutResourceKind` numbers map to the constructs the doc names, the
`float[4]` constant-buffer example reports `TypeAlignment(16 : Int)` with
`size(8 : Int, 52 : Int)` over an element reporting 4, and the `nonuniform`
worked example splits into exactly two `Func(Vec(Float, 4 : Int), UInt)`
functions with neither `nonuniform` nor `Attributed` anywhere in the dump.
