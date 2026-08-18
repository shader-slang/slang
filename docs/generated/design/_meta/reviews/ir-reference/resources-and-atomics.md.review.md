---
review_report: true
reviewer_model: gpt-5.6-sol
reviewed_at: 2026-08-04T08:18:17+00:00
target_doc: ir-reference/resources-and-atomics.md
target_doc_source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_watched_paths_digest: 64be22b621bde4e26ac349ba999894219b13a0f0d103c6e61d02970a8258d1bc
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
checklist:
  factual_accuracy: pass
  cross_references: pass
  completeness: fail
  style_consistency: fail
  source_alignment: partial
  front_matter_validity: pass
finding_count: 4
severity_breakdown:
  critical: 0
  major: 3
  minor: 1
  nit: 0
---

# Review report for ir-reference/resources-and-atomics.md

## Summary

The page's source citations and factual claims are unusually well grounded, but it does not conform to the IR-reference contract. The most important issue is that every opcode table substitutes a producer-oriented column for the mandatory AST-origin column, and much of that replacement content is target-specific lowering that the per-document prompt explicitly forbids.

## Items checked

- Reviewed the target page, `_common.md`, the per-document prompt, all four dependency documents, and the five resolved watched files reported by `regenerate.py show`.
- Verified all 190 cited source line positions and range endpoints across 22 files against commit `53b76e6d3009b8e6434d41573524c7ce5c499d23`; the cited symbols or operations occur at those lines.
- Checked more than 10 factual claims in detail, including Lua operand layouts, generated and hand-written wrappers, atomic memory-order positions, image-subscript legalization, unused sampling helpers, byte-address-store operand order, descriptor-heap construction, Metal mesh builders, and `fixedArgCount` usage.
- Resolved the page's linked source files and generated peer pages at the recorded commit and confirmed the referenced generated pages are manifest entries.
- Checked front matter, section order, table schemas, hierarchy semantics, notable-opcode formatting, the 65,536-byte size cap, and prompt-specific forbidden content.

## Findings

| ID    | Severity | Location                                                                                       | Description                                                                                                                                                                                                                                                                    | Evidence                                                                                                                                                                                                                                                                         | Recommendation                                                                                                                                                                            |
| ----- | -------- | ---------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| F-001 | major    | `## Opcodes`, lines 164-383                                                                    | Every opcode table uses `Produced by` instead of the mandatory `AST origin` column. The cells consequently describe core-module declarations, builder calls, or pass producers rather than the AST class or required `(synthesized)` / `—` classification.                     | `docs/generated/design/_meta/prompts/_common.md:230-241` fixes the six columns and defines `AST origin`; for example, the target's `imageLoad` row at line 167 names `IRBuilder::emitImageLoad` and an IR pass.                                                                  | Rename the fifth column to `AST origin` in every subtable and replace each producer description with the originating AST class, `(synthesized)`, or `—` as the contract specifies.        |
| F-002 | major    | `## Family hierarchy`, lines 131-158                                                           | The diagram is a conceptual taxonomy, not a mirror of Lua nesting: nodes such as `Images`, `Samplers`, `Buffers`, `ShaderIO`, and `Descriptors` are not Lua grouping entries. At this commit the page's actual grouping parents are `AtomicOperation` and `BindingQuery`.      | The hierarchy contract is `docs/generated/design/_meta/prompts/_common.md:226-229`. The real parent entries are `source/slang/slang-ir-insts.lua:1186-1212` and `source/slang/slang-ir-insts.lua:1736-1750`; the other listed resource opcodes are direct leaf entries.          | Replace the conceptual taxonomy with a small `flowchart TD` showing the actual `AtomicOperation` and `BindingQuery` nesting; keep topical organization in the opcode subsection headings. |
| F-003 | major    | `## Opcodes` and `## Notable opcodes`, especially lines 272-275, 432-455, 513-520, and 624-651 | The page repeatedly documents target-specific lowering and emitter behavior, including SPIR-V atomic scope and image operands, CUDA reduction assembly, Metal emission, and backend-specific latent paths. This material is expressly forbidden for this page.                 | `docs/generated/design/_meta/prompts/ir-reference-resources-and-atomics.md:70-77` sends target-specific lowering to `pipeline/06-emit.md` and the emit files.                                                                                                                    | Remove backend-specific lowering and emitter details from rows and callouts; retain only opcode shape and origin, then link `pipeline/06-emit.md` for target behavior.                    |
| F-004 | minor    | `## Opcodes`, buffer and Shader IO tables, lines 191-234                                       | The prompt-specific quality check requires at least one buffer row and one shader-IO row to cite a `slang-lower-to-ir.cpp` visitor. No buffer row cites that file, and the shader-IO rows cite either the file without a visitor or a builder call without naming its visitor. | `docs/generated/design/_meta/prompts/ir-reference-resources-and-atomics.md:79-84` states the requirement. Relevant visitor entry points include `visitInvokeExpr` at `source/slang/slang-lower-to-ir.cpp:7172` and `visitVarDecl` at `source/slang/slang-lower-to-ir.cpp:11927`. | Add a visitor citation to at least one buffer AST-origin cell and one shader-IO AST-origin cell, naming the visitor and linking `slang-lower-to-ir.cpp`.                                  |

## No-issues notes

- All recorded line-number citations matched the historical source exactly.
- The page is 53,631 bytes, below its 65,536-byte cap.
- Front matter contains every mandatory key and a valid 64-character hexadecimal digest.
