---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-05T15:05:36+00:00
target_doc: ir-reference/generics-and-existentials.md
target_doc_source_commit: 52339028a2aa703271533454c6b9528a534bac31
target_doc_watched_paths_digest: 5ac7df35674b391db414495e8be54b9c8c58690cd2b324a3a4c6804a1748f586
source_commit: fb192be9f5b3b58555e034599e072158e5c48dfd
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: pass
  front_matter_validity: pass
finding_count: 2
severity_breakdown:
  critical: 0
  major: 1
  minor: 1
  nit: 0
---

# Review report for ir-reference/generics-and-existentials.md

## Summary
The page covers the main generic, existential, RTTI, type-flow, and AnyValue rows, and all relative links resolved. Two issues remain: witness-table and witness-fact opcodes are summarized as bullets instead of required opcode-table rows, and the source section omits the later Lua range that owns the type-flow rows.

## Items checked
- Ran `regenerate.py show ir-reference/generics-and-existentials.md` and checked the resolved watched files and dependencies.
- Verified front matter keys, target source commit, watched-path digest shape, 11 relative links, required IR-reference sections, and table columns.
- Checked 52 opcode table rows against `slang-ir-insts.lua`, including generic application, witness lookup, existential construction and projection, RTTI, type-flow sets, tag operations, dispatchers, specialization keys, and AnyValue marshalling.
- Spot-checked more than 10 factual claims about operand counts, hoistable and global flags, wrappers, AST origins, and notable opcode behavior against `slang-ir-insts.lua`, `slang-ir-insts.h`, `slang-ir.h`, `slang-ir.cpp`, and `slang-lower-to-ir.cpp`.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `### Witness tables and witness facts`, lines 126-141 | The witness-table and witness-fact opcodes are summarized as bullets instead of rows in an opcode table with the required columns. | `source/slang/slang-ir-insts.lua:816-824` defines `key`, `global_generic_param`, `witness_table`, `indexedFieldKey`, `thisTypeWitness`, and `TypeEqualityWitness`; `source/slang/slang-ir-insts.lua:1042-1048` defines `witness_table_entry` and `interface_req_entry`. | Replace the bullet list with a sub-table covering the concrete witness and witness-fact opcodes, using the standard Opcode, C++ wrapper, Operands, Flags, AST origin, and Summary columns. |
| F-002 | minor | `## Source`, lines 24-41 | The source range list omits the later type-flow specialization range even though the page documents `TypeSet`, dispatcher, tagged-union, tag, and specialization-key opcodes from that range. | `source/slang/slang-ir-insts.lua:2926-2929` defines the set opcodes, `source/slang/slang-ir-insts.lua:2979-2991` defines `GetDispatcher`, and `source/slang/slang-ir-insts.lua:3033-3162` defines tagged-union and specialization-key opcodes. | Add the type-flow specialization range around lines 2916-3162 to the `## Source` range list. |
