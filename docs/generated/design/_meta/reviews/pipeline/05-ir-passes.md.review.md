---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-30T13:33:12+00:00
target_doc: pipeline/05-ir-passes.md
target_doc_source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_watched_paths_digest: cdb8cb5584a28f89a0b7db40000011a37d1ed999cfb2fc9243256f34051010a7
source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: pass
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 2
severity_breakdown:
  critical: 0
  major: 0
  minor: 2
  nit: 0
---

# Review report for pipeline/05-ir-passes.md

## Summary

The document mostly satisfies the prompt contract: front matter is complete, required sections are present, and all checked links resolve. I found two minor source-alignment issues where the page overstates which passes run again from `linkAndOptimizeIR` and classifies helper files as target-specific passes.

## Items checked

- Read the target document, `_common.md`, `pipeline-05-ir-passes.md`, and the dependency document `pipeline/04-ast-to-ir.md`.
- Used `regenerate.py show pipeline/05-ir-passes.md` to confirm the scoped prompt, watched paths, `depends_on`, and resolved watched files.
- Verified the target front matter uses `source_commit` `c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8` and `watched_paths_digest` `cdb8cb5584a28f89a0b7db40000011a37d1ed999cfb2fc9243256f34051010a7`.
- Checked all 185 Markdown relative links in the target document for filesystem resolution.
- Spot-checked source claims in `slang-emit.cpp`, `slang-lower-to-ir.cpp`, `slang-ir-ssa-simplification.cpp`, `slang-ir-coverage-instrument.cpp`, `slang-ir-translate.h`, `slang-ir-translate.cpp`, `slang-ir-spirv-snippet.h`, and representative pass headers.
- Verified at least 10 concrete claims, including the `linkAndOptimizeIR` entry point, the `SLANG_PASS` wrapper, `linkIR`, pre-link SSA/SCCP/CFG/peephole/DCE calls, the mandatory early-inlining loop, `simplifyIR`'s fixed-point contents, coverage buffer element-width handling, boolean coverage stores, `finalizeCoverageInstrumentationMetadata`, `TargetProgram::createIRModuleForLayout`, and representative pass-file existence.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | minor | `## How the passes are ordered`, paragraph beginning `Individual category tables below` | The sentence says `constructSSA`, `propagateConstExpr`, `eliminateDeadCode`, `simplifyCFG`, and `peepholeOptimize` are `all invoked both in generateIRForTranslationUnit and again from linkAndOptimizeIR`. `propagateConstExpr` is shown in the pre-link region, but the post-link `simplifyIR` helper that `linkAndOptimizeIR` calls is documented and implemented as SSA/SCCP/SimplifyCFG/DCE plus related cleanup, not constexpr propagation. | `source/slang/slang-lower-to-ir.cpp:15339` has `propagateConstExpr(module, compileRequest->getSink());`; `source/slang/slang-ir-ssa-simplification.cpp:50` says `simplifyIR` runs "SSA, SCCP, SimplifyCFG, and DeadCodeElimination"; `source/slang/slang-emit.cpp:1452` calls `SLANG_PASS(simplifyIR, targetProgram, fastIRSimplificationOptions, sink)`. | Remove `propagateConstExpr` from the "invoked both" list, or state separately that it appears in the pre-link non-essential-validation block while post-link cleanup uses `simplifyIR`/SCCP rather than `propagateConstExpr`. |
| F-002 | minor | `### Target-specific lowering`, table rows `SPIR-V snippet` and `Translate` | The section says `These passes run only for their named target`, but the rows for `SPIR-V snippet` and `Translate` describe helper infrastructure rather than target-gated lowering passes. `SpvSnippet` is a parsed SPIR-V assembly snippet helper, and `slang-ir-translate` exposes a shared translation dictionary/context used by specialization and related analysis. | `source/slang/slang-ir-spirv-snippet.h:22` says `SpvSnippet` "Represents a parsed Spv ASM from intrinsic definition"; `source/slang/slang-ir-translate.h:18` declares `initializeTranslationDictionary` / `clearTranslationDictionary` and `TranslationContext`; `source/slang/slang-emit.cpp:1479` clears the translation dictionary as shared post-link cleanup. | Move these rows to `## Pass utilities` / helper prose, or change the target-specific section wording so it does not present helper files as target-only passes. |

## No-issues notes

- The required top-level sections from `pipeline-05-ir-passes.md` are present.
- The document is under the 64 KB size cap.
- The dependency link to `pipeline/04-ast-to-ir.md` and peer links to target-pipeline pages resolve.
- The detailed coverage instrumentation row aligns with the checked coverage pass implementation.
