---
review_report: true
reviewer_model: gpt-5.6-sol
reviewed_at: 2026-08-04T08:18:09+00:00
target_doc: ir-reference/values.md
target_doc_source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_watched_paths_digest: 64be22b621bde4e26ac349ba999894219b13a0f0d103c6e61d02970a8258d1bc
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: partial
  source_alignment: partial
  front_matter_validity: pass
finding_count: 5
severity_breakdown:
  critical: 0
  major: 2
  minor: 3
  nit: 0
---

# Review report for ir-reference/values.md

## Summary
The page is extensive, well linked, and mostly source-anchored, but it does not fully satisfy the per-document table structure and contains one substantial false producer claim. Most importantly, `ReinterpretOptional` is described as emitted by typeflow even though that path directly builds control flow and no producer of the opcode exists in `source/` at the recorded commit.

## Items checked
- Read the target, `_common.md`, `ir-reference-values.md`, all four dependency documents, and the five resolved watched files at commit `53b76e6d3009b8e6434d41573524c7ce5c499d23`.
- Compared all 100 opcode-table rows with the relevant Lua declarations and spot-checked well over 10 behavior, wrapper, operand, flag, and AST-origin claims against C++ lowering/builders.
- Re-derived every line-number citation in the body against the recorded commit; cited definitions and ranges were present and within the prompt's tolerance.
- Ran the document lint and checked all relative links and generated-peer references; all resolved.
- Checked required section order, per-document subsection requirements, universal style rules, and all mandatory front-matter fields.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Opcodes`, lines 144-464 | The prompt requires each named opcode group to have its own subsection/table, including `Arithmetic and logic` and `Reshape and pack helpers`. The page instead splits arithmetic from logical operations and folds reshape/tuple projection rows into `Aggregate constructors`, so a required subsection is absent. | `docs/generated/design/_meta/prompts/ir-reference-values.md:25-49` says each listed group is its own table. | Reorganize the existing rows under the required subsection names, adding a dedicated `### Reshape and pack helpers` table; do not change opcode ownership while moving rows. |
| F-002 | major | `### Conversions`, line 281; `### Live-but-unproduced opcodes`, lines 701-733 | The page says typeflow emits `ReinterpretOptional` and later counts only four live-but-unproduced opcodes. In fact, the cited typeflow branch calls `openOptional`, which directly builds an if/else helper and returns a call; the only other source mentions lower or consume `ReinterpretOptional`, and no producer exists. | `source/slang/slang-ir-typeflow-set.cpp:255-273` returns `openOptional(...)`; `source/slang/slang-ir-lower-reinterpret.cpp:228-260` only processes existing instances. | Mark `ReinterpretOptional` as having no producer at this commit, remove the false typeflow-origin claim, and update the live-but-unproduced count/list to include it. |
| F-003 | minor | `### Aggregate constructors`, lines 359-375 | Several AST-origin cells omit direct lowering producers: `makeMatrix` and `makeTuple` are produced by `InitializerListExpr`, `makeArrayFromElement` by `MakeArrayFromElementExpr`, `makeValuePack` by `PackExpr`, and `getTupleElement` by `EachExpr`. Calling `makeValuePack` only synthesized is therefore inaccurate. | `source/slang/slang-lower-to-ir.cpp:6542-6562,6757-6765,6865-6887,6962-6970` contains those direct producers. | Add these AST classes/visitors to the respective origin cells and classify `makeValuePack` as both AST-originated and pass-synthesized. |
| F-004 | minor | `### Descriptor-handle conversions`, lines 661-690 | The callout says “Four opcodes on this page convert to and from” `DescriptorHandle<T>`, but the conversions table contains six: two `uint2` casts, two `uint64_t` casts, and the resource/handle pair. The following “other two” wording continues the inconsistent count. | `source/slang/slang-ir-insts.lua:2756-2757,2773-2778` declares all six conversion opcodes. | Change the total to six and describe the three conversion pairs explicitly. |
| F-005 | minor | `### Descriptor-handle conversions`, lines 676-690 | The detailed peephole-fold and Metal buffer-element legalization walkthrough is IR-pass behavior, which this page's prompt forbids. Identifying an opcode as pass-synthesized is appropriate for `AST origin`; explaining specific rewrite pairs and optimization behavior belongs in the pass catalog. | `docs/generated/design/_meta/prompts/ir-reference-values.md:72-75` forbids IR-pass behavior; the described behavior comes from `source/slang/slang-ir-peephole.cpp:1240-1259` and `source/slang/slang-ir-lower-buffer-element-type.cpp:3253-3254`. | Retain only the concise producer classification needed for the opcode table/callout and replace pass mechanics with a link to `../pipeline/05-ir-passes.md`. |

## No-issues notes
- Front matter is complete, uses a full source SHA, and has a valid 64-character hexadecimal watched-path digest.
- Literal flags correctly distinguish constant-map deduplication from the `H` opcode flag.
- The scalar-versus-vector/global behavior of `select` is accurately qualified.
