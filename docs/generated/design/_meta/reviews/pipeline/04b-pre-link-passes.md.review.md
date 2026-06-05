---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-05T14:56:27+00:00
target_doc: pipeline/04b-pre-link-passes.md
target_doc_source_commit: 52339028a2aa703271533454c6b9528a534bac31
target_doc_watched_paths_digest: 18c422c7671f2e36b802b77eaab4617e384c7719e835466116d64fcc7c9bd423
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
  critical: 0
  major: 1
  minor: 1
  nit: 0
---

# Review report for pipeline/04b-pre-link-passes.md

## Summary
The page covers the required pre-link pipeline and most phase-table claims match `generateIRForTranslationUnit`. The most important issue is the mandatory early-inlining loop description: it implies the `performMandatoryEarlyInlining` result is preserved through the iteration, while the source overwrites `changed` with the global peephole result before the final break test. The phase tables also use a non-contract column name.

## Items checked
- Ran `regenerate.py show pipeline/04b-pre-link-passes.md` and reviewed the manifest entry, prompt, resolved watched files, and dependencies on `pipeline/03-semantic-check.md`, `pipeline/04-ast-to-ir.md`, `pipeline/05-ir-passes.md`, and `ir-reference/index.md`.
- Verified front matter fields and resolved all 75 relative links.
- Checked the required sections and table shape against `pipeline-04b-pre-link-passes.md` and the pre-link mandatory-pass contract in `_common.md`.
- Spot-checked 32 source-alignment claims, including Phase A-D source ranges, all phase table calls, debug/minimum-optimization/non-essential-validation/obfuscation/language-version gates, adjacent constructs, line-number citations for `generateIRForTranslationUnit`, `SpecializedComponentTypeIRGenContext::process`, `TargetProgram::createIRModuleForLayout`, the per-function DCE sweep, and the mandatory early-inlining loop.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Loops in the pipeline`, lines 341-349 | The page says the outer loop continues as long as any inner pass reports progress and terminates only when `performMandatoryEarlyInlining` is false and the inner cluster makes no changes. In source, the `performMandatoryEarlyInlining` result is immediately overwritten by the `peepholeOptimizeGlobalScope` result when inlining reports true, and the final break test uses that overwritten value plus any later OR-assignment updates. | `source/slang/slang-lower-to-ir.cpp:14957` assigns `changed` from `performMandatoryEarlyInlining`; `source/slang/slang-lower-to-ir.cpp:14960` overwrites `changed` from `peepholeOptimizeGlobalScope`; `source/slang/slang-lower-to-ir.cpp:14977-14978` breaks when the final `changed` value is false. | Revise the loop diagram and prose to describe the actual `changed` variable flow, including that the inlining result is overwritten before the termination check. |
| F-002 | minor | Phase A through Phase D tables, lines 121, 159, 197, and 258 | The phase tables use `Call` as the second column header, but the contract requires the ordered table columns to be `#`, `Pass`, `File`, `Gate`, and `Notes`. | `docs/generated/design/_meta/prompts/_common.md:444-445` requires one ordered table per phase with `# / Pass / File / Gate / Notes`; `docs/generated/design/pipeline/04b-pre-link-passes.md:121` shows the first phase table header as `#`, `Call`, `File`, `Gate`, and `Notes`. | Rename the second column header from `Call` to `Pass` in all four phase tables. |
