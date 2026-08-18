---
gap_intake_report: true
intake_model: claude-opus-5[1m]
intake_at: 2026-08-12T06:40:40Z
target_doc: ir-reference/metadata.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: 67149d1e03ebf1d4645ddd224ff4647a8ea5db53
gap_count: 14
actions:
  fixed: 14
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated_to_finding: 0
---

# Gap-intake report for ir-reference/metadata.md

## Summary

This is a re-run of the gap-intake stage covering the single gap the
previous cycle deferred, `028643968b5f` (the `nonuniform` attribute); the
other thirteen verdicts and their evidence are carried forward unchanged.
That gap is now `fixed`, so the breakdown is fourteen `fixed` and nothing
deferred, rejected or escalated. Two things had changed since the previous
cycle: `slang-ir-specialize-function-call.cpp` has been added to this
document's `watched_paths`, and a native `slangc` was in fact available, so
the question the previous cycle could not answer — whether the attribute
survives into a dumped module — was settled by compiling a shader that
reaches the producing code.

The compiler run answered it in the gap's favour but for a reason the gap's
suggested wording did not state, so the wording written into the document is
not the suggested one. The attribute is created (line 618) and immediately
wrapped in a real interned `Attributed` type (line 621), but that type only
ever becomes an entry in an `IRSimpleSpecializationKey`, which is a plain
`List<IRInst*>` used as a `Dictionary<Key, IRFunc*>` lookup — never an
operand of an emitted instruction. `nonuniform` is therefore a write-only
value: it has one producer and, at HEAD, no reader anywhere in the tree. Its
only observable effect is that two call sites differing only in a
`NonUniformResourceIndex` specialize into two distinct functions with
identical signatures, which is what the new callout shows.

The `## Manifest coverage` section was corrected in the same edit: it still
listed `slang-ir-specialize-function-call.cpp` as unwatched and said the page
"cannot say what a reader is expected to write to obtain a `nonuniform`
attribute", both of which are now false.

## Actions

| Gap ID       | Action   | Evidence                                                                                                                                                                                                                                                                                                                                                                                                                                                | Fix summary                                                                                                                       |
| ------------ | -------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------- |
| 028643968b5f | fixed    | `slang-ir-specialize-function-call.cpp` is now watched. Confirmed there: 618 creates the attr, 678 (`findNonuniformIndexInst`) is the trigger, 621 wraps it in an `Attributed` type, 622 adds that type to `ioInfo.key.vals`; the key is used only at 335/384/402 as a `Dictionary<Key, IRFunc*>` lookup, and `grep -rn NonUniformAttr source/ include/` finds no reader at all. Settled by compiling `ConstantBuffer<Params> gCB[64]` with `f(gCB[NonUniformResourceIndex(i)]) + f(gCB[i])`: `slangc -target spirv -dump-ir` gives 81 pass dumps with 0 occurrences of `nonuniform` and 0 of `Attributed` (114 of `nonUniformResourceIndex`), and `-dump-ir-after specializeResourceUsage` shows `func %f` and `func %f1`, both `Func(Vec(Float, 4 : Int), UInt)`. | named the surface shape and added a `### nonuniform` callout on the key-only, never-dumped attribute; corrected `## Manifest coverage` |
| bfb77d573b0f | fixed    | Confirmed in watched `source/slang/core.meta.slang`: `__align_attr` (1516) and `__memoryscope_attr` (1557) are `internal` and under `//@hidden:`; the public wrappers are `loadAligned` (1536), `storeAligned` (1550), `storeCoherent` (1573) and `loadCoherent` (1586), the coherent pair carrying `[require(SPV_KHR_vulkan_memory_model)]` at 1570 and 1582. Watched `slang-emit-spirv.cpp:8793` asserts the Vulkan memory model when a `MemoryScope` reaches emit. | replaced the `__*_attr` citations with the public wrappers, added a capability paragraph, and corrected the `emitLoad`/`emitStore` line numbers |
| a43e4751357e | fixed    | Confirmed in watched `source/slang/slang-ir-insts.h:1645` — `IRStageAttr::getStage()` returns `Stage(getIntVal(getStageOperand()))`, so the literal is a `Slang::Stage` enumerator. The value comes from `include/slang.h:913` (`SLANG_STAGE_COMPUTE = 6`), which `slang-profile.h:52` re-exports as `Stage`; recorded in `## Manifest coverage` as an unwatched anchor.                                                                              | named the `Stage` enumeration in the `stage` row and gave `stage(6 : Int)` as the compute value                                    |
| 8b396dac437f | fixed    | Source agrees with the observation, so the document was wrong, not the compiler. Watched `slang-ir-insts.h:2827` declares `getParentScope()` reading operand 5 when `getOperandCount() > 5`; watched `slang-ir.cpp:3655-3671` emits the 6- or 5-operand form rather than a null; watched `slang-lower-to-ir.cpp:14700-14719` passes the source file's `DebugCompilationUnit`, which is null at Minimal *and* for an included / `#line`-remapped source. | added `parentScope?` to the `DebugFunction` row and a `### DebugFunction` callout covering both absence cases                      |
| e1a31870fa3e | fixed    | All gating confirmed in watched `slang-lower-to-ir.cpp`: 15458 (nothing at `None`), 15471 (source text only at `Standard`+, or under `-debug-info-include-source`), 15481 (`DebugCompilationUnit` at `Standard`+, non-included files), 15596 (`insertDebugValueStore` at `Standard`+), 11977 (`let` `DebugVar` at `Standard`+), 9927 (`DebugLine` at `Minimal`+). Watched `slang-emit.cpp:1052-1054` ties `-g0` to `DebugInfoLevel::None` and `stripDebugInfo`. `-g` defaulting to Standard is `slang-options.cpp:2619-2621`; corroborated by the bundle's own `-g` / `-g1` test split. | added a debug-level paragraph to `### Debug info family` mapping each level to the records it produces (also covers 8dd64f408ea2, 1079d955aaa9) |
| 8dd64f408ea2 | fixed    | Confirmed in watched `source/slang/slang-emit.cpp:1022-1032`: the `emitDebugBuildIdentifier` call sits inside `if (targetCompilerOptions.shouldEmitSeparateDebugInfo())`. That predicate is `CompilerOptionName::EmitSeparateDebug` (`slang-compiler-options.h:375`), spelled `-separate-debug-info` at `slang-options.cpp:949`; the bundle's `debug-build-identifier-with-separate-debug-info.slang` passes exactly that flag. | named `-separate-debug-info` in the `DebugBuildIdentifier` row and corrected its line citation to 1031                             |
| 1079d955aaa9 | fixed    | Confirmed in watched `slang-lower-to-ir.cpp:15466-15474`, which passes `source->getContent()` as operand 1 at `Standard`+ and an empty `UnownedStringSlice()` otherwise, and at 9735 in `getOrEmitDebugSource` for the same rule. The bundle's `debug-source-records-path-and-include-flag.slang` wildcards that operand for exactly this reason.                                                                                                     | noted the whole-file operand in the `DebugSource` row and added a `### DebugSource` callout on the echoed input                    |
| 32069964919e | fixed    | The family contract in `_meta/prompts/_common.md` fixes the six table columns, so the surfaces are given as prose rather than a seventh column. Each surface is pinned by a passing test in the reporting bundle: `parameter-group-type-layout-cbuffer.slang`, `array-type-layout-element-layout-first.slang`, `matrix-type-layout-mode-operand.slang`, `ptr-type-layout-omits-pointee-layout.slang`, `existential-type-layout-interface-field.slang`, `structured-buffer-type-layout-element-operand.slang`, `type-layout-fallback-attrs-only.slang`, `stream-output-type-layout-geometry.slang`, `struct-type-layout-field-attrs-in-declaration-order.slang`, `varlayout-type-layout-then-attr-tail.slang`, `entry-point-layout-two-varlayout-operands.slang`. | added a paragraph after the Layout table giving the minimal Slang declaration for each row                                         |
| e974ee53af16 | fixed    | Confirmed in watched `slang-ir-insts.h:1439-1441` — `IRMatrixTypeLayout::getMode()` casts operand 0 to `MatrixLayoutMode`. The two values are `include/slang.h:899-901` (`SLANG_MATRIX_LAYOUT_ROW_MAJOR = 1`, `..._COLUMN_MAJOR = 2`) and are pinned by the bundle's `matrix-type-layout-mode-operand.slang`, which compiles the same shader twice and requires `matrixTypeLayout(1 : Int, ...)` under `-matrix-layout-row-major` and `(2 : Int, ...)` under `-matrix-layout-column-major`. | gave `1`/`2` in the `matrixTypeLayout` row and named the two options in the surface paragraph                                      |
| 8eab4b5f6ea9 | fixed    | Printed shape derived from watched `slang-ir.cpp`: `dumpIRDecorations` (7905-7915) wraps each decoration in `[...]` on its own line; `getVarLayout` (7528-7535) builds layout insts with `getVoidType()`; `shouldFoldInstIntoUses` (7822-7858) folds only constants, types and `SPIRVAsmOperand`s, so a layout inst is always printed as a separate `let %N` definition and referenced by id. Corroborated by the bundle's `layout-decoration-reaches-concrete-layout-child.slang`. Operand text elided rather than invented. | added a four-line dump excerpt and a sentence on the `Void`-typed, never-folded layout insts to §`Layout`                          |
| 74ef1e3a36aa | fixed    | Confirmed in watched `slang-ir.cpp:7404-7407` — `getTypeSizeAttr` writes `size.unsafeGetRaw()` straight into a signed `IRIntLit`, so the `size_t` sentinels surface as negative literals. The sentinels are `slang-type-layout.h:248-249` (`s_infiniteValue = RawValue(-1)`, `s_invalidValue = RawValue(-2)`). The `-1` spelling is pinned by the bundle's `size-attr-unsized-array-raw-value.slang`. | stated the `-1` / `-2` raw spellings in §`size` and `offset` (one edit with 75a053a8073a)                                          |
| 75a053a8073a | fixed    | `LayoutResourceKind` is `typedef slang::ParameterCategory` (`slang-type-layout.h:589`), whose enumerators are `include/slang.h:2230-2244`. The two the gap names are pinned by the bundle's `size-attr-resource-kind-first.slang`: a `float4` reports `size(8 : Int, 16 : Int)` (16 bytes, kind 8 Uniform) and a texture+sampler pair `size(9 : Int, 2 : Int)` (2 slots, kind 9 DescriptorTableSlot). | added the practical kind enumerators and a worked byte-vs-slot reading to §`size` and `offset`                                    |
| 5fbf90b33fd2 | fixed    | Confirmed in watched `slang-ir.cpp`: `dumpInstBody` routes `kIROp_SPIRVAsm` to `dumpIRParentInst` (8310), which prints a typed one-line header then an indented brace block (8041-8074); `shouldFoldInstIntoUses` returns true for every `IRSPIRVAsmOperand` (7855), so operand instructions are printed inline; `emitSPIRVAsmInst` (7294) gives the child a `Void` type, so `opHasResult` (7646) is false and no `let %N =` prefix appears. Opcode numbers from `external/spirv-headers` (`OpExtInst = 12`, `OpFMul = 133`, `GLSLstd450Sqrt = 31`); shape corroborated by `spirv-asm-parent-owns-inst-children.slang` and `spirv-asm-operand-result-and-named-id.slang`. | added a printed-form paragraph and a two-instruction dump excerpt to §SPIR-V inline asm                                            |
| 9b9e225bc5b0 | fixed    | Confirmed in watched `slang-ir.cpp:8222-8280`, the `dumpInstExpr` switch: `SPIRVAsmOperandEnum` / `Literal` / `Inst` print only `getOperand(0)`'s expression (8224-8232), which is exactly why an enumerator and a literal are indistinguishable; `Id` prints `%` plus the quoted string, `Result` prints `result`, `Truncate` prints `__truncate`, the type functions and the three `...FromLocation` kinds print as named calls; anything not in the switch falls through to `opInfo.name` plus its operand list (8278-8279), and `dumpInstOperandList` (7997-8002) emits no parentheses for a nullary op. Printed forms corroborated by `spirv-asm-operand-builtin-var.slang`, `spirv-asm-operand-glsl450-set.slang` and `spirv-asm-truncate-as-opcode-operand.slang`. | added a bulleted printed-form mapping to §`SPIRVAsmOperand`, stating which kinds a dump cannot tell apart                          |
