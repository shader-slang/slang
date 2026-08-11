---
gap_intake_report: true
intake_model: claude-opus-5[1m]
intake_at: 2026-08-11T16:17:50Z
target_doc: ir-reference/types.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: ec47ea72b6aa5fefc3b36f8a780dbd3ecf5b1f6e
gap_count: 12
actions:
  fixed: 11
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 1
  escalated_to_finding: 0
---

# Gap-intake report for ir-reference/types.md

## Summary

Twelve gaps, all reported by `design/ir-reference/types`. Eleven were confirmed
in source and fixed; one is deferred. Nothing was escalated — none of the twelve
turned out to be a compiler defect, and in the one `drift-from-source` case
(`SPIRVUntypedPtr`) the source agreed with the observation, not with the
document: the retyping is gated behind the `spvDescriptorHeapEXT` capability
arm of `defaultGetDescriptorFromHandle`, so a plain `-target spirv` compile
correctly produces no untyped pointer.

Five of the gaps asked, in different sections, "what does this print in
`-dump-ir`?". They are answered once, by a new paragraph in the `## Opcodes`
preamble that states the rule from `shouldFoldInstIntoUses`: every type is
folded into its use sites and printed as `Mnemonic(operand, ...)`, except the
four nominal opcodes `struct`, `class`, `GLSLShaderStorageBuffer` and
`interface`, which get their own line; and only the first three of those are
also printed with children in braces. Per-section notes then say only what is
specific to the section.

Two fixes correct claims the document had wrong rather than merely missing.
`Enum` did **not** encode its cases as child instructions — `createEnumType`
builds the inst with the tag-type operand alone and `visitEnumCaseDecl` lowers a
case to an ordinary constant typed by the `Enum`, so there is nothing to walk.
And `Result` has no AST class and no source spelling at all; it is created by
the error-handling pass out of a `throws` clause. Both were found by confirming
the gap's premise against the watched source before writing anything.

Two operator notes. The deferred gap and several confirmations needed source
outside this page's `watched_paths` (named per row below); and the `## Source`
paragraph's standing recommendation to add `core.meta.slang` and
`hlsl.meta.slang` to `watched_paths` is now stale — both are already watched.

## Actions

| Gap ID | Action | Evidence | Fix summary |
| --- | --- | --- | --- |
| 60366ad2b2ce | fixed | Confirmed in watched `source/slang/core.meta.slang`: `extension intptr_t` / `extension uintptr_t` at lines 1682 and 1692 (under `//@public:` from line 1642), and the typedefs `size_t` / `usize_t` / `ssize_t` at lines 20-24. The names come from the base-type table in `source/slang-core-module/slang-embedded-core-module-source.cpp:107,150`. Both types are shader-reachable; "host-side only" would have been wrong. | added the `intptr_t` / `uintptr_t` spellings to both AST-origin cells and the `size_t` / `ssize_t` aliases to their summaries |
| dcbf50926593 | deferred | Blocked on `watched_paths`. Which of the `Minimal` / ordinary / `Trivial` context types is used is decided in `source/slang/slang-check-decl.cpp` (~3730, 15165-15270, 19526), `source/slang/slang-ir-translate.cpp:167-232` and `source/slang/slang-ir-autodiff-fwd.cpp:3704,3755` — none of which this page watches, and none of which is reachable from what it does watch (`slang-ir.cpp:3115` only builds the type). A first read also suggests the gap's premise is off: the checker synthesizes the full (`BwdCallable`) and `MinimalContext` structs together for every differentiable function rather than one per user construct, so writing the requested per-variant surface examples would need both a `watched_paths` expansion and a compiler run to verify. I cannot run `slangc` (Linux x86-64 build, arm64 host). | — |
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
