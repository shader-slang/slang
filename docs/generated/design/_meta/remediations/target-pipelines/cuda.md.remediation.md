---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-30T14:14:20Z
target_doc: target-pipelines/cuda.md
review_report: ../../reviews/target-pipelines/cuda.md.review.md
target_doc_source_commit_before: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_source_commit_after: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
actions:
  fixed: 0
  rejected_bogus: 3
  rejected_out_of_scope: 0
  deferred: 1
  escalated: 0
---

# Remediation report for target-pipelines/cuda.md

## Summary
Verified all four findings against the current on-disk target document
and source at HEAD. No body edits were required. Three findings
(F-001, F-002, F-004) are rejected as bogus because the current
document already complies: it describes the `PTX -> CUDASource` mapping
correctly throughout, already carries a `checkStaticAssert` table row,
and already cites the refreshed line numbers the reviewer requested.
Those three findings describe an earlier draft that had already been
rewritten before this cycle. F-003 (a missing `watched_paths` entry) is
valid but deferred: it needs a manifest change, which is out of scope
for the remediation stage. The target document is unchanged, so
`target_doc_source_commit_after` equals `_before`.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | rejected-bogus | Doc already complies. The intro (target doc lines 14-32) states a final `PTX` request does **not** drive the pipeline as `CodeGenTarget::PTX`, maps it to `CUDASource` via `_getDefaultSourceForTarget` (`source/slang/slang-code-gen.cpp:261-262`), and says the whole IR pipeline "therefore sees `CUDASource`". Every affected row (lines 157, 350, 360) and the Phase prose (lines 98-102, 696, 787, 815) already qualifies with "final `PTX` runs as `CUDASource`". The misdescription the finding cites is not present. | — |
| F-002 | rejected-bogus | Doc already complies. The Phase B table has a `checkStaticAssert` row (#63, target doc line 371) with file `slang-emit.cpp`, gate `(always)`, and a note that it is a direct call, matching the `cSA` diagram node; the source call is at `source/slang/slang-emit.cpp:1795`. The contract's one-row-per-node requirement is met. | — |
| F-003 | deferred | Valid and in-scope: the page cites and links `source/slang/slang-ir-synthesize-active-mask.cpp` (target doc lines 67, 480, 730) but `regenerate.py show target-pipelines/cuda.md` confirms it is absent from the resolved `watched_paths`, so changes to that source would not mark the page stale. The fix is a manifest `watched_paths` expansion, which the remediation stage may not edit. The cited link itself resolves (the file exists), so no doc-body fix applies. Follow-up: add `source/slang/slang-ir-synthesize-active-mask.cpp` to the cuda.md `watched_paths` in the manifest, then re-digest. | — |
| F-004 | rejected-bogus | Doc already complies. The page already cites line ~2680 for `shouldLegalizeExistentialAndResourceTypes = false`, line ~2635 for `new CUDASourceEmitter`, and line ~2766 for artifact wrapping — the exact values the finding requests — confirmed at `source/slang/slang-emit.cpp:2680`, `:2635`, `:2766`. No stale line references remain. | — |
