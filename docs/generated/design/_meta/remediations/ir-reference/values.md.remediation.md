---
remediation_report: true
remediator_model: claude-sonnet-5-5
remediated_at: 2026-11-03T08:34:20Z
target_doc: ir-reference/values.md
review_report: ../../reviews/ir-reference/values.md.review.md
target_doc_source_commit_before: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
actions:
  fixed: 9
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ir-reference/values.md

## Summary

All nine findings were verified against current source and found valid; all nine were fixed in the doc. F-006 was fixed for the two code blocks only; the over-long callouts were not trimmed. The doc stays well under its 98304-byte cap.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | slang-ir-insts.lua:785 says "tuple in target language"; no targetSwitch relation | makeTargetTuple row: positional target-language tuple for PyTorch binding; removed targetSwitch link |
| F-002 | fixed | glsl.meta.slang:600-662 route through asint/asuint/asfloat; line 768 is `unpack8` with BitCast | bitCast row: example now `unpack8`, BitsTo family explained |
| F-003 | fixed | slang-ir-insts.h declares IR-prefixed structs; _common.md requires the prefix | added `IR` to 147 wrapper cells (scripted column rewrite, not per-cell Edit) |
| F-004 | fixed | core.meta.slang:2132-2141, 2240-2241 name different declarations | producers now `NativeString.__init(String)`, `NativeRef<T>.__init(T)`, `__managed_ptr_attach` |
| F-005 | fixed | slang-ir-insts.lua:974-990 defines distinct non-infectious semantics | added LoadFromUninitializedMemory paragraph under the Poison callout |
| F-006 | fixed | _common.md forbids code blocks in notable callouts | two code blocks converted to inline prose; callout length not trimmed |
| F-007 | fixed | slang-lower-to-ir.cpp:11837-11850 lowers initialized `let` without IRVar | var statement qualified with the `let` exception |
| F-008 | fixed | literals have no operands; blob has a named producer | literal operand cells now em-dash; blob origin `emitEmbeddedDownstreamIR` | 
| F-009 | fixed | slang-lower-to-ir.cpp:1008-1018 says no backend can emit castToVoid | removed blanket emitter-support claim; castToVoid excluded explicitly |
