---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-05-15T16:50:36+00:00
target_doc: cross-cutting/targets.md
target_doc_source_commit: e75b9a3d03659cefb39882da3adecb2eb8751e0d
target_doc_watched_paths_digest: 4cd99452519c5be61ce5cca85589b24210e710c615e711767e7bd28139fcadbd
source_commit: 2580ad341db243d8bd27edd0327f08a29be906b3
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: fail
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 2
severity_breakdown:
  critical: 0
  major: 1
  minor: 1
  nit: 0
---

# Review report for cross-cutting/targets.md

## Summary
The page is structurally lint-clean, but review found 2 findings; the most significant severity is major. The main remediation need is to align the page with watched source evidence and the per-page prompt contract before marking this review cycle complete.

## Items checked
- Checked emit backend files, `SourceLanguage`, capability vocabulary, `Profile`, target-specific pass references, and add-target checklist.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Targets` | The target table is incomplete relative to public `SlangCompileTarget` values; it omits DXIL/DXBC, headers, host object/shared-library targets, WGSL-SPIRV variants, CUDA object/header, and Metal library variants. | `include/slang.h` enumerates many `SlangCompileTarget` values beyond the table. | Expand the table to cover all public target values, grouping variants that share an emitter. |
| F-002 | minor | `## How target choice affects IR` | The target-specific lowering list omits HLSL legalization even though the file exists. | `source/slang/slang-ir-hlsl-legalize.cpp` exists. | Add HLSL to the target-specific lowering bullet list. |
