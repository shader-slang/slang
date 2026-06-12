---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-05T15:06:28+00:00
target_doc: ir-reference/structure.md
target_doc_source_commit: 52339028a2aa703271533454c6b9528a534bac31
target_doc_watched_paths_digest: 5ac7df35674b391db414495e8be54b9c8c58690cd2b324a3a4c6804a1748f586
source_commit: fb192be9f5b3b58555e034599e072158e5c48dfd
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 2
severity_breakdown:
  critical: 0
  major: 2
  minor: 0
  nit: 0
---

# Review report for ir-reference/structure.md

## Summary
The page has valid front matter, required top-level sections, and all relative links resolve at the recorded source commit. Two major findings remain: the `## Source` section cites several lowering symbols that are not present under those names, and the witness-table row omits the concrete-type operand while not providing the prompt-required `witness_table` callout.

## Items checked
- Ran `python3 docs/generated/design/_meta/regenerate.py show ir-reference/structure.md` and verified the prompt, dependency list, watched paths, target source commit, and digest against the document front matter.
- Read the target doc, `_common.md`, `ir-reference-structure.md`, and the dependency docs `cross-cutting/ir-instructions.md`, `pipeline/04-ast-to-ir.md`, and `ast-reference/declarations.md`.
- Resolved all 24 relative links in the target document against `52339028a2aa703271533454c6b9528a534bac31`.
- Checked all required IR-reference sections, the `GlobalValueWithCode` hierarchy, and 22 opcode table rows against `source/slang/slang-ir-insts.lua`.
- Spot-checked more than 10 factual claims, including `module`, `func`, `generic`, `param`, `call`, `global_var`, `global_param`, `globalConstant`, `StructKey`, `InterfaceRequirementEntry`, `WitnessTableEntry`, `thisTypeWitness`, `TypeEqualityWitness`, `SymbolAlias`, and the named lowering helpers.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Source`, lines 41-47 | The source paragraph cites `lowerProgram`, `lowerCallableDecl`, `lowerGenericDecl`, `lowerStructDecl`, `lowerInterfaceDecl`, and `lowerInheritanceDecl`, but the watched implementation does not define those symbols under those names. Related implemented entry points are named differently, such as `lowerFuncDecl`, `visitGenericDecl`, `visitInterfaceDecl`, and `visitInheritanceDecl`. | `source/slang/slang-lower-to-ir.cpp:10671` defines `visitInheritanceDecl`, `source/slang/slang-lower-to-ir.cpp:11520` defines `visitInterfaceDecl`, and `source/slang/slang-lower-to-ir.cpp:14089` defines `lowerFuncDecl`. Struct lowering is handled by aggregate logic that calls `createStructType` at `source/slang/slang-lower-to-ir.cpp:11991`. | Replace the nonexistent helper names with the actual visitor or helper names, or phrase this as declaration lowering handled by the lowering visitor and cite the concrete functions that exist. |
| F-002 | major | `## Opcodes` and `## Notable opcodes`, lines 134-139 and 147-214 | The `witness_table` row describes only child entries, while source creation records the concrete conforming type as operand 0 and the conformance interface in the result type. The prompt also requires a notable `witness_table` callout, but the notable section only discusses `witness_table_entry` versus `interface_req_entry`. | `source/slang/slang-ir.cpp:4992-4998` creates `kIROp_WitnessTable` with `getWitnessTableType(baseType)` and `subType`; `source/slang/slang-ir-insts.h:2211-2216` exposes `getConformanceType()` and `getConcreteType()`; `source/slang/slang-ir.cpp:5003-5017` inserts `witness_table_entry` children into the table. | Update the row to mention the concrete-type operand and result-type conformance, then add a `### witness_table` notable callout that explains those two pieces plus its owned entries. |
