---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-05T15:05:51+00:00
target_doc: ir-reference/resources-and-atomics.md
target_doc_source_commit: 52339028a2aa703271533454c6b9528a534bac31
target_doc_watched_paths_digest: 5ac7df35674b391db414495e8be54b9c8c58690cd2b324a3a4c6804a1748f586
source_commit: fb192be9f5b3b58555e034599e072158e5c48dfd
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: partial
  source_alignment: partial
  front_matter_validity: pass
finding_count: 3
severity_breakdown:
  critical: 0
  major: 2
  minor: 1
  nit: 0
---

# Review report for ir-reference/resources-and-atomics.md

## Summary
The page has valid front matter and all relative links resolve at the target source commit. I found three issues: two completeness gaps around shader-IO coverage and required sampling callouts, plus one style-scope issue where target-specific lowering details appear despite the prompt's forbidden-content rule.

## Items checked
- Ran `python3 docs/generated/design/_meta/regenerate.py show ir-reference/resources-and-atomics.md`.
- Read `_common.md`, `ir-reference-resources-and-atomics.md`, the target document, dependency docs, and watched source files at `52339028a2aa703271533454c6b9528a534bac31`.
- Resolved all 17 relative Markdown links at the target source commit.
- Verified front matter keys and checked the target source commit and watched-path digest values against the document front matter.
- Checked every `AtomicOperation` child row and spot-checked more than 10 additional claims across texture, buffer, shader-IO, mesh-output, barrier, cooperative, wave, raytracing, descriptor-heap, binding-query, and `IRMemoryOrder` coverage.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Opcodes` | The shader-IO coverage omits `GetPerVertexInputArray` and `ResolveVaryingInputRef`, two concrete varying-input opcodes in the watched Lua source that are not listed by sibling family pages. | `source/slang/slang-ir-insts.lua:1534-1539` declares `GetPerVertexInputArray` and `ResolveVaryingInputRef`; `docs/generated/design/_meta/prompts/ir-reference-resources-and-atomics.md:35-37` assigns shader IO entries to this page. | Add these opcodes to an appropriate shader-IO or varying-IO sub-table, or explicitly move them to another family page and cross-link from here. |
| F-002 | major | `## Notable opcodes` | The prompt requires a notable discussion comparing implicit, explicit, and gradient sampling encodings, but the notable section has no sampling callout. | `docs/generated/design/_meta/prompts/ir-reference-resources-and-atomics.md:54-60` requires sampling notable coverage; `source/slang/slang-ir-insts.lua:1508-1509` declares `sample` and `sampleGrad`. | Add a focused sampling callout that explains the available sampling opcodes in this source revision and how their operands encode LOD or gradient information. |
| F-003 | minor | `SubpassLoad` row and notable opcode prose | The page includes target-specific lowering and emission details even though the resources prompt forbids target-specific lowering content. | `docs/generated/design/_meta/prompts/ir-reference-resources-and-atomics.md:70-77` forbids target-specific lowering; the `SubpassLoad`, `imageLoad` and `imageStore`, and `ControlBarrier` prose describes SPIR-V, HLSL, and Metal lowering behavior. | Trim those backend-lowering details and replace them with links to the target pipeline or emit pages already listed in `## See also`. |
