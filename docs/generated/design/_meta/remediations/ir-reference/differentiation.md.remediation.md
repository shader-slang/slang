---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-30T14:00:03Z
target_doc: ir-reference/differentiation.md
review_report: ../../reviews/ir-reference/differentiation.md.review.md
target_doc_source_commit_before: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_source_commit_after: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
actions:
  fixed: 0
  rejected_bogus: 2
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ir-reference/differentiation.md

## Summary
Both findings were rejected as bogus: neither matches the target document's current contents. The `## Notable opcodes` section already contains the required `GetDifferential` / `GetPrimal` callout and a `Reverse-mode context` callout that explicitly handles the `BackwardDiffPropagateContext` spelling against the current source opcode names, and the `detachDerivative` AST-origin cell already reads `DetachExpr` / `visitDetachExpr`, not `DetachDerivativeExpr`. The review describes an earlier doc state; these were corrected in a prior cycle. No new edits were made; `target_doc_source_commit_after` equals `_before`.

## Actions
| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | rejected-bogus | The doc already satisfies the prompt. `docs/generated/design/ir-reference/differentiation.md:236` is a `### GetDifferential / GetPrimal` notable callout, and `:248` is a `### Reverse-mode context` callout that explicitly states no `BackwardDiffPropagateContext` opcode exists, names the current `BackwardDiffIntermediateContextType` / `BackwardDiffMinimalContextType` (`source/slang/slang-ir-insts.lua:172`, `:189`), and defers the context types to `types.md`. | — |
| F-002 | rejected-bogus | `docs/generated/design/ir-reference/differentiation.md:223` already gives the AST origin as `DetachExpr` (`detach(...)`) via `visitDetachExpr`, matching `source/slang/slang-lower-to-ir.cpp:5730` (`visitDetachExpr(DetachExpr*)` emitting `emitDetachDerivative`). The claimed `DetachDerivativeExpr` text is not present. | — |
