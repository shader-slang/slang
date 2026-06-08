---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-05T14:56:27+00:00
target_doc: pipeline/04-ast-to-ir.md
target_doc_source_commit: 52339028a2aa703271533454c6b9528a534bac31
target_doc_watched_paths_digest: 9ab5063bb78269f0497d90eee21d470b180a0ade8c090e36af38aa4c444758ed
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

# Review report for pipeline/04-ast-to-ir.md

## Summary
The page is structurally complete and most sampled lowering claims match the recorded source commit. One major source-alignment issue remains: the adjacent-pipelines description says the layout IR module is not fed into `linkAndOptimizeIR`, but `linkIR` explicitly considers an existing layout module when linking a target program.

## Items checked
- Ran `regenerate.py show pipeline/04-ast-to-ir.md` and reviewed the manifest entry, prompt, resolved watched files, and dependencies on `pipeline/03-semantic-check.md` and `cross-cutting/ir-instructions.md`.
- Verified front matter fields and resolved all 19 relative links.
- Checked the required sections against `pipeline-04-ast-to-ir.md` and `_common.md`.
- Spot-checked 18 source-alignment claims, including `generateIRForTranslationUnit`, `generateIRForSpecializedComponentType`, `generateIRForTypeConformance`, `IRBuilder`, `kIROpFlag_Hoistable`, `kIROpFlag_Global`, `IRModule::create`, `IRModule::buildMangledNameToGlobalInstMap`, entry-point lowering, the decl walk, hashed string literals, NVAPI decorations, block parameters, generics, witness tables, and adjacent `04b` / `04c` pipeline descriptions.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Adjacent pipelines`, lines 170-174 | The page says the layout IR module `is not fed into linkAndOptimizeIR`. The source contradicts that absolute wording: `linkIR` adds an existing layout module to the list of IR modules so layout-decorated global symbols are considered during linking. | `source/slang/slang-ir-link.cpp:2120-2127` says `We will also consider the IR global symbols from the IR module attached to the TargetProgram` and then adds `irModuleForLayout` when present. | Replace the absolute statement with a narrower one: the layout IR module is not the executable per-translation-unit module and does not run the mandatory pre-link passes, but an existing layout module is considered by `linkIR` for layout-decorated symbols. |
