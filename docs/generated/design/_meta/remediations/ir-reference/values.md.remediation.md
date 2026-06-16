---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-12T14:14:47Z
target_doc: ir-reference/values.md
review_report: ../../reviews/ir-reference/values.md.review.md
target_doc_source_commit_before: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_source_commit_after: eb9403ef595a99c2ff6def1d538dbd7a792d9371
actions: { fixed: 1, rejected_bogus: 0, rejected_out_of_scope: 0, deferred: 0, escalated: 0 }
---

# Remediation report for ir-reference/values.md

## Summary
The review contained one minor finding, which was fixed. The `See also` bullet for `misc.md` incorrectly directed readers there for the `bitfieldExtract` and `bitfieldInsert` opcodes, which are actually documented in this page and absent from `misc.md`. No findings were rejected, deferred, or escalated.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Verified: the `bitfieldExtract`/`bitfieldInsert` rows live in this doc (values.md:125-126); `grep -i bitfield` over `docs/generated/design/ir-reference/misc.md` finds only an unrelated "op-flag bitfield" mention (misc.md:35) and no matching opcode rows, while `misc.md` does document the type-introspection predicates `IsType`/`IsInt` (misc.md:124-126). The bullet misdirected readers. | Removed the bitfield-opcode wording from the `misc.md` see-also bullet, leaving the accurate type-introspection-predicate reference. |
