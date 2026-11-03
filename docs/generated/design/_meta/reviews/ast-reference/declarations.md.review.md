---
review_report: true
reviewer_model: gpt-6
reviewed_at: 2026-11-03T06:57:41+00:00
target_doc: ast-reference/declarations.md
target_doc_source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_watched_paths_digest: 9bd5adedc83ca3a7808fb8f71217d7c3156e83b21e5365d11e8a6458233d96bf
source_commit: 56b1b0b83fb96f88f36da7e37cd9c2c7f27af70c
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: partial
  source_alignment: partial
  front_matter_validity: pass
finding_count: 4
severity_breakdown:
  critical: 0
  major: 1
  minor: 3
  nit: 0
---

# Review report for ast-reference/declarations.md

## Summary

The document covers every concrete declaration class and its links resolve, but it needs corrections to meet the reference-page contract. The most important omission is inherited data-shape information: several substantial nodes have only `(inherits)` in their Key fields cells. One attribute-construction claim also names the wrong producer, and the notable-node sections exceed their prescribed format and scope.

## Items checked

- Reviewed against the CURRENT working tree, which the operator states equals the document’s source commit for all of `source/`; no files were written.
- Read `_review.md`, `_common.md`, and the document-specific prompt; ran `regenerate.py show ast-reference/declarations.md`.
- Matched all 64 concrete FIDDLE-declared classes to table rows and all 12 abstract intermediates to the hierarchy diagram.
- Spot-checked more than 10 claims, including generic ownership, default-implementation construction, associated-type constraint placement, explicit accessor parsing, implicit getter synthesis, semantic accessor types, attribute-class lookup, module specialization flavors, uniform shadow construction, `UnresolvedDecl` usage, and attribute declaration counts.
- Checked all three explicit line citations: `slang-check-modifier.cpp:956`, `slang-check-modifier.cpp:1527`, and `core.meta.slang:4829–4830`.
- Resolved all 94 relative-link occurrences, comprising 27 distinct targets, in the working tree; checked grammar anchors and manifest membership for all 10 referenced generated pages. Dependency documents were consulted only for cross-reference checks.
- Checked required sections, table columns, front-matter keys and formats, Mermaid syntax, and the 39,757-byte document against its 98,304-byte cap.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Nodes`, especially `ParamDecl`, `FuncDecl`, and accessor rows | Numerous Key fields cells contain only `(inherits)`, omitting the required named, typed fields. For example, readers cannot discover a function’s return type or body from its row. | `docs/generated/design/_meta/prompts/_common.md`, AST reference contract, requires 1–4 fields in `name: Type` form. `source/slang/slang-ast-decl.h:668` declares `returnType: TypeExp`; line 700 declares `body: Stmt*`; `VarDeclBase` supplies parameter `type` and `initExpr`. | Replace `(inherits)` with the most informative inherited fields, identifying them as inherited where useful. Include `returnType: TypeExp` and `body: Stmt*` for `FuncDecl`, and `type: TypeExp` for parameter rows. Reserve “no additional state” for explaining the absence of new fields, rather than replacing meaningful inherited shape. |
| F-002 | minor | Lines 280–285 | The claim that `slang-check-modifier.cpp` “creates from `__attributeTarget(...)` (line 956)” identifies the wrong construction path. That location synthesizes a modifier for a user-defined attribute struct. | `source/slang/slang-check-modifier.cpp:946–960` uses `findOrSynthesizeAttributeDeclFromUserDefinedAttributeStruct`. `source/slang/slang-parser.cpp:11055–11066` implements `parseAttributeTargetModifier`, constructing the modifier at line 11063; line 11243 registers `__attributeTarget`. | Attribute construction from `__attributeTarget(...)` to `parseAttributeTargetModifier` in `slang-parser.cpp`. Retain `slang-check-modifier.cpp:1527` for use-site placement validation. |
| F-003 | minor | `## Notable nodes`, particularly lines 253–308 and 393–396 | The callouts violate the required 2–5-sentence prose format. Several are extended explanations, and two contain fenced code blocks despite the explicit prohibition. | `docs/generated/design/_meta/prompts/_common.md`, AST reference contract item 5: “Do not include code blocks.” `docs/generated/design/_meta/prompts/ast-reference-declarations.md` requires 2–5-sentence callouts. | Condense each callout to 2–5 sentences. Render indispensable short syntax examples inline and remove both fenced Slang examples. |
| F-004 | minor | Lines 280–301, 522–531, and 561–567 | The page expands into attribute-validation semantics, target-compilation diagnostics and CLI specialization behavior, and semantic/lowering handling of uniform shadows. These exceed the declaration reference’s explicit scope. | `docs/generated/design/_meta/prompts/ast-reference-declarations.md`, Forbidden content, excludes type-checking semantics and IR-level declaration lowering. The affected passages discuss validation in `source/slang/slang-check-modifier.cpp:1526–1555` and explicitly reference a lowering handler. | Keep the declaration fields, parser producers, and brief synthesis descriptions. Replace the detailed validation, specialization-diagnostic, and lowering discussions with links to the semantic-checking and AST-to-IR pipeline pages. |
