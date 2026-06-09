---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-05T15:45:00Z
target_doc: name-resolution/overload-resolution.md
review_report: ../../reviews/name-resolution/overload-resolution.md.review.md
target_doc_source_commit_before: 52339028a2aa703271533454c6b9528a534bac31
target_doc_source_commit_after: 52339028a2aa703271533454c6b9528a534bac31
actions:
  fixed: 3
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for name-resolution/overload-resolution.md

## Summary

The review reported three findings, all fixed in the target doc. F-001: the IR-lowering/ICE prose for unresolved `PartiallyAppliedGenericExpr` (forbidden by the prompt and citing unwatched `slang-lower-to-ir.cpp`) was replaced, in both the partial-generic section and its edge-case echo, with a resolver-side invariant that defers downstream handling to `pipeline/04-ast-to-ir.md`. F-002: the arity step's source link to unwatched `slang-diagnostics.lua` was dropped while keeping the diagnostic names in prose. F-003: the no-applicable edge case was qualified to distinguish the single-best path (re-run via `CompleteOverloadCandidate`) from the tied non-applicable path, which emits `NoApplicableOverloadForNameWithArgs`/`NoApplicableWithArgs` directly (`source/slang/slang-check-overload.cpp:3216-3232`).

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Prompt `name-resolution-overload-resolution.md:107-114` forbids IR-lowering of resolved overloads; `slang-lower-to-ir.cpp` is unwatched. | Replaced the lowerer/ICE sentence in `## Partial generic application` and the matching edge-case bullet with a resolver-side invariant linking `pipeline/04-ast-to-ir.md`. |
| F-002 | fixed | `slang-diagnostics.lua` is not in this doc's watched paths; manifest edits are out of remediation scope, so the link was removed. | Removed the `slang-diagnostics.lua` source link from the arity step, keeping the diagnostic identifiers as inline prose. |
| F-003 | fixed | Tied non-applicable candidates emit `NoApplicableOverloadForNameWithArgs`/`NoApplicableWithArgs` directly (`slang-check-overload.cpp:3216-3232`), not via `CompleteOverloadCandidate`. | Split the no-applicable bullet into single-best (re-run) and tied (direct no-applicable diagnostics) cases with a watched citation. |
