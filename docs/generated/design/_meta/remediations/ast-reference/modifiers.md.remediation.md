---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-12T14:13:06Z
target_doc: ast-reference/modifiers.md
review_report: ../../reviews/ast-reference/modifiers.md.review.md
target_doc_source_commit_before: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_source_commit_after: eb9403ef595a99c2ff6def1d538dbd7a792d9371
actions:
  fixed: 2
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ast-reference/modifiers.md

## Summary

The review listed two minor findings, both source-alignment errors in
the `Key fields` column of the Nodes tables. Both were verified against
`source/slang/slang-ast-modifier.h` at the reviewed commit and fixed
(fixed=2; no rejections, deferrals, or escalations).

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Confirmed against source: `source/slang/slang-ast-modifier.h:264-274` declares `Token opToken` and `FIDDLE() uint32_t op = 0`; the doc's `opcode: int`, `irOp: uint32_t` were not the real field names. | Changed the `IntrinsicOpModifier` Key fields cell to `opToken: Token`, `op: uint32_t`. |
| F-002 | fixed | Confirmed against source: `source/slang/slang-ast-modifier.h:416-420` declares `GLSLUnparsedLayoutModifier : public Modifier` with only `FIDDLE(...)` and no additional fields, so `text Token` was wrong. | Changed the `GLSLUnparsedLayoutModifier` Key fields cell to `(no additional state)`. |
