---
review_report: true
reviewer_model: gpt-6
reviewed_at: 2026-11-03T08:06:53Z
target_doc: ir-reference/differentiation.md
target_doc_source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_watched_paths_digest: a4029edc3abd590ad67edfef94772eff296140ceb8dff5e1e1d47a84eaada224
source_commit: 8341c7e537a3f5cb3a60cd062066ffd630da40d8
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: partial
  source_alignment: partial
  front_matter_validity: pass
finding_count: 8
severity_breakdown:
  critical: 3
  major: 1
  minor: 4
  nit: 0
---

# Review report for ir-reference/differentiation.md

## Summary

The document provides substantial, mostly accurate opcode coverage, but needs corrections before acceptance. Most importantly, it attributes synthesized derivative witness opcodes to the wrong AST representation and lowering path. Its blanket statements about SPIR-V names and projection folding also contradict source behavior.

## Items checked

- Read `_review.md`, `_common.md`, the document-specific prompt, and `regenerate.py show ir-reference/differentiation.md`.
- Verified against the **current working tree**, which equals the document’s recorded source commit for all of `source/`, as specified by the operator.
- Checked more than 20 behavioral claims, including intrinsic origins, synthesized-declaration lowering, translation tuple indices, checkpoint reporting, weak references, finalization gates, wrapper generation, and unused placeholder helpers.
- Checked every explicit source line-number citation in the body, including ranges and repeated citations.
- Compared all 39 opcode rows with Lua declarations and checked the 17 marked hand-written wrappers.
- Resolved all 85 Markdown link occurrences, representing 39 distinct destinations; checked generated-page manifest membership and linked heading targets. Consulted dependency documents only for cross-reference checks.
- Checked required front-matter fields, section coverage, table conventions, Mermaid syntax, and the 46,439-byte document against its 65,536-byte cap.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | `## Source`, lines 74–89; synthesized derivative witness rows, lines 292–293 | The rows identify `SynthesizedFuncDecl` as the AST origin of `SynthesizedForwardDerivativeWitnessTable` and `SynthesizedBackwardDerivativeWitnessTable`, reinforced by the assertion that all listed checking origins use the synthesized-function lowering site. These witness opcodes instead arise from synthesized inheritance declarations carrying a `SynthesizedModifier`. | `source/slang/slang-check-decl.cpp:9877` and `:16261` pass the witness opcode to `extendContainerDecl`; `source/slang/slang-lower-to-ir.cpp:11138–11165` reads the inheritance declaration’s modifier and emits `(IROp)synModifier->op`. | Change both rows to name `InheritanceDecl` carrying `SynthesizedModifier`, produced through `extendContainerDecl` and lowered by `visitInheritanceDecl`. Add this distinct path to Source and qualify the claim that all origins use synthesized-function lowering. Preserve the separately documented forward-pass producers. |
| F-002 | critical | `#### Reverse-mode`, lines 248–263 | The categorical assertion that SPIR-V output carries `no OpName for the derivative function` is false. The cited autodiff code creates name hints; the SPIR-V emitter handles function name hints through its ordinary decoration path. The surrounding guarantee that names appear in every text output is also stronger than the cited name-generation code establishes. | `source/slang/slang-ir-autodiff-rev.cpp:405` generates the propagate name; `source/slang/slang-emit-spirv.cpp:4505` emits function decorations; `:6327–6335` converts `IRNameHintDecoration` to `OpName`. | Describe the prefixes as generated IR name hints. State that surviving function hints can become SPIR-V `OpName` records; remove unconditional guarantees about names surviving optimization or appearing in every output. |
| F-003 | critical | `### GetDifferential / GetPrimal`, lines 402–403 | `No pass folds GetPrimal(MakeDiffPair(a, b)) down to a before that rewrite` contradicts the unzip pass. For a mixed pair, unzip records its components and replaces a subsequent primal projection with the recorded primal. | `source/slang/slang-ir-autodiff-unzip.cpp:433–436` returns the pair’s original components; `:710–717` records them; `:796–803` replaces and removes the primal projection when its base is mapped. | Remove the blanket statement. Explain briefly that unzip can eliminate projections of mapped pairs before final pair-to-struct lowering. |
| F-004 | minor | `### Differentiation operators`, lines 200–225 | The deduplication key is described as `opcode plus operands`, and a second request is said to require a different base function. The full result type also participates in identity, so equal operands alone do not guarantee deduplication. | `source/slang/slang-ir.cpp:2246–2250` hashes `getFullType()`; `source/slang/slang-ir.h:1962–1971` explicitly rejects keys whose full types differ. | Include the full result type in the key description and qualify the example as requests having identical opcode, type, and operands. |
| F-005 | major | `## Opcodes`; operand legend, lines 29–31 | Numerous unnamed-operand cells use `† min=1`, `† min=2`, or `† min=3`, contrary to the required `(N unnamed)` spelling. The legend repurposes `†`, which the contract reserves for meaningful names recovered from C++ accessors. Two unused-opcode rows also substitute `none at this commit` for the mandatory producer label. | `docs/generated/design/_meta/prompts/_common.md:272–274` specifies both conventions; `source/slang/slang-ir-insts.lua:2857–2891` supplies the unnamed operand counts. | Replace these cells with `(1 unnamed)`, `(2 unnamed)`, or `(3 unnamed)`. Where accessors justify names, use those names with `†` and correct the legend. Change the `DiffTypeInfo` and unused legacy-witness origins to `no producer at HEAD`. |
| F-006 | minor | `### checkpointObj and ReportCheckpointStore`, lines 591–594 | The citation to line `652` points to the weak-reference rule for `Annotation`, not the checkpoint marker. | `source/slang/slang-ir-dce.cpp:645–649` handles `ReportCheckpointStore` operand 2; `:651–653` handles `Annotation` operand 0. | Replace the citation with lines 645–649. |
| F-007 | minor | `## See also`, lines 722–749 | The required relevant AST-reference links are absent despite the table’s substantial use of expression, declaration, and value origins. | `docs/generated/design/_meta/prompts/_common.md:288–295` requires relevant AST-reference pages. Existing targets include `docs/generated/design/ast-reference/expressions.md`, `declarations.md`, and `values.md`. | Add relative links to the applicable AST-reference pages, identifying the differentiation expressions, synthesized declarations, and differentiation values they cover. |
| F-008 | minor | `### C++ wrappers: hand-written vs generated`, lines 670–675 | `The two abstract pair bases` miscounts the bases described immediately afterward: one construction base and two projection bases. | `source/slang/slang-ir-insts.h:2457`, `:2466`, and `:2474` declare `IRMakeDifferentialPairBase`, `IRDifferentialPairGetDifferentialBase`, and `IRDifferentialPairGetPrimalBase`. | Change `two abstract pair bases` to `three abstract pair bases`. |