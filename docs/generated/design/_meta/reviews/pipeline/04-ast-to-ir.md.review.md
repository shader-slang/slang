---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-30T13:32:43+00:00
target_doc: pipeline/04-ast-to-ir.md
target_doc_source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_watched_paths_digest: 8a4f880d8b981d67ad88abf7aa75a535d3a572cbe530098477dde31879574746
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

# Review report for pipeline/04-ast-to-ir.md

## Summary

The document satisfies the requested structure, front matter, link shape, and most sampled source claims. The single issue is a factual mismatch in the `IRBuilder` section: it groups `Global` opcodes with hoistable opcodes as deduplicated values, but the source explicitly says global opcodes are hoisted and never deduplicated.

## Items checked

- Read the target document, `_common.md`, the per-doc prompt, and dependency docs `pipeline/03-semantic-check.md` and `cross-cutting/ir-instructions.md`.
- Resolved the manifest data with `regenerate.py show pipeline/04-ast-to-ir.md` and sampled claims against the watched files `slang-lower-to-ir.h`, `slang-lower-to-ir.cpp`, `slang-ir.h`, `slang-ir.cpp`, `slang-ir-insts.h`, and `slang-ir-insts.lua`.
- Verified 14 concrete source claims, including the three public lowering entry points, `generateIRForTranslationUnit` entry-point/member lowering, `lowerBuiltinOperatorExpr`, `visitBuiltinOperationIntVal`, `getInterfaceRequirementKey`, pack-count witness lowering, unsupported-assignment diagnostics, synthesized-constructor debug-info gating, `IRModule::create`, `IRBuilder`, and the IR opcode enum include.
- Checked that the required sections from `pipeline-04-ast-to-ir.md` are present and that the front matter contains the mandatory generated-doc keys.
- Checked the relative links used by this document to dependency docs, peer generated docs, source files, and handwritten design docs.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | `## IRBuilder and instruction creation`, lines 64-69 | The document says `IRBuilder` hash-conses both `hoistable` and `global` values and that `kIROpFlag_Global` opcodes "take part in this deduplication." The source distinguishes the two: hoistable instructions are deduplicated, while global instructions are hoisted but not deduplicated. | `source/slang/slang-ir.h:53-56` says `kIROpFlag_Hoistable` is "a hoistable inst that needs to be deduplicated" and `kIROpFlag_Global` "should always be hoisted but should never be deduplicated." | Revise this bullet to say that `IRBuilder` deduplicates hoistable instructions, while global instructions are always hoisted to module scope but are not deduplicated. Do not list `kIROpFlag_Global` as a deduplication flag. |

## No-issues notes

- The lowering-driver section matches the public signatures and header comments in `source/slang/slang-lower-to-ir.h`.
- The built-in operator section matches `lowerBuiltinOperatorExpr`, including `%` selecting `FRem` versus `IRem` and the `?:` / `&&` / `||` unexpected cases.
- The built-in requirement-key discussion matches `getInterfaceRequirementKey`, including the `IRInst*` cache, `BuiltinRequirementModifier`, hoistable `IRBuiltinRequirementKey`, and `IRBuiltinRequirementDecoration`.
- The pack-count witness section matches the hidden `WitnessTableType(void)` parameter and module-level proof-only witness table implementation.
