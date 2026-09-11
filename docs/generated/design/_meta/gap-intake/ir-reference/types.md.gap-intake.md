---
gap_intake_report: true
intake_model: claude-opus-5[1m]
intake_at: 2026-08-12T00:00:00Z
target_doc: ir-reference/types.md
target_doc_source_commit_before: ec47ea72b6aa5fefc3b36f8a780dbd3ecf5b1f6e
target_doc_source_commit_after: ec47ea72b6aa5fefc3b36f8a780dbd3ecf5b1f6e
gap_count: 12
actions:
  fixed: 12
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated_to_finding: 0
---

# Gap-intake report for ir-reference/types.md

## Summary

This is a **re-run** of the gap-intake stage, revisiting the single gap the
first pass left `deferred`. That gap, `dcbf50926593`, is now `fixed`, so the
queue of twelve is twelve `fixed` and nothing else — no rejections, no
deferrals, no escalations. The eleven gaps the first pass already settled were
not revisited; their verdicts and `Evidence` are carried forward verbatim
below.

Both of the first pass's stated blockers on `dcbf50926593` turned out to be
false. It said the deciding source — `slang-check-decl.cpp`,
`slang-ir-translate.cpp`, `slang-ir-autodiff-fwd.cpp` — was outside
`watched_paths`; `regenerate.py show ir-reference/types.md` now resolves all
three. It also said `slangc` could not be run on this host; a native
macOS-arm64 build at `build-arm64/Debug/bin/slangc` (`2026.14.1-80-g6122d03def`)
compiles and dumps IR fine. Both the source read and three `-dump-ir` runs went
into the fix.

The gap's own premise was the part that held up: the document could not say
what user-level construct selects a context-type variant. What the compiler
showed is that the question was mis-framed by the document, not by the gap.
`Minimal` and ordinary are not competing variants at all — the checker
synthesizes a `MinimalContext` **and** a `BwdCallable` struct for every
differentiable function, and both appear as entries of the same
`IBackwardDifferentiable` witness table. What the surface actually selects is
the *family* prefix, at semantic-checking time: `[Differentiable]` /
`[BackwardDifferentiable]` gives the plain pair, `[TreatAsDifferentiable]` the
`Trivial*` pair, and `[BackwardDerivative(fn)]` / `[BackwardDerivativeOf(fn)]`
the `FromLegacy*` pair. The document's claim that "the specialization pass"
chooses "based on whether the propagation strategy needs full state, minimal
state, or nothing" was therefore wrong on both counts, and is replaced.

One item outside the gap queue was also corrected, as the operator asked: the
`## Source` section still recommended adding `core.meta.slang`,
`hlsl.meta.slang` and `workgraph.slang` to `watched_paths`, which
`regenerate.py show` resolves as already watched. That paragraph now records
`source/slang/slang-ir.h.lua` as the one genuinely missing path.

## Actions

| Gap ID | Action | Evidence | Fix summary |
| --- | --- | --- | --- |
| 60366ad2b2ce | fixed | Confirmed in watched `source/slang/core.meta.slang`: `extension intptr_t` / `extension uintptr_t` at lines 1682 and 1692 (under `//@public:` from line 1642), and the typedefs `size_t` / `usize_t` / `ssize_t` at lines 20-24. The names come from the base-type table in `source/slang-core-module/slang-embedded-core-module-source.cpp:107,150`. Both types are shader-reachable; "host-side only" would have been wrong. | added the `intptr_t` / `uintptr_t` spellings to both AST-origin cells and the `size_t` / `ssize_t` aliases to their summaries |
| dcbf50926593 | fixed | Re-run of a previously `deferred` gap; both stated blockers were false. All three deciding files are now watched (`regenerate.py show ir-reference/types.md` resolves `source/slang/slang-check-decl.cpp`, `source/slang/slang-ir-translate.cpp`, `source/slang/slang-ir-autodiff-fwd.cpp`), and `build-arm64/Debug/bin/slangc` runs natively. Confirmed in watched `slang-check-decl.cpp`: the `BackwardDifferentiableAttribute` arm at `:15142` sets `kIROp_BackwardDiffIntermediateContextType` (`:15169`) and `kIROp_BackwardDiffMinimalContextType` (`:15179`); the `TreatAsDifferentiableAttribute` arm at `:15227` sets the `Trivial*` pair (`:15254,15264`); `translateBwdDerivativeAttributeToAD2` at `:19210` sets the `FromLegacy*` pair (`:19261,19272`). Surface spellings are watched `core.meta.slang:470` (`attribute_syntax [Differentiable(order:int = 0)] : BackwardDifferentiableAttribute`), `:451` (`[BackwardDifferentiable]`) and `:131` (`[TreatAsDifferentiable]`). Verified by running `SLANG_ASSERT=release-assert-only build-arm64/Debug/bin/slangc -dump-ir -target hlsl -entry computeMain ctx.slang -o ctx.hlsl` on a shader with `[Differentiable] float f` and `[TreatAsDifferentiable] float g` both called through `bwd_diff`: the dump shows `witness_table_entry(%30,BackwardDiffIntermediateContextType(%f))` and `witness_table_entry(%29,BackwardDiffMinimalContextType(%f))` in one witness table and the `Trivial*` pair in `g`'s, plus `Func(tuple_type(Float, BackwardDiffMinimalContextType(%f)), Float) = BackwardDifferentiatePrimal(%f)` and `Func(BackwardDiffIntermediateContextType(%f), BackwardDiffMinimalContextType(%f), Float) = BackwardRemat(%f)`. The same command on `[BackwardDerivative(h_bwd)]` and on `[BackwardDerivativeOf(k)]` shaders produced `BackwardContextFromLegacyBwdDiffFunc(%h, %h_bwd_diff)` / `BackwardMinimalContextFromLegacyBwdDiffFunc(...)`. The document's "specialization pass chooses" claim is a documentation defect, not a compiler one: `slang-check-decl.cpp:3727-3728` says only the *contents* are decided later. | replaced the §Differentiation types preamble's missing surface with an attribute-to-opcode table for the three families, the both-are-synthesized correction with its witness-table dump lines, and the `apply_bwd` / `remat` / propagate role split; rewrote the contradicting sentence in §`BackwardDiffIntermediateContextType` to match |
| 4a3bac7372ce | fixed | The gap's premise is false and the document was wrong. Confirmed in watched `slang-lower-to-ir.cpp:12352-12378` (`visitEnumDecl` creates the type from the tag type and adds nothing else), `:12334-12350` (`visitEnumCaseDecl` lowers a case to the value of its tag expression) and `slang-ir.cpp:5337-5342` (`createEnumType` passes only `tagType`). Nothing in `source/` gives an `Enum` inst children; `slang-ir-lower-enum-type.cpp:124-126` confirms a case is an `IRIntLit` *typed by* the enum. The printed form `Enum(Int)` follows from `slang-ir.cpp:7852` (types fold) and is pinned by the bundle test `enum-tag-type.slang`. | rewrote §`Enum` and its table row: cases are neither operands nor children, `P` only means the opcode may hold children, and the type prints as `Enum(Int)` |
| 77f10c369661 | fixed | Confirmed in watched `slang-ir.cpp:7840-7850` (`InterfaceType` is one of four opcodes excluded from folding) and `:8296-8311` (`dumpInstBody`'s parent-inst special case covers `WitnessTable` / `StructType` / `ClassType` / `GLSLShaderStorageBufferType` / `SPIRVAsm` but **not** `InterfaceType`, so `interface` takes the ordinary `let %I : Type = interface(...)` path), plus `slang-lower-to-ir.cpp:12124-12130` (`createInterfaceType(operandCount, nullptr)` then `setInsertBefore(irInterface)` for the entries). The entry's printed shape is pinned by the bundle test `interface-and-this-type.slang`. | added the generalized printing rule to the `## Opcodes` preamble and a note under §Existentials and interfaces with the `interface_req_entry(%key, Func(Float, this_type(%IShape)))` form; same edit serves 4a3bac7372ce and 173c31f83602 |
| 173c31f83602 | fixed | Confirmed in watched `source/slang/hlsl.meta.slang`: the six markers are `IBufferDataLayout` implementations at lines 23-71, and `L` is the second generic parameter of `StructuredBuffer<T, L : IBufferDataLayout = DefaultDataLayout>` at line 5970, so `lowerSimpleIntrinsicType` (`slang-lower-to-ir.cpp:2885-2908`) puts the marker in operand 1. The printed forms `StructuredBuffer(Float, Std140Layout, ...)` and `RWStructuredBuffer(Float, DefaultLayout, ...)` are pinned by the bundle test `buffer-layout-marker-operands.slang`. | added a paragraph showing `StructuredBuffer<float, Std140DataLayout>` landing in the data-layout operand, plus the `[require(spirv)]` gate on two of the six |
| a5bba1b768e1 | fixed | Source agrees with the observation, so the document was wrong. Watched `hlsl.meta.slang:27708-27807`: `defaultGetDescriptorFromHandle` `__target_switch`es on capability and only `case spvDescriptorHeapEXT` (line 27797) emits `__spirvLoadDescriptorFromHeap`; the default arm is `__castDescriptorHandleToResource`. `slang-ir-spirv-legalize.cpp:1339-1357` only rewrites an `IRSPIRVLoadDescriptorFromHeap` typed by a `ConstantBuffer`, so with no such inst the retyping never fires. `spvDescriptorHeapEXT = SPV_EXT_descriptor_heap + SPV_KHR_untyped_pointers` (`source/slang/slang-capabilities.capdef:977`). Both non-meta files are outside `watched_paths`. Not a compiler bug: the observed absence is the documented default path. | named the missing condition in §`SPIRVUntypedPtr` — the capability arm that emits the heap load — and stated that a plain `-target spirv` heap fetch emits no untyped pointer |
| bb807f37d4bb | fixed | Confirmed in watched `core.meta.slang:1705-1751`: `FloatE4M3`, `FloatE5M2` and `BFloat16` are public `struct`s (the `//@public:` marker at line 1642 governs), each conforming only to `IFloatingPointCoopElement` — not to `__BuiltinArithmeticType` — with widening supplied by the `extension<T : __BuiltinFloatingPointType>` constructors at line 1754, and each carrying `[require(spvFloat8EXT)]` / `[require(cuda_sm_8_9)]` or `[require(spvBFloat16KHR)]` / `[require(cuda_sm_8_0)]`. The struct-field usage is pinned by the bundle test `storage-only-float-struct-fields.slang`. | added a paragraph with the `struct S { BFloat16 b; FloatE4M3 e; FloatE5M2 f; }` example, the `float(...)` conversion requirement, and the capability gates |
| 57da50efca74 | fixed | Confirmed, but three of the four confirming files are outside `watched_paths`. `source/standard-modules/experimental/workgraph.slang:9-11` declares `[ExperimentalModule] module workgraph;`; watched `slang-lower-to-ir.cpp:15449-15451` turns that attribute into `ExperimentalModuleDecoration`, and `source/slang/slang-session.cpp:1766-1774` diagnoses `NeedToEnableExperimentFeature` when the module carries it and `CompilerOptionName::ExperimentalFeature` is unset. `source/slang/slang-capabilities.capdef:1532` defines `alias node = _node + _sm_6_8`, which is why SM 6.8 is needed. The full flag set is exercised by all six `workgraph-*.slang` tests in the bundle. | added a paragraph to §The work-graph record types listing the four requirements: `import experimental.workgraph;`, `-experimental-feature`, a `[shader("node")]` entry point at `-stage node`, and `-profile lib_6_8` |
| 53d93b30593a | fixed | Confirmed in watched `hlsl.meta.slang:27590-27607`: the `__subscript(uint index)` getters on `__ResourceDescriptorHeapType` / `__SamplerDescriptorHeapType` are declared to return `UntypedResourceHandle` / `UntypedSamplerHandle`, so the value produced by indexing the heap carries the type. The observed dump shape (handle-typed value feeding a call, concrete resource type only on the conversion result) is pinned by the bundle tests `untyped-resource-handle.slang` and `untyped-sampler-handle.slang`. | added the observation point to §`UntypedResourceHandle` and `UntypedSamplerHandle` — visible on the heap-subscript value in a lowering-stage `-dump-ir`, gone by emit |
| ce8aaa00510a | fixed | Confirmed in watched `core.meta.slang:2295-2296` (`kRowMajorMatrixLayout` / `kColumnMajorMatrixLayout` spliced from `SLANG_MATRIX_LAYOUT_ROW_MAJOR` / `_COLUMN_MAJOR`) and `:2301` (`matrix<T, R, C, L>`'s fourth parameter defaults to `SLANG_MATRIX_LAYOUT_MODE_UNKNOWN`), plus watched `slang-lower-to-ir.cpp:2858-2865`, which lowers the AST layout value straight into operand 3. The integers themselves are `include/slang.h:899-901` (0/1/2); the check-time pinning is `slang-check-expr.cpp:9305-9325` and the unknown-to-default rewrite is `slang-ir-specialize-matrix-layout.cpp:28-46` (run from `slang-emit.cpp:1366`) — those three are outside `watched_paths`. | gave the three `SlangMatrixLayoutMode` values and their meanings in §`Vec` and `Mat`, plus who sets `0` and what later resolves it |
| 5c7ca9a187c9 | fixed | The gap's observation is confirmed by watched source. `slang-lower-to-ir.cpp:2997-3003` (`visitAndType`) builds the `Conjunction` from an `AndType`, but `emitGenericConstraintValue` at `:12789-12810` decomposes a conjunction sup-type into one `getWitnessTableType(caseType)` parameter per case and returns a `MakeTuple` of them, so a `T : IA & IB` constraint leaves separate witness-table constraints and no `Conjunction` in the generic's signature — exactly what the reporting test saw. | named the `&` spelling in the `Conjunction` AST-origin cell and recorded the constraint decomposition that hides the opcode |
| 2dfdf40e0637 | fixed | The document was wrong: there is no `ResultType` AST class (nothing matches in `source/slang/slang-ast-type.h`) and no `Result` in the core module. Watched `slang-lower-to-ir.cpp:4809-4819` and `:2733-2747` show a `throws E` clause lowering to `Func(T, params..., FuncThrowTypeAttr(E))`, and `source/slang/slang-ir-lower-error-handling.cpp:43,93,153,174` is the only producer of the `Result` type, rewriting that into `Func(Result(T, E), params...)` — matching the bundle test `result-type-from-throws.slang`. The error-handling file is outside `watched_paths`. | replaced the `Result` row's AST origin with the real producer (the `throws` clause plus the error-handling pass) and gave the resulting `Func(Result(Int, Enum(...)), Int)` shape |
