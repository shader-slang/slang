---
remediation_report: true
remediator_model: claude-sonnet-5-5
remediated_at: 2026-11-03T08:34:34Z
target_doc: cross-cutting/serialization.md
review_report: ../../reviews/cross-cutting/serialization.md.review.md
target_doc_source_commit_before: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
actions:
  fixed: 4
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for cross-cutting/serialization.md

## Summary

All four findings were verified against the current source and fixed in the
document. The IR field-addition guidance no longer implies a schema change,
the repro bullet was replaced, the manifest-coverage claim was qualified, and
the page was trimmed below 24 KB.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | source/slang/slang-serialize-ir.cpp:471-488 writes operand count and operands generically, so ordinary additions fit the flat schema. | Reworded IR steps 1-2 of "Adding a new serialized field": schema/version bump only for encoding or special-payload changes. |
| F-002 | fixed | source/slang/slang-repro.cpp keeps loadState and extractFilesToDirectory; CLAUDE.md retains -load-repro/-extract-repro. | Bullet now says repro format details are outside the watched paths. |
| F-003 | fixed | slang-fossil.h, slang-relative-ptr.h and slang-session.cpp are not in watched_paths. | Rewrote "Manifest coverage" to qualify the claim and name the unwatched dependencies. |
| F-004 | fixed | Prompt requires under 24 KB; the page was 27,202 bytes. | Condensed historical RIFF backend, source-loc, validation, version and worked-example prose; now 23,974 bytes. |
