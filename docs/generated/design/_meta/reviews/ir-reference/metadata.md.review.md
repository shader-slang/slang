---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-12T12:06:07+00:00
target_doc: ir-reference/metadata.md
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

# Review report for ir-reference/metadata.md

## Summary
No findings were identified. The page covers the required `Layout`, `Attr`, debug-info, and `SPIRVAsmOperand` families, includes the prompt-required notable topics, and sampled row details matched the Lua and builder/helper sources.

## Items checked
- Ran `regenerate.py show ir-reference/metadata.md` and used its prompt path, watched files, and dependencies.
- Read `_common.md`, `ir-reference-metadata.md`, the full target document, `cross-cutting/ir-instructions.md`, and `pipeline/04-ast-to-ir.md`.
- Verified front matter keys, target source commit, watched-path digest shape, required IR-reference sections, table columns, and all relative links via source inspection plus pending lint.
- Checked the concrete rows under `Layout`, `Attr`, debug-info, `SPIRVAsm`, `SPIRVAsmInst`, and `SPIRVAsmOperand` against `source/slang/slang-ir-insts.lua`.
- Spot-checked more than 10 factual claims: `Layout` hoistability, `varLayout` operands, `TypeLayoutBase`, `PointerTypeLayout`, `EntryPointLayout`, `stage`, `structFieldLayout`, `DebugSource`, `DebugLine`, `DebugValue` source origin, `EmbeddedDownstreamIR`, `SPIRVAsm` parent flag, `SPIRVAsmOperandInst` non-hoistability, and sampled/image/truncate operand wrappers in `slang-ir.cpp`.

## Findings
(no findings)
