---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-05T15:05:36+00:00
target_doc: ir-reference/control-flow.md
target_doc_source_commit: 52339028a2aa703271533454c6b9528a534bac31
target_doc_watched_paths_digest: 5ac7df35674b391db414495e8be54b9c8c58690cd2b324a3a4c6804a1748f586
source_commit: fb192be9f5b3b58555e034599e072158e5c48dfd
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

# Review report for ir-reference/control-flow.md

## Summary
No findings were identified in this independent review. The page covers the required control-flow opcode set, resolves its relative links, and the sampled source claims matched the watched files at the target source commit.

## Items checked
- Ran `regenerate.py show ir-reference/control-flow.md` and checked the resolved watched files and dependencies.
- Verified front matter keys, target source commit, watched-path digest shape, 12 relative links, required IR-reference sections, and table columns.
- Checked all 25 opcode table rows against `slang-ir-insts.lua`, including `block`, `param`, every `TerminatorInst` leaf, `discard`, `gpuForeach`, and the adjacent requirement or assertion opcodes.
- Spot-checked more than 10 factual claims about operand counts, wrappers, flags, AST origins, and notable opcode behavior against `slang-ir-insts.lua`, `slang-ir-insts.h`, `slang-ir.h`, `slang-ir.cpp`, and `slang-lower-to-ir.cpp`.

## Findings

(no findings)
