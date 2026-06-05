---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-05T15:05:51+00:00
target_doc: ir-reference/metadata.md
target_doc_source_commit: 52339028a2aa703271533454c6b9528a534bac31
target_doc_watched_paths_digest: 156c66694255ff678fb0eaa18abd2bb50bfa9979d070210c7bb229025fcd0b6b
source_commit: fb192be9f5b3b58555e034599e072158e5c48dfd
checklist:
  factual_accuracy: pass
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: pass
  front_matter_validity: pass
finding_count: 2
severity_breakdown:
  critical: 0
  major: 2
  minor: 0
  nit: 0
---

# Review report for ir-reference/metadata.md

## Summary
The page is broadly source-aligned: front matter is valid, links resolve, and the opcode rows I checked match the recorded Lua declarations. I found two completeness issues against the metadata prompt: the debug table rows do not cite an introducing pass, and the required notable coverage for `Layout` and `DebugScope` is incomplete.

## Items checked
- Ran `python3 docs/generated/design/_meta/regenerate.py show ir-reference/metadata.md`.
- Read `_common.md`, `ir-reference-metadata.md`, the target document, dependency docs, and watched source files at `52339028a2aa703271533454c6b9528a534bac31`.
- Resolved all 12 relative Markdown links at the target source commit.
- Verified front matter keys and checked the target source commit and watched-path digest values against the document front matter.
- Spot-checked more than 10 factual claims across the `Layout`, `Attr`, debug-info, and `SPIRVAsmOperand` rows, including source line ranges, operands, flags, and wrapper names.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `### Debug info family` | The prompt requires at least one debug-info row to cite the IR pass that introduces it, but every debug-info row uses only `(synthesized)` in the `AST origin` column. | `docs/generated/design/_meta/prompts/ir-reference-metadata.md:59-65` requires a debug-info row citation; `source/slang/slang-ir-insts.lua:2747-2784` defines the debug opcodes listed in this table. | Update at least one debug-info row, such as `DebugValue` or `DebugLine`, so its `AST origin` cell names the introducing pass file requested by the prompt. |
| F-002 | major | `## Notable opcodes` | The required notable coverage is incomplete: there is no `DebugScope` callout, and the `Layout` parent is not covered as its own prompt-required topic relating it to `LayoutDecoration`. | `docs/generated/design/_meta/prompts/ir-reference-metadata.md:38-48` requires notable coverage for `Layout`, `VarLayout`, `DebugLine`, `DebugScope`, and `SPIRVAsmOperand`; `source/slang/slang-ir-insts.lua:2652` declares `Layout` and `source/slang/slang-ir-insts.lua:2767` declares `DebugScope`. | Add focused notable callouts for `Layout` and `DebugScope`, or expand existing callouts so those required topics are explicitly covered. |
