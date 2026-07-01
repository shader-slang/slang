---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-30T14:00:14Z
target_doc: ir-reference/metadata.md
review_report: ../../reviews/ir-reference/metadata.md.review.md
target_doc_source_commit_before: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_source_commit_after: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
actions:
  fixed: 0
  rejected_bogus: 1
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ir-reference/metadata.md

## Summary
One finding processed; no document edits were made. F-001 claimed the `## See also` section omits the AST-to-IR lowering page `../pipeline/04-ast-to-ir.md`, but that bullet is already present in the target document, so the finding is rejected as bogus. Front-matter is unchanged and `target_doc_source_commit_after` equals `_before`.

## Actions
| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | rejected-bogus | The lowering page is already linked at `docs/generated/design/ir-reference/metadata.md:264-267`: the `## See also` bullet `[../pipeline/04-ast-to-ir.md](../pipeline/04-ast-to-ir.md) — the AST-to-IR lowering stage that introduces the few metadata opcodes with a direct lowering origin`, which matches the reviewer's recommendation verbatim. The `_common.md:251` see-also contract is satisfied and the target `docs/generated/design/pipeline/04-ast-to-ir.md` exists so the link resolves. The reviewer's premise that only pipeline 05 and 06 are linked is incorrect. | — |
