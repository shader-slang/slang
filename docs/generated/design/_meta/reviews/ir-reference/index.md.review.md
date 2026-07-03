---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-30T13:27:15+00:00
target_doc: ir-reference/index.md
target_doc_source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_watched_paths_digest: b01105947bb6bdcf6a24a6d12b46521c4b6bfb52a24e7ee5da31dceb7f981082
source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
checklist:
  factual_accuracy: pass
  cross_references: pass
  completeness: pass
  style_consistency: pass
  source_alignment: pass
  front_matter_validity: pass
finding_count: 0
severity_breakdown:
  critical: 0
  major: 0
  minor: 0
  nit: 0
---

# Review report for ir-reference/index.md

## Summary
The IR reference index passes this review. I found no factual, link, front-matter, required-section, or source-alignment findings in the checked material.

## Items checked
- Ran `regenerate.py show ir-reference/index.md` and reviewed the target document, `_common.md`, `ir-reference-index.md`, the listed dependency docs, and the resolved watched source `source/slang/slang-ir-insts.lua`.
- Checked front matter for all required generated-doc keys, the target source commit, the warning string, and a 64-character hex watched-path digest.
- Resolved the relative links to all ten family pages plus the referenced generated pipeline, cross-cutting, AST-reference, glossary, and source files.
- Verified the required `# IR Reference`, `## Family taxonomy`, `## Pages`, `## How AST nodes lower to IR`, `## Cross-cutting topics`, and `## How to navigate` sections against the per-doc prompt.
- Spot-checked more than 10 claims against watched files and dependencies, including the `Type`, `Constant`, `GlobalValueWithCode`, `TerminatorInst`, `AtomicOperation`, `MakeDifferentialPairBase`, `Decoration`, `Layout`, `Attr`, and `SPIRVAsmOperand` Lua roots.
- Counted opcode table rows in the ten family pages and confirmed the index's rounded approximate counts are within the prompt's +/- 10 tolerance.

## Findings
(no findings)

## No-issues notes
- The mermaid taxonomy covers each family page exactly once.
- The page stays in navigation scope and does not duplicate per-opcode reference content.
- The recent-addition examples (`MetalPackedVec`, `Abort`, `glslFragDepthGreater`, `glslFragDepthLess`) resolve in the watched Lua source or dependency pages.
