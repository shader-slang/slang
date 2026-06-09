---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-05T14:58:02+00:00
target_doc: ast-reference/modifiers.md
target_doc_source_commit: 52339028a2aa703271533454c6b9528a534bac31
target_doc_watched_paths_digest: 27e8704d639742ca3f3a7523ba881ce0c549f37a2ecf72d2fba11ec9fd4e0064
source_commit: fb192be9f5b3b58555e034599e072158e5c48dfd
checklist:
  factual_accuracy: pass
  cross_references: partial
  completeness: partial
  style_consistency: pass
  source_alignment: pass
  front_matter_validity: pass
finding_count: 1
severity_breakdown:
  critical: 0
  major: 1
  minor: 0
  nit: 0
---

# Review report for ast-reference/modifiers.md

## Summary
The modifier page is largely aligned with the watched AST header: sampled concrete `FIDDLE()` rows, field summaries, and hierarchy relationships match the source. One issue remains: many grammar-column links point to an anchor that does not exist in the dependency grammar page, so readers cannot jump to the referenced modifier or attribute grammar.

## Items checked
- Ran `python3 docs/generated/design/_meta/regenerate.py show ast-reference/modifiers.md` and used the listed watched files at `52339028a2aa703271533454c6b9528a534bac31`.
- Compared the `## Nodes` table against concrete and abstract `FIDDLE()` declarations in `source/slang/slang-ast-modifier.h`, including the modifier, semantic, layout, unchecked-layout, attribute, capability, target, and differentiability groups.
- Read the per-document prompt, `_common.md`, and dependency docs `ast-reference/base.md` and `syntax-reference/grammar.md`.
- Resolved all relative markdown links and heading anchors; only the repeated grammar anchor described below failed.
- Spot-checked at least 12 source-alignment claims, including `AttributeBase` fields, HLSL semantic classes, `BorrowModifier`, matrix-layout inheritance, `GLSLLayoutModifierGroupBegin`, `NumThreadsAttribute`, `DifferentiableAttribute`, `TargetIntrinsicModifier`, `RequireCapabilityAttribute`, parser modifier registration, core-module syntax declarations, and notable-node statements.
- Checked required front matter keys and verified that the target digest is a 64-character hex value.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Nodes` grammar column and `## See also` | The document repeatedly links grammar references to `../syntax-reference/grammar.md#modifiers-and-attributes`, but that anchor is absent from the dependency page and the modifier prompt asks for the separate modifier and attribute anchors. | `docs/generated/design/syntax-reference/grammar.md:220` has `## Modifiers` and `docs/generated/design/syntax-reference/grammar.md:252` has `## Attributes and decorations`; `docs/generated/design/_meta/prompts/ast-reference-modifiers.md:21-24` names `#modifiers` or `#attributes` as the expected targets. | Replace modifier grammar links with `../syntax-reference/grammar.md#modifiers` and attribute grammar links with `../syntax-reference/grammar.md#attributes-and-decorations`, or add a matching compatibility anchor in the grammar page during remediation. |
