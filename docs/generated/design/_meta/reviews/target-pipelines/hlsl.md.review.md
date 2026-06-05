---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-05T15:05:26+00:00
target_doc: target-pipelines/hlsl.md
target_doc_source_commit: 52339028a2aa703271533454c6b9528a534bac31
target_doc_watched_paths_digest: 60ecd47fa50d35c35f1d4882efbb315be0fbf4a8c6dfcb71f79634016ea11275
source_commit: fb192be9f5b3b58555e034599e072158e5c48dfd
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 2
severity_breakdown:
  critical: 1
  major: 0
  minor: 1
  nit: 0
---

# Review report for target-pipelines/hlsl.md

## Summary
The HLSL page covers the required target-pipeline sections, and all checked relative links resolve at the recorded source commit. One material ordering issue remains: three passes shown in Phase A actually run in the later Phase C region. There is also a small table-contract mismatch in the Phase D table header.

## Items checked
- Ran `python3 docs/generated/design/_meta/regenerate.py show target-pipelines/hlsl.md` and used the target front matter source commit and digest in this report.
- Read the HLSL target doc, `_common.md`, `target-pipelines-hlsl.md`, and the dependency docs `pipeline/04-ast-to-ir.md`, `pipeline/05-ir-passes.md`, `pipeline/06-emit.md`, `ir-reference/index.md`, and `cross-cutting/targets.md`.
- Resolved all 145 relative Markdown links at `52339028a2aa703271533454c6b9528a534bac31`; no dangling links were found.
- Checked the required target-pipeline sections, front matter keys, conditional-gate grouping, loop statement, downstream DXC and fxc coverage, and Phase D table columns.
- Verified more than 15 factual claims against source at the target commit, including `CodeGenTarget::HLSL` branch selection, D3D target predicates, HLSL-specific legalization passes, cooperative-vector skip behavior, byte-address-buffer options, logical-and-or legalization, uniform-buffer-load handling, phi elimination options, and downstream DXIL/DXBytecode pass-through.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | `## Phase A: Link and entry-point prep` and `## Phase C: HLSL legalization, lowering, phi elimination` | The Phase A diagram and table place `translateGlobalVaryingVar`, `resolveVaryingInputRef`, and `fixEntryPointCallsites` before uniform collection, but the source runs those passes in the Phase C range before target entry-point legalization and before `floatNonUniformResourceIndex`. This makes the ordered HLSL pipeline materially wrong. | `source/slang/slang-emit.cpp:982-1001` has only SSBO lowering, `translateEntryPointInParamToBorrow`, `replaceGlobalConstants`, and `bindExistentialSlots` in this early region; `source/slang/slang-emit.cpp:1952-1962` runs `translateGlobalVaryingVar`, `resolveVaryingInputRef`, and `fixEntryPointCallsites`. | Move those three nodes and rows from Phase A to Phase C, placing them after the HLSL filtered target-specific switches and before `floatNonUniformResourceIndex`; update row numbering and Phase A prose. |
| F-002 | minor | `## Phase D: HLSL emit and downstream tools` | The Phase D table header uses `Pass / step` instead of the required `Pass` column. The contract requires the ordered phase table columns to be exactly `#`, `Pass`, `File`, `Gate`, and `Notes`. | `docs/generated/design/_meta/prompts/_common.md:326-335` defines the required table columns; the target doc Phase D table header uses `Pass / step`. | Rename the Phase D table column to `Pass`, keeping downstream compiler rows in that column as pass-like steps if needed. |
