---
remediation_report: true
remediator_model: claude-sonnet-5-5
remediated_at: 2026-11-02T21:21:30Z
target_doc: pipeline/04c-layout-ir.md
review_report: ../../reviews/pipeline/04c-layout-ir.md.review.md
target_doc_source_commit_before: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
actions:
  fixed: 5
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for pipeline/04c-layout-ir.md

## Summary

All five findings were verified against the current source and fixed with minimal edits. The capability-atom claim, the construction flow diagram, the IR instruction class, the layout decoration wording, and the contradictory manifest sentence were corrected. Front-matter `generated_at` and `watched_paths_digest` were refreshed.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `source/slang/slang-lower-to-ir.cpp:16564-16575` iterates all atom sets and filters by atom value only, with no target gate. | Step 6 note and the caveat bullet now say only SPIR-V/Metal-version atoms are forwarded regardless of target; removed tool guidance. |
| F-002 | fixed | `source/slang/slang-parameter-binding.cpp:4853-4868` guards binding with `!m_layout`, latches on error-count delta, and rejects on `m_layoutHasErrors`. | Flow diagram adds `m_layout` gate, latch node, and `m_layoutHasErrors` gate; line 260 prose aligned. |
| F-003 | fixed | `source/slang/slang-lower-to-ir.cpp:11375` calls `createGlobalParam`. | Step 1: `IRGlobalVar` -> `IRGlobalParam`. |
| F-004 | fixed | `source/slang/slang-lower-to-ir.cpp:16528-16531` decorates the module with an `IRVarLayout`. | Wording now says an `IRVarLayout` whose type layout is the parameter-group or struct layout. |
| F-005 | fixed | `docs/generated/design/_meta/manifest.yaml:241` lists `slang-type-layout.cpp`. | Replaced the contradictory sentence with a statement that the file is watched. |
