---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-12T12:06:07+00:00
target_doc: ir-reference/generics-and-existentials.md
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

# Review report for ir-reference/generics-and-existentials.md

## Summary
No findings were identified. The page now includes table rows for witness-table facts and the type-flow specialization cluster, and sampled operand, flag, and source-origin claims matched the watched files.

## Items checked
- Ran `regenerate.py show ir-reference/generics-and-existentials.md` and used its prompt path, watched files, and dependencies.
- Read `_common.md`, `ir-reference-generics-and-existentials.md`, the full target document, `cross-cutting/ir-instructions.md`, `pipeline/04-ast-to-ir.md`, `ast-reference/declarations.md`, `ir-reference/types.md`, and `ir-reference/structure.md`.
- Verified front matter keys, target source commit, watched-path digest shape, required IR-reference sections, table columns, and all relative links via source inspection plus pending lint.
- Checked the documented generic application, witness lookup, existential construction/destructuring, witness-table facts, RTTI, type-flow specialization, tagged-union, dispatcher, specialization-key, and AnyValue rows against `source/slang/slang-ir-insts.lua`.
- Spot-checked more than 10 factual claims: `specialize` operands and `H` flag, `lookupWitness` wrapper and minimum operands, `makeExistential` operands, existential projection flags, `interface_req_entry` `G` flag, `builtinRequirementKey` operands and `H` flag, `GetSequentialID`, `GetDynamicResourceHeap`, `GetDispatcher` key type, `SpecializeExistentialsInFunc` operands, `WeakUse`, and `packAnyValue`/`unpackAnyValue` behavior.

## Findings
(no findings)
