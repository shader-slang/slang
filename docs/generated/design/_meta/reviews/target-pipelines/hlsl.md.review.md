---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-05T13:46:37+00:00
target_doc: target-pipelines/hlsl.md
target_doc_source_commit: 52339028a2aa703271533454c6b9528a534bac31
target_doc_watched_paths_digest: 60ecd47fa50d35c35f1d4882efbb315be0fbf4a8c6dfcb71f79634016ea11275
source_commit: 05132edd86435f217f95634406f85184e58991f8
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 1
severity_breakdown:
  critical: 1
  major: 0
  minor: 0
  nit: 0
---

# Review report for target-pipelines/hlsl.md

## Summary
The HLSL page is complete in structure and all checked relative links resolve at the recorded source commit. One ordering error remains: three passes shown in Phase A actually run in the later Phase C region.

## Items checked
- Read `regenerate.py show target-pipelines/hlsl.md`, the HLSL prompt, `_common.md`, and dependency docs.
- Checked front matter, required sections, the `CodeGenTarget::HLSL` path, and downstream `DXIL` / `DXBytecode` handling against `source_commit` `52339028a2aa703271533454c6b9528a534bac31`.
- Resolved all 145 relative links in the page at the recorded source commit.
- Spot-checked HLSL claims for `legalizeNonVectorCompositeSelect`, skipped cooperative-vector lowering, HLSL ray-payload and non-struct-parameter legalization, `wrapStructuredBuffersOfMatrices`, DXC / fxc downstream dispatch, and loop absence.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | `## Phase A: Link and entry-point prep` and `## Phase C: HLSL legalization, lowering, phi elimination` | The Phase A diagram/table place `translateGlobalVaryingVar`, `resolveVaryingInputRef`, and `fixEntryPointCallsites` before uniform collection, but the source runs those passes in Phase C before the target entry-point legalization switch and before `floatNonUniformResourceIndex`. This makes the ordered HLSL pipeline materially wrong. | `source/slang/slang-emit.cpp:1955-1962` runs `translateGlobalVaryingVar`, `resolveVaryingInputRef`, and `fixEntryPointCallsites` in the Phase C range, not in the Phase A range around `source/slang/slang-emit.cpp:982-1001`. | Move those three nodes and rows from Phase A to Phase C, placing them after the HLSL filtered target-specific switches and before `floatNonUniformResourceIndex`; update row numbering and Phase A prose. |
