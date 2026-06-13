---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-12T12:06:07+00:00
target_doc: ir-reference/decorations.md
target_doc_source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_watched_paths_digest: c993f7837f8ee2af868f6b993bef4697dfae2a5a4522346050a4c431dadfeb19
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

# Review report for ir-reference/decorations.md

## Summary
No findings were identified. The page now covers the prompt-required semantic buckets and notable decoration callouts, and sampled decoration rows aligned with the Lua `Decoration` family and wrapper declarations.

## Items checked
- Ran `regenerate.py show ir-reference/decorations.md` and used its prompt path, watched files, and dependencies.
- Read `_common.md`, `ir-reference-decorations.md`, the full target document, `cross-cutting/ir-instructions.md`, `pipeline/04-ast-to-ir.md`, and `ast-reference/modifiers.md`.
- Verified front matter keys, target source commit, watched-path digest shape, required IR-reference sections, table columns, and all relative links via source inspection plus pending lint.
- Compared the decoration table buckets with the `Decoration` Lua range in `source/slang/slang-ir-insts.lua`, including target-specific, linkage, autodiff, debug, SPIR-V, and catch-all rows.
- Spot-checked more than 10 factual claims: `nameHint`, `layout`, `targetIntrinsic`, `intrinsicOp`, `branch`, `flatten`, `loopControl`, `KeepAliveDecoration`, `BuiltinRequirementDecoration`, `diffInstDecoration`, `DifferentiableTypeDictionaryDecoration`, `DebugLocation`, and `DisallowSpecializationWithExistentialsDecoration`.

## Findings
(no findings)
