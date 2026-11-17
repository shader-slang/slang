---
review_report: true
reviewer_model: gpt-6
reviewed_at: 2026-11-02T21:19:09Z
target_doc: pipeline/04c-layout-ir.md
target_doc_source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_watched_paths_digest: ec9d30d850ec7e35bbb21364629106ad1c885d9bf8f52ed78e6df228739b2df5
source_commit: be3c344414f6541ba6731813b36591a94cd05482
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: pass
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 5
severity_breakdown:
  critical: 2
  major: 0
  minor: 3
  nit: 0
---

# Review report for pipeline/04c-layout-ir.md

## Summary

The document covers the required construction workflow and provides substantially accurate parameter-binding detail. Its most important error is interpreting capability-atom filtering as target filtering: the implementation does not guarantee that non-SPIR-V/non-Metal layout modules lack capability decorations. The construction diagram also misrepresents error handling; three smaller inaccuracies concern IR types and manifest coverage.

## Items checked

- Reviewed against the **current working tree**. Confirmed HEAD matches the supplied report commit and `git diff 2518a942d59b1228fc8cc70a37739baab82b6002 -- source/` is empty.
- Read `_review.md`, `_common.md`, the document-specific prompt, and `regenerate.py show pipeline/04c-layout-ir.md`; checked required sections, both numbered construction tables, and exclusions.
- Checked every body line-number citation against the corresponding source location. All are within the required ±5-line tolerance.
- Spot-checked more than 10 claims, including lazy construction, cache reuse, mutex acquisition, error latching, explicit-binding reservation, mixed-binding completion, default-space selection, attachment-index occupancy, bindless-space allocation, diagnostic IDs, ray-tracing layout rules, entry-point guards, capability forwarding, obfuscation options, and linker consumption.
- Checked all 51 Markdown link occurrences: filesystem destinations exist, local heading anchors resolve, and linked generated pages have manifest entries. Consulted dependency documents only for cross-reference checks.
- Checked required front-matter keys, Markdown structure, Mermaid conventions, and Liquid safety. The document is 36,593 bytes, below its 49,152-byte cap. No files were written; shader examples were assessed from source rather than executed.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
|---|---|---|---|---|---|
| F-001 | critical | Per-entry-point steps, line 491; Caveats and gotchas, lines 596–604 | The claim that `HLSL, WGSL, and CUDA layout-module entry points do not carry capability decorations` confuses the namespace of retained atoms with the selected target. Construction iterates all alternatives in the inferred capability set without a target-selection gate. | `source/slang/slang-parameter-binding.cpp:3373` copies entry-point or specialization requirements into the layout. `source/slang/slang-lower-to-ir.cpp:16564–16575` iterates `set.getAtomSets()` and filters atom values only. | State that only SPIR-V-version and Metal-version atoms are forwarded, regardless of the selected target. Remove the guaranteed absence on other targets and the tool guidance based on that absence. |
| F-002 | critical | Construction flow, lines 405–428 | The diagram always runs binding and then tests `sink->getErrorCount() != 0`. Actual construction binds only when `m_layout` is absent and rejects using the persistent `m_layoutHasErrors` latch. Pre-existing sink errors do not alone trigger that latch; a previously latched failure still rejects with a fresh sink. | `source/slang/slang-parameter-binding.cpp:4853–4868`: binding is guarded by `if (!m_layout)`; the error-count comparison is against `errorCountBefore`; rejection tests `m_layoutHasErrors`. | Add the `!m_layout` branch around binding, show the before/after error-count comparison setting the latch, and gate continuation on `m_layoutHasErrors`. Preserve the cached-layout path. |
| F-003 | minor | Per-global-parameter steps, line 447 | `Materialize stub IRGlobalVar` names the wrong instruction class for global shader parameters. These lower through `lowerGlobalShaderParam` to `IRGlobalParam`. | `source/slang/slang-lower-to-ir.cpp:11576–11578` dispatches global shader parameters to `lowerGlobalShaderParam`; line 11375 calls `builder->createGlobalParam(paramType)`. | Change the step to `Materialize stub IRGlobalParam`, retaining the existing materialization expression. |
| F-004 | minor | Global-scope type layout, lines 462–476 | Saying the module's layout decoration `is an IRParameterGroupTypeLayout rather than the raw struct layout` skips the variable-layout wrapper. The decoration always references an `IRVarLayout`; its type-layout operand contains the parameter-group or struct layout. | `source/slang/slang-lower-to-ir.cpp:16528–16531` calls `lowerVarLayout(context, globalScopeVarLayout, irGlobalScopeTypeLayout)` and decorates the module with that result. | Say that the module's `IRVarLayout` references an `IRParameterGroupTypeLayout` when wrapped, otherwise an `IRStructTypeLayout`. |
| F-005 | minor | Source, lines 65–69 | The sentence saying `slang-type-layout.cpp` is `not` in the watched paths contradicts both its own continuation and the manifest. | `docs/generated/design/_meta/manifest.yaml:241` includes `source/slang/slang-type-layout.cpp`. | Replace the contradictory sentence with a direct statement that this file is included in the watched paths. |