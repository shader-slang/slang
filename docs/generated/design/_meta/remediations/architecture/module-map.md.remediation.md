---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-05T15:45:00Z
target_doc: architecture/module-map.md
review_report: ../../reviews/architecture/module-map.md.review.md
target_doc_source_commit_before: 52339028a2aa703271533454c6b9528a534bac31
target_doc_source_commit_after: 52339028a2aa703271533454c6b9528a534bac31
actions:
  fixed: 1
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for architecture/module-map.md

## Summary

The single major finding was fixed (fixed=1; no rejections, deferrals,
or escalations). F-001 reported that seven `source/` subdirectories were
collapsed into one two-column catch-all table instead of receiving their
own level-2 sections with the prompt-required `Logical unit | Files |
Responsibility` columns. The catch-all section was split into one
level-2 section per subdirectory, matching the per-subdirectory heading
convention already used by the rest of the page; each new section has a
single-row three-column table citing representative files that resolve
at the source commit.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | The module-map prompt (`docs/generated/design/_meta/prompts/architecture-module-map.md:23-31,66-67`) requires each overview logical-unit group its own level-2 section with the three-column table; the catch-all used a two-column Subdirectory/Role table. | Replaced `## Other source/ subdirectories` with seven level-2 sections (`slang-llvm`, `slang-glslang`, `slang-dispatcher`, `slang-rt`, `slang-record-replay`, `slang-wasm`, `slangc`), each with a Logical unit / Files / Responsibility table. |
