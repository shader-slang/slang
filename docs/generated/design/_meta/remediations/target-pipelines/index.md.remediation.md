---
remediation_report: true
remediator_model: claude-opus-4.7
remediated_at: 2026-05-15T20:30:00+00:00
target_doc: target-pipelines/index.md
review_report: ../../reviews/target-pipelines/index.md.review.md
target_doc_source_commit_before: e75b9a3d03659cefb39882da3adecb2eb8751e0d
target_doc_source_commit_after: 470b96e8c29ca660c537d4d0f88cc21a12f962e6
actions:
  fixed: 1
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for target-pipelines/index.md

## Summary

Removed the per-pass detail from the four-phase overview, leaving
just the high-level "what each phase does" summary as the contract
requires.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | The index contract in `_common.md` forbids documenting individual passes; the four-phase block was citing specific pass names and line ranges. | Rewrote the four-phase bullet list to describe each phase in one or two sentences and removed the cited pass names (`collectEntryPointUniformParams`, `simplifyIR`, `specializeArrayParameters`, `legalizeByteAddressBufferOps`, etc.) and the line ranges (928-1205, 1207-1773, 1798-2413, ~2418, ~2957). Readers wanting that detail are routed to the per-target pages and `pipeline/05-ir-passes.md`. |
