---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-05T13:46:25+00:00
target_doc: pipeline/04b-pre-link-passes.md
target_doc_source_commit: 52339028a2aa703271533454c6b9528a534bac31
target_doc_watched_paths_digest: 18c422c7671f2e36b802b77eaab4617e384c7719e835466116d64fcc7c9bd423
source_commit: 05132edd86435f217f95634406f85184e58991f8
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

# Review report for pipeline/04b-pre-link-passes.md

## Summary
The page is complete and most phase-table claims match `generateIRForTranslationUnit`, but review found one major source-alignment issue in the mandatory early-inlining loop description. The loop termination prose and diagram imply the `performMandatoryEarlyInlining` result is preserved through the iteration, while the source overwrites `changed` with the global peephole result before the final break test.

## Items checked
- Ran `regenerate.py show pipeline/04b-pre-link-passes.md` and reviewed the manifest entry, prompt, resolved watched files, and dependencies on `pipeline/03-semantic-check.md`, `pipeline/04-ast-to-ir.md`, `pipeline/05-ir-passes.md`, and `ir-reference/index.md`.
- Verified front matter fields and resolved all 75 relative links.
- Checked Phase A-D source ranges, all phase table calls, debug/minimum-optimization/non-essential-validation/obfuscation/language-version gates, adjacent constructs, and line-number citations for `generateIRForTranslationUnit`, `SpecializedComponentTypeIRGenContext::process`, and `TargetProgram::createIRModuleForLayout`.
- Verified the mandatory early-inlining loop source around lines 14953-14979 and the documented per-function DCE and per-modified-function iteration claims.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Loops in the pipeline`, lines 341-349 | The page says the outer loop continues as long as any inner pass reports progress and terminates only when `performMandatoryEarlyInlining` is false and the inner cluster makes no changes. In source, `changed = performMandatoryEarlyInlining(...)` is immediately overwritten by `changed = peepholeOptimizeGlobalScope(...)` when inlining reports true, and the final `if (!changed) break;` tests that overwritten value plus any later `|=` updates. | `source/slang/slang-lower-to-ir.cpp:14957` assigns `changed = performMandatoryEarlyInlining(...)`; `source/slang/slang-lower-to-ir.cpp:14960` overwrites it with `changed = peepholeOptimizeGlobalScope(...)`; `source/slang/slang-lower-to-ir.cpp:14973-14974` breaks when the final `changed` value is false. | Revise the loop diagram and prose to describe the actual `changed` variable flow, including that the inlining result is overwritten before the termination check. |
