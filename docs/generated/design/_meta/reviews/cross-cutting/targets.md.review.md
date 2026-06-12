---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-05T14:58:30+00:00
target_doc: cross-cutting/targets.md
target_doc_source_commit: 52339028a2aa703271533454c6b9528a534bac31
target_doc_watched_paths_digest: 44ece489c777e2db22b80fbab38fd7ce4eb6569d58ded6c76fe63d582db24928
source_commit: fb192be9f5b3b58555e034599e072158e5c48dfd
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: pass
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 1
severity_breakdown:
  critical: 0
  major: 1
  minor: 0
  nit: 0
---

# Review report for cross-cutting/targets.md

## Summary
The page satisfies the requested structure and all workspace-relative links resolve, but review found 1 factual source-alignment issue. The capability-system example for `raytracing` uses a stale expansion that is contradicted by the authoritative alias declaration in `slang-capabilities.capdef`.

## Items checked
- Ran `regenerate.py show cross-cutting/targets.md` and used the listed prompt, dependencies, and watched files.
- Verified required front-matter keys, source commit and watched digest shape, first paragraph, size cap, and required sections.
- Resolved all 63 workspace-relative markdown links at target source commit `52339028a2aa703271533454c6b9528a534bac31`.
- Spot-checked 17 source-alignment claims against `include/slang.h`, `slang-emit-*`, `slang-capabilities.capdef`, `slang-capability.*`, and `slang-profile.*`.
- Checked target enum grouping, representative emit files, capability vocabulary, generated capability headers, `SourceLanguage`, profile declarations, target-specific pass references, and add-target checklist.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `### Vocabulary` | The page states that `raytracing` expands to GLSL ray tracing, SPIR-V ray tracing, or HLSL shader model 6.4 capability alternatives. The authoritative alias later in the same capability file is different: it expands to `GL_EXT_ray_tracing`, `_sm_6_3`, or `cuda`. | `source/slang/slang-capabilities.capdef` line 1312 declares `alias raytracing = GL_EXT_ray_tracing` plus `_sm_6_3` plus `cuda`; the top-of-file comment at lines 15-18 is only an example and is stale relative to the declaration. | Update the example to match the alias declaration, or avoid naming `raytracing` and use a capability expression that is not contradicted by the actual `.capdef` entry. |
