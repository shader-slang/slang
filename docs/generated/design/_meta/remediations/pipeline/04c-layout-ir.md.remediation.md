---
remediation_report: true
remediator_model: claude-opus-4.7
remediated_at: 2026-05-15T19:30:00+00:00
target_doc: pipeline/04c-layout-ir.md
review_report: ../../reviews/pipeline/04c-layout-ir.md.review.md
target_doc_source_commit_before: 07b911645ab895c59decdcc25c6c56ee245833af
target_doc_source_commit_after: 470b96e8c29ca660c537d4d0f88cc21a12f962e6
actions:
  fixed: 2
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for pipeline/04c-layout-ir.md

## Summary

Two minor citation findings addressed. Both were typos / stale line
numbers that misdirected readers to the wrong file region.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | The line range was typo'd as `14374-14394` instead of `15374-15394`. | Updated the "Per-global-parameter steps" section to cite `lines 15374-15394` of `createIRModuleForLayout`. |
| F-002 | fixed | The `getExistingIRModuleForLayout` declaration is near line 111 of `slang-target-program.h`, not line 15311. | Updated the "Cache and reuse" paragraph to cite `near line 111`. |
