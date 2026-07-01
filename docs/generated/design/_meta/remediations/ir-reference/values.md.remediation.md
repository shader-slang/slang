---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-30T14:14:24Z
target_doc: ir-reference/values.md
review_report: ../../reviews/ir-reference/values.md.review.md
target_doc_source_commit_before: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_source_commit_after: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
actions:
  fixed: 0
  rejected_bogus: 5
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ir-reference/values.md

## Summary
All five review findings were re-derived from scratch against the current
target document and the watched source at `c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8`,
ignoring the prior (inconsistent) report content. Every finding describes a
defect the current document does not exhibit: the opcode rows the review calls
missing are present, the disputed flag and prose are already correct, and the
AST-origin names it flags already match the lowering source. All five are
therefore `rejected-bogus`. No edits were applied, so
`target_doc_source_commit_after` equals `target_doc_source_commit_before` and
`mark-fresh` should not be rerun.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | rejected-bogus | Doc already lists both opcodes the finding calls missing: `docs/generated/design/ir-reference/values.md:241` (`makeValuePack`) and `:242` (`makeCombinedTextureSampler`) in the aggregate-constructors table, matching `source/slang/slang-ir-insts.lua:998-999`. | — |
| F-002 | rejected-bogus | Doc already covers the full conversion cluster: `values.md:175-181` lists `CastStorageToLogical`, `CastStorageToLogicalDeref`, `CastUInt64ToDescriptorHandle`, `CastDescriptorHandleToUInt64`, `CastDescriptorHandleToResource`, `CastResourceToDescriptorHandle`, and `TreatAsDynamicUniform`, matching `source/slang/slang-ir-insts.lua:2596-2611`. | — |
| F-003 | rejected-bogus | Literal rows `values.md:91-97` carry no `H` flag, and prose `values.md:82-85` already states literals are "not marked with the `H` (hoistable) opcode flag, but are deduplicated through the constant map by `IRBuilder::_findOrEmitConstant`". Source confirms: `slang-ir-insts.lua:866-880` sets no `hoistable=true`; `slang-ir.cpp:2301` defines `_findOrEmitConstant`. | — |
| F-004 | rejected-bogus | Doc already uses the correct AST classes: `values.md:94` cites `NullPtrLiteralExpr` and `:95` cites `NoneLiteralExpr` / `IRBuilder::getVoidValue`, matching `slang-lower-to-ir.cpp:6843` (`visitNullPtrLiteralExpr`) and `:6848-6850` (`visitNoneLiteralExpr` -> `getVoidValue`). The disputed `NullPtrExpr`/`VoidLiteralExpr` names are absent. | — |
| F-005 | rejected-bogus | Doc already qualifies `select` lowering: `values.md:137` and the notable callout `values.md:374-384` state only vector-typed or global-scope selects (via `visitInvokeExpr`) produce the opcode, while scalar in-function ternaries lower to `ifElse` + block `Param`, matching `slang-lower-to-ir.cpp` `visitSelectExpr`. | — |
