---
remediation_report: true
remediator_model: claude-opus-4.7
remediated_at: 2026-05-15T18:30:00+00:00
target_doc: cross-cutting/targets.md
review_report: ../../reviews/cross-cutting/targets.md.review.md
target_doc_source_commit_before: e75b9a3d03659cefb39882da3adecb2eb8751e0d
target_doc_source_commit_after: 470b96e8c29ca660c537d4d0f88cc21a12f962e6
actions:
  fixed: 1
  rejected_bogus: 1
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for cross-cutting/targets.md

## Summary

F-001 addressed by restructuring the `## Targets` table around
public `SlangCompileTarget` value groups, naming every public
enumerator (including DXIL/DXBC, host object/shared-library /
header targets, WGSL-SPIRV variants, CUDA object/header, and
Metal library variants) and pointing at the emit backend that
produces each group. F-002 rejected as bogus: HLSL legalization
is already listed in the "Target-specific lowering passes"
bullet list at the cited section.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `include/slang.h:614-659` declares the full `SlangCompileTarget` enum with all of the values the reviewer named; the original table covered roughly half of them and lost the multi-format-per-emitter relationship. | Replaced the per-emit-file table with a per-emit-backend grouping table whose first two columns are `Target group` and (comma-separated) `Public SlangCompileTarget values`. Every non-sentinel public enumerator is now named in exactly one row, and the sentinels (`SLANG_TARGET_UNKNOWN`, `SLANG_TARGET_NONE`, `SLANG_TARGET_COUNT_OF`) are explicitly called out below the table. |
| F-002 | rejected-bogus | The cited "## How target choice affects IR" bullet list explicitly contains "HLSL: [slang-ir-hlsl-legalize.cpp](...)" as the first item under "Target-specific lowering passes". The finding mis-reports the page as omitting HLSL. | No change to the document; the row is preserved as-is for the audit trail. |
