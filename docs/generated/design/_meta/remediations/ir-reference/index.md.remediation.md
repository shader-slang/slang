---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-05T15:45:00Z
target_doc: ir-reference/index.md
review_report: ../../reviews/ir-reference/index.md.review.md
target_doc_source_commit_before: 52339028a2aa703271533454c6b9528a534bac31
target_doc_source_commit_after: 52339028a2aa703271533454c6b9528a534bac31
actions:
  fixed: 1
  rejected_bogus: 0
  rejected_out_of_scope: 1
  deferred: 0
  escalated: 0
---

# Remediation report for ir-reference/index.md

## Summary

Of two findings, one was fixed and one was rejected as out-of-scope.
The missing target-backends cross-reference required by the prompt was
added. The exhaustive-coverage finding was rejected as out-of-scope:
its defect (two unlisted opcodes) is owned by the family page
`ir-reference/resources-and-atomics.md`, which is remediated separately
this cycle; the navigation index must not be edited to paper over a
peer page's coverage gap.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | rejected-out-of-scope | The defect is the absence of `GetPerVertexInputArray` / `ResolveVaryingInputRef` (`source/slang/slang-ir-insts.lua:1534,1539`) from a family page; that is the owning family page's responsibility (resources-and-atomics.md F-001 adds them this cycle). Editing peer docs is forbidden (`_remediate.md:93`), and once the family page lists them the index's wording and ~85 count stay accurate, so no index edit applies. | — |
| F-002 | fixed | `docs/generated/design/_meta/prompts/ir-reference-index.md:51-56` requires a target-backends bullet in `## Cross-cutting topics`; `docs/generated/design/cross-cutting/targets.md` exists but was not linked. | Added a `../cross-cutting/targets.md` bullet to `## Cross-cutting topics`. |
