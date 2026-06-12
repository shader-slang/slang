---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-12T14:12:59Z
target_doc: cross-cutting/ir-instructions.md
review_report: ../../reviews/cross-cutting/ir-instructions.md.review.md
target_doc_source_commit_before: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_source_commit_after: eb9403ef595a99c2ff6def1d538dbd7a792d9371
actions:
  fixed: 1
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for cross-cutting/ir-instructions.md

## Summary

The review contained one minor finding, which I fixed. No findings were rejected, deferred, or escalated. The fix corrects a source-citation error that attributed `getBuiltinRequirementKey` to the wrong file.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Confirmed against source: `IRBuilder::getBuiltinRequirementKey` is defined inline at `source/slang/slang-ir-insts.h:3492`, not in `slang-ir.cpp` (which contains only `getPoison` at line 3147). The cited file was wrong. | Changed the link for `getBuiltinRequirementKey` from `slang-ir.cpp` to `slang-ir-insts.h`. |
