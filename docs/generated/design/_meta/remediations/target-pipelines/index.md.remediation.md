---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-12T14:16:15Z
target_doc: target-pipelines/index.md
review_report: ../../reviews/target-pipelines/index.md.review.md
target_doc_source_commit_before: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_source_commit_after: eb9403ef595a99c2ff6def1d538dbd7a792d9371
actions: { fixed: 1, rejected_bogus: 0, rejected_out_of_scope: 0, deferred: 0, escalated: 0 }
---

# Remediation report for target-pipelines/index.md

## Summary

The review raised one minor finding, which was fixed. No findings were rejected, deferred, or escalated. The HLSL and CUDA Phase C entry cells, which had been expanded into per-pass inventories, were replaced with the exact compact labels mandated by the index prompt.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Confirmed against the contract: the index prompt at `docs/generated/design/_meta/prompts/target-pipelines-index.md:51,54` mandates the exact cells `(no single entry; per-pass HLSL arms)` and `(no single entry; per-pass CUDA arms)`, and `_common.md:412-414` forbids per-pass details in the index. The doc had expanded both cells into full pass-name inventories. | Replaced the HLSL and CUDA Phase C entry cells with the compact labels `(no single entry; per-pass HLSL arms)` and `(no single entry; per-pass CUDA arms)`. |
