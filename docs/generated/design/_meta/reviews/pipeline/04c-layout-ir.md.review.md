---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-05T14:56:27+00:00
target_doc: pipeline/04c-layout-ir.md
target_doc_source_commit: 52339028a2aa703271533454c6b9528a534bac31
target_doc_watched_paths_digest: 17a141b9faa0ddd9e6fc9cf12a80084dc633df2e889700f9139cdd980e671b52
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

# Review report for pipeline/04c-layout-ir.md

## Summary
The page matches the requested layout-IR shape and the sampled construction steps in `createIRModuleForLayout`. One major source-alignment issue remains: the page repeatedly says the layout IR module is not fed into `linkAndOptimizeIR`, but `linkIR` explicitly considers an existing layout module so its layout-decorated global symbols participate in linking.

## Items checked
- Ran `regenerate.py show pipeline/04c-layout-ir.md` and reviewed the manifest entry, prompt, resolved watched files, and dependencies on `pipeline/04-ast-to-ir.md`, `pipeline/04b-pre-link-passes.md`, `cross-cutting/targets.md`, and `ir-reference/index.md`.
- Verified front matter fields and resolved all 18 relative links.
- Checked the required sections and table shape against `pipeline-04c-layout-ir.md` and `_common.md`.
- Spot-checked 23 source-alignment claims, including line-number citations for `createIRModuleForLayout`, `m_irModuleForLayout`, `getOrCreateIRModuleForLayout`, the global-parameter loop, parameter-group branch, entry-point loop, obfuscation block, `buildMangledNameToGlobalInstMap`, lazy cache behavior, `m_layout` assertion, SPIR-V/Metal capability atom forwarding, import decorations, optional obfuscation, and the stated relationship to executable IR and post-link linking.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | Intro and `## When it is built` and `## What this module is not`, lines 12-22, 66-73, and 193-202 | The page says the layout IR module is `not part of linkAndOptimizeIR` and `not fed into linkAndOptimizeIR`. The source contradicts the absolute wording: `linkIR` adds an existing target-program layout module to the IR module list so layout-decorated global symbols are considered during linking. | `source/slang/slang-ir-link.cpp:2120-2127` says `We will also consider the IR global symbols from the IR module attached to the TargetProgram` and then adds `irModuleForLayout` when present. | Reword these passages to say the layout module is not the executable per-translation-unit IR and does not run the pre-link mandatory optimization sequence, while noting that an existing layout module is considered by `linkIR` for layout-decorated symbols. |
