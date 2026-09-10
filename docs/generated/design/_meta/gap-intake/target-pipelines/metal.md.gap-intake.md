---
gap_intake_report: true
intake_model: claude-opus-5[1m]
intake_at: 2026-08-11T16:23:37Z
target_doc: target-pipelines/metal.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: ec47ea72b6aa5fefc3b36f8a780dbd3ecf5b1f6e
gap_count: 10
actions:
  fixed: 7
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 2
  escalated_to_finding: 1
---

# Gap-intake report for target-pipelines/metal.md

## Summary

One gap was escalated: `e01b0a239000` asks the document to record
that the two parameters synthesized for an `AppendStructuredBuffer`
carry no `[[buffer(N)]]` slot, which the emitter source says they
should, so it is a compiler defect already tracked by
`metal-append-buffer-params-missing-binding-slot`. Seven gaps were
fixed with edits confirmed against watched paths — the subpass
diagnostic's real trigger and the capability check that shadows it,
a fragment `out`-parameter before/after, the emitted byte-address
load/store form, the capability status of `NonUniformResourceIndex`
on Metal, the unreachability of `double` at Metal emit, a new
"Entry-point shape" subsection, and a new "Resource-type spellings"
mapping table. Two gaps were deferred because the source that would
settle them lives outside `watched_paths`
(`slang-ir-lower-buffer-element-type.cpp` for the packed-matrix
column-count question, and `slang-ir-wrap-cbuffer-element.cpp` plus
`slang-parser.cpp` for the cbuffer wrapper's emitted struct names).
Two `Suggested addition`s were corrected rather than transcribed:
`74f4ac69bb7b` claimed Metal has *no* surface for
`floatNonUniformResourceIndex`, but `nonuniform()` on a
`DescriptorHandle<T>` carries no `[require]` and reaches it; and
`5fb5b210d783` claimed `[numthreads]` is dropped outright, whereas
it becomes `[[required_threads_per_threadgroup]]` under
`metallib_4_0`.

## Escalated gaps

- **`e01b0a239000`** — `lowerAppendConsumeStructuredBuffers` on
  Metal. `MetalSourceEmitter::emitFuncParamLayoutImpl`
  (`source/slang/slang-emit-metal.cpp:166-176`) emits
  `[[buffer(N)]]` for any parameter whose type is an
  `IRPtrTypeBase` / structured-buffer / byte-address-buffer /
  parameter-group / acceleration-structure and whose layout carries
  a `LayoutResourceKind::MetalBuffer` offset. The element and
  counter pointers the pass synthesizes are ordinary `device*`
  buffers and satisfy that type test, so by the source they should
  receive slots. The compiler emits both without any attribute
  while the plain `RWStructuredBuffer` beside them takes
  `[[buffer(3)]]` — the slots the two would have occupied are
  allocated but never written. Existing finding:
  `docs/generated/tests/_meta/findings/metal-append-buffer-params-missing-binding-slot.yaml`.
  Documenting the observed parameter list would bless invalid MSL,
  so the document was not edited. The descriptive half of the gap
  (the `_elements` / `_counter` split, the atomic counter bump)
  additionally needs `slang-ir-lower-append-consume-structured-buffer.cpp`,
  which is not in `watched_paths`.

## Actions

| Gap ID | Action | Evidence | Fix summary |
| --- | --- | --- | --- |
| baa2d8f9c3ec | fixed | `source/slang/slang-ir-metal-legalize.cpp:298-303` (diagnosed when no fragment entry point reaches the global) and `:364-376` (per-use arm); `source/slang/hlsl.meta.slang:22679` and `:22692` carry `[require(glsl_hlsl_metal_spirv, subpass)]`; `source/slang/slang-diagnostics.lua:2452-2454` defines `36107` as "unavailable features in entry point". The fragment-only definition of the `subpass` atom is at `source/slang/slang-capabilities.capdef:1622`, **outside `watched_paths`**. | rewrote the subpass bullet to state the two real trigger conditions and note that user-written non-fragment uses are rejected first by the entry-point capability check (`36107`), making the pass diagnostic defence in depth |
| 9f215724380b | fixed | `source/slang/slang-ir-legalize-varying-params.cpp:5090-5117` (`legalizeShaderOutputParamsForMetal` calls `lowerOutParameters` with `alwaysUseReturnStruct = true`) and `:5150-5156` (vertex **and** fragment dispatch); emitted shape verified by the reporting bundle's `fragment-out-target-becomes-color-attribute.slang` CHECK lines (<code>struct main_Result_&#123;&#123;[0-9]+&#125;&#125;</code>, `color(0)`, <code>fragment&#123;&#123;.*&#125;&#125; main_Result_&#123;&#123;[0-9]+&#125;&#125; main_0</code>, `position`); `_<N>` uniquing at `source/slang/slang-emit-c-like.cpp:1151-1190`. | added a minimal Slang/MSL before-after pair under the stage-trigger paragraph showing the synthesized `main_Result_0` struct and the `[[fragment]]` signature |
| d0badd8620c9 | deferred | The column-count question is decided by `MetalBufferElementTypeLoweringPolicy::needsPackedVectorStorage` / `usesPackedVectorStorage` in `source/slang/slang-ir-lower-buffer-element-type.cpp:2984-3010`, which is **not in `watched_paths`**; nothing in the watched set states the rule. (For the operator: the answer is *every* vector of more than one element, so a `float4x4` device-buffer field does become `packed_float4`; and only for `Natural`-layout buffers in the `StorageBuffer` / `UserPointer` address spaces — constant and argument buffers keep native MSL layout.) Follow-up: add `source/slang/slang-ir-lower-buffer-element-type.cpp` to `watched_paths`, then state the rule with one example. | — |
| e01b0a239000 | escalated-to-finding | See `## Escalated gaps`. Finding `metal-append-buffer-params-missing-binding-slot` already covers it; source expectation at `source/slang/slang-emit-metal.cpp:166-176`. | — |
| 74f4ac69bb7b | fixed | `source/slang/slang-emit.cpp:2275` confirms the `!isSPIRV` gate as documented, so this is not a doc-vs-source contradiction. `source/slang/hlsl.meta.slang:13946-13949` shows `NonUniformResourceIndex` declared `[require(cpp_cuda_glsl_hlsl_spirv, nonuniformqualifier)]`, excluding Metal. The gap's "no Slang surface on Metal" is too strong: `source/slang/hlsl.meta.slang:27817-27818` declares `nonuniform<T:IOpaqueDescriptor>(DescriptorHandle<T>)` with the same `__intrinsic_op($(kIROp_NonUniformResourceIndex))` and **no** `[require]`. | added a note to Phase-C row 7 recording the capability exclusion of the HLSL spelling (error `36107`) and naming `nonuniform()` on a `DescriptorHandle<T>` as the reachable Metal surface |
| 0b7cd7836638 | fixed | `source/slang/slang-emit-metal.cpp:925-940` (`ByteAddressBufferLoad` → `as_type<T>(buf[(offset)>>2])`), `:941-954` (`ByteAddressBufferStore` → `buf[(offset)>>2] = as_type<uint32_t>(v)`), `:1476-1480` (byte-address buffers emit as `uint32_t device*`); both emitter comments state wider element types must already have been lowered, which is what the option set does. Corroborated by `byte-address-buffer-load-store-lowering.slang`. | appended the emitted load/store forms and the `uint32_t device*` buffer spelling to the `legalizeByteAddressBufferOps` row's Notes |
| aee9d597c1d1 | fixed | `source/slang/slang-emit-metal.cpp:1191-1214` shows the `BaseType::Double` arm falling through to no suffix, so the doc matched that code; `:1301-1303` shows `emitSimpleTypeImpl` answering `kIROp_DoubleType` with `SLANG_UNEXPECTED("'double' type emitted")`, so the arm is unreachable whenever the type must be spelled. Non-finite spellings confirmed at `:1172-1186`. The abort itself is already recorded as finding `types-double-member-metal-wgsl-emit-abort`. | replaced the `BaseType::Double` clause with a statement that `double` is not an emittable Metal type, gave the exact NaN/infinity spellings, and flagged the `E99997` abort as a known gap |
| 5fb5b210d783 | fixed | `source/slang/slang-emit-c-like.cpp:1130-1143` renames an entry point named exactly `main` on Metal / CPU / CUDA and reports `Diagnostics::MainEntryPointRenamed`, defined as warning `40100` at `source/slang/slang-diagnostics.lua:5952-5957`; `source/slang/slang-emit-metal.cpp:218-264` shows the stage attributes and that a threadgroup-size attribute is emitted **only** under `metallib_4_0`, as `[[required_threads_per_threadgroup(...)]]`. | added an "Entry-point shape" subsection to Phase D covering the `main` rename with warning `40100`, the stage-attribute set, and the conditional `[[required_threads_per_threadgroup]]` replacement for `[numthreads]` |
| aec242e9392f | fixed | `source/slang/slang-emit-metal.cpp:58-133` (`_emitHLSLTextureType`: `texture`/`depth` prefix, rank stem, `_ms` / `_array` order, `access::sample` / `read` / `write` / `read_write` selection), `:1335-1340` (samplers), `:1348-1360` (`RayQuery`, `ConstantBuffer` / `ParameterBlock`), `:1453-1490` (`texture_buffer`, `T device*`, `uint32_t device*`, acceleration structure), `:154-192` (attribute-kind selection incl. `[[stage_in]]` / `[[payload]]`), `:676-691` (`MetalCastToDepthTexture`). | added a "Resource-type spellings" subsection to Phase D with a Slang-type → MSL-type → attribute table and a note on the `depth2d` cast route for `SampleCmp*` |
| 726d790825ac | deferred | The two struct names the gap asks to codify are not produced by `wrapCBufferElementsForMetal`: `ParameterGroup_<name>` comes from `source/slang/slang-parser.cpp:4066` and the `_MatrixStorage_...natural_<N>` shape from `source/slang/slang-ir-lower-buffer-element-type.cpp:446`. Neither file, nor `slang-ir-wrap-cbuffer-element.cpp`, is in `watched_paths`, so the example could only be written by mis-attributing output to this pass. `cbuffer-matrix-storage-wrap.slang` verifies the two names appear but not which pass emits them. Follow-up: add `slang-ir-wrap-cbuffer-element.cpp` and `slang-ir-lower-buffer-element-type.cpp` to `watched_paths`, then split the example between the two owning passes. | — |
