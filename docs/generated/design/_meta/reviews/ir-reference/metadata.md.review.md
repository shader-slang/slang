---
review_report: true
reviewer_model: gpt-6
reviewed_at: 2026-11-03T08:03:35Z
target_doc: ir-reference/metadata.md
target_doc_source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_watched_paths_digest: 20d50d60a85384fcf21d93a052e6ee1e8ba60a6c2da3146dcaf4302e395fb9e8
source_commit: 8341c7e537a3f5cb3a60cd062066ffd630da40d8
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 4
severity_breakdown:
  critical: 0
  major: 0
  minor: 4
  nit: 0
---

# Review report for ir-reference/metadata.md

## Summary

The document covers the required families and accurately describes most checked operand encodings and producers. The most consequential correction is the claim that a structured buffer’s element always receives the fallback `typeLayout`; lowering preserves specialized element layouts. Three further findings concern operand-column conventions, detailed dumps, and stale citations.

## Items checked

- Reviewed against the current working tree; `source/` has no differences from the document’s source commit. Confirmed HEAD matches the supplied review commit.
- Read `_review.md`, `_common.md`, the document-specific prompt, and the resolved watched paths from `regenerate.py show`.
- Checked all 60 opcode rows for family coverage and wrapper identity; confirmed 39 handwritten wrappers and 21 generated wrappers.
- Checked every line-number citation: 84 citation groups, including repeated references, lists, and ranges.
- Spot-checked more than 20 behavioral claims, including alignment operand order/defaults, size and offset encoding, layout lowering, optional debug operands, debug-level gates, dormant builders, inline-assembly producers, and dump formatting.
- Resolved all 21 distinct relative file links and the internal heading link; confirmed all 10 linked generated pages exist in the manifest. Consulted dependency documents only for cross-reference checks.
- Checked required sections, table columns, front-matter fields, Mermaid syntax, Liquid safety, and the size cap: 45,162 bytes against 65,536.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | minor | Layout family, lines 162–165 | `RWStructuredBuffer<T>`, “whose element type `T` gets the fallback `typeLayout`,” overgeneralizes. A struct element receives `structTypeLayout`; other specialized element layouts are likewise preserved. | `source/slang/slang-lower-to-ir.cpp:16187` recursively calls `lowerTypeLayout` on `structuredBufferTypeLayout->elementTypeLayout`; the following `StructTypeLayout` branch constructs `IRStructTypeLayout::Builder`. | Use `RWStructuredBuffer<float>` as the fallback example and state that general `T` is recursively lowered to its appropriate layout opcode. |
| F-002 | minor | Opcodes tables, lines 138–321 | Numerous unnamed Lua operands receive descriptive names without the required `†` annotation. For example, `DebugSource` supplies `fileName, source, isIncludedFile`, while Lua declares only a minimum count. The general introduction does not provide the contract’s per-cell distinction between schema names and accessor-derived names. | `docs/generated/design/_meta/prompts/_common.md`, IR-reference Operands contract; `source/slang/slang-ir-insts.lua:3011` declares `min_operands = 3`, while `source/slang/slang-ir-insts.h:2739` supplies the accessor meanings. | Mark accessor-derived operand cells with `†` and add its legend. Use `(N unnamed)` where names cannot be established through accessors, and `(variadic)` for variable operand lists as required by the contract. |
| F-003 | minor | SPIR-V inline asm, lines 281–285; notable `SPIRVAsmOperand`, lines 622–625 | The assertion that every operand is folded into its use site omits the detailed-dump exception. Detailed dumps preserve separate instruction definitions. | `source/slang/slang-ir.cpp:7913`: `if (context->options.mode == IRDumpOptions::Mode::Detailed) return false;` precedes the `IRSPIRVAsmOperand` folding rule. | Qualify the folding description and example as applying to ordinary, non-detailed dumps; mention that detailed mode disables folding. |
| F-004 | minor | Notable `SPIRVAsmOperand`, lines 608–616 | Two citations point to unrelated code: Lua line `3020` is within `DebugVar`, and lowering line `6150` is outside `visitSPIRVAsmExpr`. The Source section already contains the correct locations. | `source/slang/slang-ir-insts.lua:3062` declares `SPIRVAsmOperand`; `source/slang/slang-lower-to-ir.cpp:5594` defines `visitSPIRVAsmExpr`. | Change `3020` to `3062` and `6150` to `5594`. |