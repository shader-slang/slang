---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-30T14:02:43Z
target_doc: pipeline/04-ast-to-ir.md
review_report: ../../reviews/pipeline/04-ast-to-ir.md.review.md
target_doc_source_commit_before: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_source_commit_after: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
actions:
  fixed: 0
  rejected_bogus: 1
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for pipeline/04-ast-to-ir.md

## Summary

One finding (F-001) was reviewed and rejected as bogus. The current target doc
already states the distinction the reviewer asked for: the `## IRBuilder and
instruction creation` section says only *hoistable* values are hash-consed and
that global opcodes are hoisted but never deduplicated. The reviewer (gpt-5.5)
assessed an earlier draft; the live doc was regenerated and already complies
with the source. No body edits were made, so `target_doc_source_commit_after`
equals `target_doc_source_commit_before`.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | rejected-bogus | The current doc, `docs/generated/design/pipeline/04-ast-to-ir.md:66-70`, already says `IRBuilder` hash-conses only *hoistable* values, that `kIROpFlag_Hoistable` "tags opcodes that are deduplicated this way," and that the separate `kIROpFlag_Global` flag marks opcodes "always hoisted to module scope but are **never** deduplicated." This matches `source/slang/slang-ir.h:53-56` (Hoistable = "needs to be deduplicated"; Global = "should always be hoisted but should never be deduplicated"). The reviewed text ("hash-conses hoistable and global values", "take part in this deduplication") belongs to a prior draft, not the live doc, so the finding's premise is false. | — |
