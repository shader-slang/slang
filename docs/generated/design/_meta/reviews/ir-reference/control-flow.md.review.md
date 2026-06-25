---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-12T12:06:07+00:00
target_doc: ir-reference/control-flow.md
target_doc_source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_watched_paths_digest: 50a5584b2851342292d4b982e8c4767f3127bd44d5e4d4de95333b7b3e0e7fa5
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

# Review report for ir-reference/control-flow.md

## Summary
No findings were identified. The page satisfies the control-flow prompt structure, covers the required `block`, `Param`, and `TerminatorInst` entries, and the sampled source claims matched the watched files at the target source commit.

## Items checked
- Ran `regenerate.py show ir-reference/control-flow.md` and used its prompt path, watched files, and dependencies.
- Read `_common.md`, `ir-reference-control-flow.md`, the full target document, `cross-cutting/ir-instructions.md`, `pipeline/04-ast-to-ir.md`, and `ast-reference/statements.md`.
- Verified front matter keys, target source commit, watched-path digest shape, required IR-reference sections, table columns, and all relative links via source inspection plus pending lint.
- Checked all 25 opcode table rows against `source/slang/slang-ir-insts.lua`, including `block`, `param`, every `TerminatorInst` leaf, `discard`, `gpuForeach`, and adjacent `Require*`/assertion rows.
- Spot-checked more than 10 factual claims: Lua line ranges, `IRBlock`/`IRParam` wrappers, `Parent` flag on `block`, `Return` wrapper, branch operand counts, `tryCall` operands, `defer` operands, `discard` non-terminator status, statement visitor origins, and flag meanings in `slang-ir.h`.

## Findings
(no findings)
