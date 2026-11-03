---
review_report: true
reviewer_model: gpt-6
reviewed_at: 2026-11-03T08:28:26Z
target_doc: ir-reference/resources-and-atomics.md
target_doc_source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_watched_paths_digest: 1fa3d4303ad1d2606cdce5b69a00ccaa688357f1c910fa3166db1700ae852df5
source_commit: 801b9e6e57d41f5cef1a5d54c0eeff9e0db4fee6
checklist:
  factual_accuracy: fail
  cross_references: partial
  completeness: partial
  style_consistency: partial
  source_alignment: fail
  front_matter_validity: pass
finding_count: 10
severity_breakdown:
  critical: 5
  major: 3
  minor: 2
  nit: 0
---

# Review report for ir-reference/resources-and-atomics.md

## Summary

The document needs correction before approval. Most cited declarations and producer locations match the source, but several behavioral claims contradict it. Most importantly, the claim that optional accessors in hand-written wrappers are unguarded is false: both generation paths return null when those operands are absent.

## Items checked

- Read the review prompt, common contract, document-specific prompt, and resolved watched paths through `regenerate.py show`.
- Verified against the **current working tree**. The source diff against the document’s recorded commit is empty for `source/`, consistent with the supplied equivalence.
- Checked more than 10 substantive claims, including intrinsic lowering, all 13 atomic children, memory-order positions, image-store operands, buffer-store operand ordering, wrapper generation, Metal mesh producers, resource-query callers, and layout attachment.
- Inspected every source line-number citation located in the document, including the Lua region table, inline references, and references continued across lines.
- Checked all 91 opcode rows’ hand-written/generated wrapper markings; all match the header.
- Resolved all 187 Markdown link occurrences to existing files where applicable, checked heading fragments, and confirmed referenced generated pages are manifest entries. Dependency documents were consulted only for cross-reference checks.
- Checked required front-matter fields, section structure, table conventions, and Liquid safety. The document is 65,434 bytes against a 65,536-byte cap.
- No files were written.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | `### C++ wrappers and accessors`, lines 100–111 | The claim that a hand-written wrapper receives an `unguarded form` that reads out of bounds is incorrect. Its `FIDDLE(leafInst())` accessor generator handles optional operands explicitly. | `source/slang/slang-ir.h.lua:75–76` checks `operand.optional` and generates `getOperandCount() > … ? … : nullptr`. The other generation path does likewise in `source/slang/slang-ir-insts.h:3175–3181`. | State that both generation paths guard optional generated accessors. Remove the claimed safety distinction and its explanation of why `IRImageSubscript`/`IRImageLoad` have presence predicates. Distinguish manually implemented accessors such as `IRImageStore::getAuxCoord1()` if retaining a warning. |
| F-002 | critical | `### Resource queries and modifiers`, lines 227–250 | The assertion that these intrinsics have `no public wrapper` and can only be reached by calling underscore names directly is contradicted by public buffer, texture-sampling, and footprint operations. | `source/slang/hlsl.meta.slang:34303–34308` implements `ByteAddressBuffer.GetBufferPointer()` through `__getStructuredBufferPtr(__getEquivalentStructuredBuffer<uint32_t>(this))`; line 34323 does likewise for writable buffers. Line 3462 calls `__metal_asDepthTexture` from sampling code. Lines 28547–28567 implement a footprint operation using both register queries. | Replace the blanket assertion with the actual caller distinctions: buffer pointer methods, Metal comparison sampling, and HLSL/NVAPI footprint queries supply public routes. Restrict direct-call-only statements to intrinsics for which source searches establish that property, and qualify the associated lowering-snapshot claim. |
| F-003 | critical | `### Mesh-shader outputs`, line 281 | The `metalSetPrimitive` summary says `never emitted`, contradicting its live intrinsic producer and the document’s later explanation. | `source/slang/core.meta.slang:2817–2818` declares the intrinsic; line 2832 calls it from the Metal arm of the output-primitive setter. `source/slang/slang-emit-metal.cpp:1088–1096` emits `_slang_mesh.set_primitive(...)`. | Delete `but never emitted`. State that output-primitive assignment produces the opcode; only the separately discussed builder helper is unused. |
| F-004 | critical | `### global_param and EntryPointParamDecoration`, lines 765–768; `## See also`, lines 840–842 | The document says layout instructions `attach as children` to `global_param`, including `EntryPointLayout`. Layout objects are referenced by a child `IRLayoutDecoration`; entry-point layouts attach to functions. | `source/slang/slang-ir-insts.h:1798–1810` documents the attachment targets and defines `IRLayoutDecoration::getLayout()` as operand 0. `source/slang/slang-ir.cpp:7431` implements `addLayoutDecoration`. | Describe the decoration as the child and the layout as its operand. Associate `IRVarLayout` with shader parameters and `IREntryPointLayout` with entry-point functions; correct the See-also summary accordingly. |
| F-005 | critical | Texture table, line 167; `### imageSubscript, imageLoad, and imageStore`, lines 530–536 | `Legalized away before emit` and the statement that the named pass rewrites `each imageSubscript` into load/store pairs overstate the transformation. That pass only visits stores and swizzled stores, and removes a subscript only when it has no remaining uses. | `source/slang/slang-ir-legalize-image-subscript.cpp:195–205` dispatches only `kIROp_Store` and `kIROp_SwizzledStore`. Lines 121–138 load the original texel only for partial updates; lines 145–148 condition removal on `!imageSubscript->hasUses()`. | Describe this pass as legalizing stores through image subscripts, with read-modify-write handling for component/swizzle stores. Remove the universal elimination claim and the implication that every subscript produces a load/store pair. |
| F-006 | minor | `### Atomic operations`, lines 323–328 | `Each atomic has two producer surfaces` incorrectly includes atomic load and store. The cited free-intrinsic block provides exchange, compare-exchange, arithmetic, bitwise, increment, and decrement operations, but no load/store counterparts. | `source/slang/hlsl.meta.slang:6353–6379`; `source/slang/core.meta.slang:4192–4197` supplies `Atomic<T>.load` and `.store`. A search of `source/` finds no `__atomic_load` or `__atomic_store`. | State that all thirteen have `Atomic<T>` methods, while the other eleven additionally have the listed free `__atomic_*` intrinsics. |
| F-007 | major | `## See also`, line 844 | The `TargetIntrinsicDecoration` link uses a nonexistent heading fragment. | `docs/generated/design/ir-reference/decorations.md:409` is headed ``### `targetIntrinsic` / `TargetIntrinsicDecoration` ``, whose fragment is `targetintrinsic--targetintrinsicdecoration`. | Change the fragment from `#targetintrinsic-targetintrinsicdecoration` to `#targetintrinsic--targetintrinsicdecoration`. |
| F-008 | major | `### Reading the operand column` and opcode tables | The custom square-bracket convention replaces the contract’s required notation for unnamed operands. It also mixes schema operands with undeclared runtime operands; the `byteAddressBufferStore` row presents semantic order as though it were the Lua accessor order. | `docs/generated/design/_meta/prompts/_common.md`, IR-reference operand-column contract, requires `(N unnamed)` or accessor-supported names marked `†`. `source/slang/slang-ir-insts.lua:1336` names operands `buffer, offset, value, alignment`; lines 1691–1696 supply only `min_operands = 1` for texture-access helpers. | Use `(N unnamed)` where no accessor establishes a name, or documented accessor-derived names with `†`. Keep undeclared memory-order operands explicitly distinguished. Show the Lua order in the buffer-store row and retain the semantic-order correction in its notable-opcode explanation. |
| F-009 | minor | `### C++ wrappers and accessors`, lines 76–95 | Hand-written wrappers are marked correctly, but their required count is absent. | `docs/generated/design/_meta/prompts/_common.md`, C++ wrapper-column contract, requires marking hand-written wrappers and stating their count. The target’s 91 opcode rows contain 20 correctly bolded hand-written wrappers, confirmed against `source/slang/slang-ir-insts.h`. | Add that 20 of the 91 listed concrete opcode wrappers are hand-written. |
| F-010 | major | `### Descriptor heaps`, lines 474–509; `## Notable opcodes`, lines 676–681 and 807–816 | The document includes extended target-specific lowering/emit behavior, a code block inside Notable opcodes, and an op-info schema explanation despite explicit scope and formatting restrictions. | `docs/generated/design/_meta/prompts/ir-reference-resources-and-atomics.md`, Forbidden content, excludes target-specific lowering. `_common.md` forbids code blocks in Notable opcodes and duplication of the schema explanation owned by `cross-cutting/ir-instructions.md`. | Reduce descriptor-heap prose to operand meanings and producer identification, linking backend details to the emit documentation. Express the `InterlockedAdd` relationship in prose. Replace the `fixedArgCount` subsection with a cross-reference to the schema documentation. |