---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-30T14:02:47Z
target_doc: pipeline/04b-pre-link-passes.md
review_report: ../../reviews/pipeline/04b-pre-link-passes.md.review.md
target_doc_source_commit_before: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_source_commit_after: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
actions:
  fixed: 0
  rejected_bogus: 1
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for pipeline/04b-pre-link-passes.md

## Summary

One finding reviewed. F-001 (critical) claims the `### obfuscateModuleLocs`
callout says locations are "simply stripped" when obfuscation is on without a
source map. That phrasing is not present in the target document at this commit;
the current callout already states the source-aligned behavior the reviewer
recommended. The finding is rejected-bogus and no document edits were made;
front-matter is unchanged.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | rejected-bogus | The "locs are simply stripped" text the finding quotes is absent. The current `### obfuscateModuleLocs` callout (`docs/generated/design/pipeline/04b-pre-link-passes.md:466-469`) already states it "sets `stripOptions.stripSourceLocs = false` unconditionally, so locs are never stripped here; if obfuscation is enabled without a source map, name hints are stripped but `obfuscateModuleLocs` does not run and the original locs are left in place." That matches `source/slang/slang-lower-to-ir.cpp:15400` (`stripOptions.stripSourceLocs = false`) and `:15415` (gate `stripOptions.shouldStripNameHints && linkage->m_optionSet.shouldHaveSourceMap()`). This is effectively the reviewer's recommended wording, so nothing to fix. | — |
