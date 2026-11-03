---
review_report: true
reviewer_model: gpt-6
reviewed_at: 2026-11-03T08:31:36Z
target_doc: ir-reference/values.md
target_doc_source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_watched_paths_digest: 9a5285aec84d075ab81dd3b3d93d9ff76bceb2489619b15eb6cd1ddf30f0c7ac
source_commit: 801b9e6e57d41f5cef1a5d54c0eeff9e0db4fee6
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: partial
  source_alignment: partial
  front_matter_validity: pass
finding_count: 9
severity_breakdown:
  critical: 2
  major: 3
  minor: 4
  nit: 0
---

# Review report for ir-reference/values.md

## Summary

The document needs corrections before acceptance, although most cited source locations match. The most important error describes `makeTargetTuple` as a target-name dispatch mechanism; it actually constructs a positional tuple in the target language. Other findings concern incorrect intrinsic producers, wrapper names, and required reference-page structure.

## Items checked

- Read `_review.md`, `_common.md`, and the document-specific prompt; ran `regenerate.py show ir-reference/values.md`.
- Verified against the **current working tree**. The source comparison against `2518a942d59b1228fc8cc70a37739baab82b6002` showed no differences under `source/`; HEAD matches the supplied review commit.
- Inspected every located line-number citation, including wrapped citations and numeric references in the Source list, against the cited source locations and surrounding implementation.
- Spot-checked more than 20 claims: literal storage and deduplication, dump formatting, poison, SSA uninitialized values, scalar selection, constexpr lowering, numeric casts, default-construction fallback, descriptor casts, swizzle encoding, allocation, target tuples, and string/native-pointer producers.
- Checked 148 opcode rows and confirmed 40 handwritten-wrapper markers. Resolved all 193 Markdown links, including fragment targets, and confirmed linked generated pages occur in the manifest. Dependency documents were consulted only for cross-reference checks.
- Checked required front-matter fields, section order, table columns, Mermaid syntax, and Liquid safety. The document is 79,381 bytes, below its 98,304-byte cap. No files were written.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | Aggregate constructors, line 387 | `makeTargetTuple` is described as a value “keyed by target name, used by `targetSwitch`.” Its elements are positional values, not target-name alternatives. | `source/slang/slang-ir-insts.lua:785` describes a tuple in the target language. `source/slang/slang-ir-pytorch-cpp-binding.cpp:135–166` builds positional tuples; `source/slang/slang-emit-cpp.cpp:1504–1526` emits `std::make_tuple` and indexed `std::get`. | Describe construction of a target-language tuple used by PyTorch binding conversion. Remove the claimed relationship to `targetSwitch` and its cross-link. |
| F-002 | critical | Conversions, line 297 | The GLSL `*BitsTo*` family is incorrectly presented as declarations carrying `__intrinsic_op(BitCast)`. These functions call the same HLSL reinterpret helpers that the row correctly says use target-specific bodies. The citation to line 768 identifies a different intrinsic. | `source/slang/glsl.meta.slang:600–662` defines `floatBitsToInt`, `floatBitsToUint`, `intBitsToFloat`, and `uintBitsToFloat` through `asint`, `asuint`, and `asfloat`. Line 768 declares `unpack8` with `BitCast`. | Replace the example with `unpack8` at line 768, or another verified `BitCast` declaration. Explain that the GLSL `*BitsTo*` functions follow the HLSL helper path. |
| F-003 | major | Opcodes, lines 159–490 | All 148 C++ wrapper cells omit the required `IR` prefix, presenting names such as `BoolLit`, `Var`, and `ConstexprAdd` instead of actual C++ struct names. | `source/slang/slang-ir.h:1123` declares `IRBoolLit`; `source/slang/slang-ir-insts.h:2280` declares `IRVar`; its template at line 3161 emits `IR$(inst.struct_name)`. `_common.md` explicitly requires prefixed wrapper names. | Prefix every wrapper cell with `IR`, preserving the existing handwritten markers. |
| F-004 | major | Strings and native pointers, lines 351–354 | Three producer labels do not match their cited declarations: `String.getNativeStr`, a `ComPtr<T>` accessor, and `__attach`. | `source/slang/core.meta.slang:2240–2241` declares `NativeString.__init(String)`; lines 2132–2135 declare `NativeRef<T>.__init(T)`; lines 2139–2140 declare `__managed_ptr_attach`. | Replace the respective producer labels with `NativeString.__init(String)`, `NativeRef<T>.__init(T)`, and `__managed_ptr_attach`. |
| F-005 | major | Notable opcodes, lines 494–898 | The required notable treatment of `LoadFromUninitializedMemory` is absent. Its table row mentions `freeze(undef)`, but the notable section discusses only `Poison`, leaving the required comparison incomplete. | `docs/generated/design/_meta/prompts/ir-reference-values.md`, “Notable opcodes,” requires both. `source/slang/slang-ir-insts.lua:974–1008` distinguishes an arbitrary value shared by an instruction’s uses from infectious poison. | Add a short `LoadFromUninitializedMemory` callout, or expand the existing `Poison` callout to cover both, explaining the distinction and linking the existing IR rationale document. |
| F-006 | minor | Notable opcodes, lines 517–522 and 640–649 | This section contains fenced IR and Slang examples despite the family contract’s explicit prohibition on code blocks in notable callouts. Several callouts also substantially exceed its 2–5-sentence guidance. | `docs/generated/design/_meta/prompts/_common.md:277–285` requires short prose callouts and says “Do not include code blocks.” | Convert the two examples to concise inline examples and shorten the extended callouts to the prescribed prose form, retaining necessary detail in the relevant opcode-table introductions. |
| F-007 | minor | `Var` vs `GlobalVar` vs `Alloca`, lines 564–568 | “Ordinary value-typed local variables always become `var`” overlooks initialized immutable `let` declarations. | `source/slang/slang-lower-to-ir.cpp:11837–11900` explicitly lowers an initialized `LetDecl` to its initializer’s SSA value without an `IRVar`. | Qualify the statement to local variables requiring storage and mention the initialized-`let` exception. |
| F-008 | minor | Literals, lines 159–165 | Literal payload descriptions occupy the Operands column even though these instructions have no operands, and `blob_constant` has the retired bare `—` in its AST-origin cell despite a known producer. | `source/slang/slang-ir.h:1045` stores literal payload separately; `source/slang/slang-ir.cpp:2614` builds blobs, and line 4347 defines `emitEmbeddedDownstreamIR`. `_common.md` requires `—` for nullary operands and a named producer. | Use `—` in literal operand cells and keep payload descriptions in summaries or introductory prose. Put `emitEmbeddedDownstreamIR` in the blob’s AST-origin cell. |
| F-009 | minor | Live-but-unproduced opcodes, lines 837–840 | The blanket claim that all five listed opcodes have “emitter support” is false for `castToVoid`. | `source/slang/slang-lower-to-ir.cpp:1008–1018` explicitly says no backend can emit this instruction; `source/slang/slang-ir.cpp:4424–4431` likewise explains that the dedicated cast has no backend emission. | Remove the blanket emitter-support assertion. State that the opcodes remain declared, with backend handling varying by opcode, and explicitly exclude `castToVoid`. |