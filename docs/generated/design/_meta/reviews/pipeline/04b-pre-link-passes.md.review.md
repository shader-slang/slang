---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-30T13:34:14+00:00
target_doc: pipeline/04b-pre-link-passes.md
target_doc_source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_watched_paths_digest: 0a827687878ad7390b1acdda49546f652c2eaf5da2d820809834f1faa2ed69cb
source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: pass
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 1
severity_breakdown:
  critical: 1
  major: 0
  minor: 0
  nit: 0
---

# Review report for pipeline/04b-pre-link-passes.md

## Summary

The document gives a well-structured, source-aligned view of the pre-link pipeline and satisfies the required four-phase contract. The only issue found is in the `obfuscateModuleLocs` notable-pass text: it says source locations are stripped when no source map is requested, but the pre-link code explicitly sets `stripSourceLocs = false` and only obfuscates locations when both name stripping and source maps are enabled.

## Items checked

- Read the target document, `_common.md`, the per-doc prompt, and dependency docs `pipeline/03-semantic-check.md`, `pipeline/04-ast-to-ir.md`, `pipeline/05-ir-passes.md`, and `ir-reference/index.md`.
- Resolved the manifest data with `regenerate.py show pipeline/04b-pre-link-passes.md` and sampled claims against the watched files `slang-lower-to-ir.cpp`, `slang-lower-to-ir.h`, `slang-ir-link.cpp`, `slang-ir-link.h`, and `slang-compiler-options.h`.
- Verified 18 concrete source claims, including the Phase A setup and debug-source gates, Phase B pass order, Phase C optimization order, DCE options, loop-inversion gate, mandatory-early-inlining loop, Phase D validation/stripping/finalization order, option accessors, `prelinkIR` input modules and linking-info behavior, `[unsafeForceInlineEarly]` cloning, and both adjacent IR-module constructors.
- Checked that the required sections from the pre-link mandatory-pass contract are present, including phase diagrams/tables, conditional gates, the single pass-level loop, notable passes, adjacent constructs, and see-also links.
- Checked representative relative links to source files, peer generated docs, target-pipeline index, and IR-reference index.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | `### obfuscateModuleLocs`, lines 461-466 | The document says that when obfuscation is enabled but no source map is requested, "locs are simply stripped." In the pre-link stripping block, source locations are explicitly not stripped: `stripOptions.stripSourceLocs = false`; `obfuscateModuleLocs` then runs only when `shouldStripNameHints && shouldHaveSourceMap()` is true. | `source/slang/slang-lower-to-ir.cpp:15391-15418` sets `shouldStripNameHints`, sets `stripSourceLocs = false`, calls `stripFrontEndOnlyInstructions`, and gates `obfuscateModuleLocs` on `stripOptions.shouldStripNameHints && linkage->m_optionSet.shouldHaveSourceMap()`. | Replace the final sentence of this callout with a source-aligned statement such as: "If obfuscation is enabled without a source map, this pre-link block strips name hints but does not run `obfuscateModuleLocs`; it also leaves `stripSourceLocs` false here." |

## No-issues notes

- The phase tables match the order of calls in `generateIRForTranslationUnit`, including the disabled dump blocks being documented as non-pipeline steps.
- The mandatory-early-inlining loop description correctly captures the subtle overwrite of `changed` by `peepholeOptimizeGlobalScope`.
- The `prelinkIR` callout matches the source: full function bodies are cloned only for `IRUnsafeForceInlineEarlyDecoration`, and linking info is built for stable input modules but not the mutating output module.
- The adjacent constructs correctly point out that specialized-component and layout IR modules are built separately and do not run the mandatory pre-link pass sequence.
