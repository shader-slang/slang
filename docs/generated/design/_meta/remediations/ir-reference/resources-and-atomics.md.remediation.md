---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-12T14:14:58Z
target_doc: ir-reference/resources-and-atomics.md
review_report: ../../reviews/ir-reference/resources-and-atomics.md.review.md
target_doc_source_commit_before: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_source_commit_after: eb9403ef595a99c2ff6def1d538dbd7a792d9371
actions: { fixed: 1, rejected_bogus: 0, rejected_out_of_scope: 0, deferred: 0, escalated: 0 }
---

# Remediation report for ir-reference/resources-and-atomics.md

## Summary

The review contained one major finding, which was fixed. The `sampleGrad` opcode was incorrectly described as variadic with trailing `gradY`/offset/bias operands; the Lua schema declares a fixed four-operand shape, so both the table operand cell and the notable paragraph were corrected. No findings were rejected, deferred, or escalated.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Confirmed against `source/slang/slang-ir-insts.lua:1527` at the source commit: `{ sampleGrad = { operands = { { "texture" }, { "sampler" }, { "coord" }, { "gradX" } } } }` has no `min_operands`, so it is a fixed four-operand opcode, not variadic. The IR-reference family contract requires the Operands column to list the Lua operand names and reserve `(variadic)` for variadic ops. | Changed the `sampleGrad` operand cell to `texture, sampler, coord, gradX` and rewrote the notable paragraph to state the fixed four-operand shape, removing the variadic `gradY`/offset/bias claim. |
