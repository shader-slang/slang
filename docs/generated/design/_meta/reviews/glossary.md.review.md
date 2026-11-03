---
review_report: true
reviewer_model: gpt-6
reviewed_at: 2026-11-03T06:29:50Z
target_doc: glossary.md
target_doc_source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_watched_paths_digest: efe086be34eebac58105bf41e309b1c28bad3c7de409c3ef1a524f4e6cd23cd9
source_commit: cd566e9f0e7783ebfbac93c6e9957b63c4f2e64e
checklist:
  factual_accuracy: partial
  cross_references: partial
  completeness: pass
  style_consistency: partial
  source_alignment: partial
  front_matter_validity: pass
finding_count: 8
severity_breakdown:
  critical: 2
  major: 1
  minor: 4
  nit: 1
---

# Review report for glossary.md

## Summary

The glossary covers the required vocabulary and sections, but needs corrections before acceptance. Most importantly, its SSA definition incorrectly attributes block-argument passing to conditional branches and switches. It also overstates the builtin-operator fast path, links to a removed language glossary, and violates several entry-format constraints.

## Items checked

- Reviewed against the **current working tree**, which the operator confirms equals the document’s source commit for all of `source/`. Confirmed current HEAD is `cd566e9f0e7783ebfbac93c6e9957b63c4f2e64e`.
- Read `_review.md`, `_common.md`, the glossary prompt, and `regenerate.py show glossary.md`; used dependency documents only for cross-reference checks.
- Verified all **10 numeric line citations**: `DeclCheckState` 478; `FrontEndEntryPointRequest` 71; `Linkage` 91; modifier classes 647 and 671; `calcRequiredLoweringPassSet` 409; Metal/WGSL legalization calls 2422 and 2452; `Session` 109; `TranslationUnitRequest` 22. All match.
- Spot-checked more than 10 additional claims, including conversion-cost computation, branch operands, builtin-operator resolution, standard modules, lookup masks/options, decl-ref subclasses, fossil pointers, hoisting, modifier fields, wrapper generation, and SPIR-V legalization placement.
- Checked all **235 relative-link occurrences**, representing **95 distinct targets**. One target is missing; all linked generated pages are manifest entries.
- Checked all **74 entries** for tags and mandatory `See:` links; verified required vocabulary, section order, and index coverage of all six dependencies.
- Verified required front-matter keys and formats, the 54,676-byte size against the 102,400-byte cap, and absence of raw Liquid openers. No files were written.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | Conventions, line 35 | `../../language-reference/glossary.md` resolves to a nonexistent file, breaking the required runtime-vocabulary cross-reference. | `docs/language-reference/README.md:3` states that the language reference moved to `shader-slang/spec`; `docs/language-reference/glossary.md` is absent. | Link to `../../language-reference/README.md` and explain that it directs readers to the relocated reference. |
| F-002 | critical | “single static assignment (SSA),” lines 784–791; related wording at 62–68 and 890–892 | The entry says `conditionalBranch`, `ifElse`, and `Switch` pass one argument per destination parameter. These instructions do not have that block-argument encoding. This gives readers an incorrect model for constructing or interpreting IR. | `source/slang/slang-ir.cpp:752` and `:768` implement branch argument access only for `UnconditionalBranch` and `Loop`. `:6532` constructs `ifElse` with condition and three block operands. `source/slang/slang-ir-insts.lua:1508` defines switch operands as value, break/default blocks, and case-value/block pairs. | Name `unconditionalBranch` and `loop` as the argument-carrying branches. Qualify the block-parameter and terminator entries so they do not imply that every branch opcode carries destination arguments. |
| F-003 | critical | “overload resolution,” lines 612–620 | `Builtin operators on scalar, vector, and matrix operands never reach that pipeline` is too broad. The fast path deliberately falls back to normal resolution for some builtin operands, including matrices in GLSL operator scope. | `source/slang/slang-check-expr.cpp:4865` documents that `convertToBuiltinArithmeticOp` returns null for normal resolution. Around `:4903`, `isGLSLOperatorScope() && as<MatrixExpressionType>(uOperandType)` explicitly returns null. | Describe the fast path as handling supported builtin arithmetic operators, and state that unsupported cases and GLSL-scope matrix operations retain normal overload resolution. Remove `never`. |
| F-004 | minor | “conversion cost,” lines 102–105 | The entry incorrectly attributes summing per-argument costs to `getImplicitConversionCostWithKnownArg`. That helper adjusts the cost of one conversion. | `source/slang/slang-check-conversion.cpp:1656` accepts one `Expr* arg` and returns `candidateCost`; `source/slang/slang-check-overload.cpp:919` performs `candidate.conversionCostSum += cost`. | Attribute accumulation to overload checking in `slang-check-overload.cpp`; describe the cited conversion helper only as adjusting an individual conversion cost. |
| F-005 | minor | “standard module,” lines 127–129 | `Three exist` contradicts both the four names immediately listed and the source’s four module groups. | `source/standard-modules/CMakeLists.txt:54–64` adds `neural`, `experimental`, `numerics`, and `functional`. | Change `Three exist` to `Four module groups exist`, retaining the four names. |
| F-006 | nit | Terms, lines 120–135 and 439–467 | The list is not alphabetically ordered: `standard module` occurs between `core module` and `dataflow analysis`; `link-time symbol alias` follows `linkage`. | `docs/generated/design/_meta/prompts/glossary.md:5–6` requires an alphabetically ordered glossary; its required structure specifies a flat A–Z list. | Move `standard module` between `specialization` and `syntax-decl`; place `link-time symbol alias` before `linkage`. |
| F-007 | minor | Entries including “capability atom,” “decl-ref,” “fossil format,” and “target legalization driver” | Multiple entries exceed the explicit limit of one concrete code anchor; the legalization-driver entry alone links five source files. Several entries also exceed the prescribed 2–4 sentences. | `docs/generated/design/_meta/prompts/glossary.md:68–70` requires one short paragraph of 2–4 sentences and at most one concrete code anchor. | Keep one defining source anchor per entry, shorten definitions to 2–4 sentences, and leave implementation walkthroughs to the existing `See:` destinations. |
| F-008 | minor | “standard module,” lines 130–133 | The entry includes the command-line option `-experimental-feature`, which the glossary contract explicitly excludes. | `docs/generated/design/_meta/prompts/glossary.md`, “Forbidden content”: `Do not include compiler flags or command-line options`. | Remove the option spelling and invocation guidance. If needed, retain only that these modules require experimental-feature enablement and defer details to the linked core-module page. |