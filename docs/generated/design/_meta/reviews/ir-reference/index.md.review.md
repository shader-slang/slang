---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-12T12:06:07+00:00
target_doc: ir-reference/index.md
target_doc_source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_watched_paths_digest: f57f85851515cdaf74d296849163598144fd9446405e6588774aac17250d6d39
source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
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
No findings were identified. The navigation page includes all family links, required cross-cutting topics, the target-backends link, and sampled approximate counts and Lua-root claims are reasonable for an index-level summary.

## Items checked
- Ran `regenerate.py show ir-reference/index.md` and used its prompt path, watched files, and dependency list.
- Read `_common.md`, `ir-reference-index.md`, the full target document, all listed IR-reference dependency pages, and `source/slang/slang-ir-insts.lua`.
- Verified front matter keys, target source commit, watched-path digest shape, required index sections, Pages table columns, mermaid taxonomy coverage, and all relative links via source inspection plus pending lint.
- Checked that the `## Pages` table links every peer family page and that the taxonomy diagram includes types, values, structure, control flow, generics/existentials, resources/atomics, differentiation, decorations, metadata, and misc.
- Spot-checked more than 10 factual claims: Lua roots for `Type`, `Constant`, `GlobalValueWithCode`, `TerminatorInst`, `specialize`, `AtomicOperation`, `MakeDifferentialPairBase`, `Decoration`, `Layout`, `Attr`, `SPIRVAsmOperand`, abstract/group-only examples, and AST-origin guidance from `slang-lower-to-ir.cpp`.

## Findings
(no findings)
