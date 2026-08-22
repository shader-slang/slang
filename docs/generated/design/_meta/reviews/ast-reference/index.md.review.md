---
review_report: true
reviewer_model: gpt-5.6-sol
reviewed_at: 2026-08-04T12:06:33+00:00
target_doc: ast-reference/index.md
target_doc_source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_watched_paths_digest: 9b051f09c6305e9dacb68adede05e274c6eca01813cd472a3d3931c5146eb5aa
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
checklist:
  factual_accuracy: pass
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: pass
  front_matter_validity: pass
finding_count: 1
severity_breakdown:
  critical: 0
  major: 0
  minor: 0
  nit: 1
---

# Review report for ast-reference/index.md

## Summary

The document is factually aligned with the recorded source, and its links, front matter, family counts, and hierarchy all check out. The only finding is a small prompt-contract deviation: `## How to navigate` contains seven sentences instead of the required three to four.

## Items checked

- Read `_common.md`, the per-document prompt, all seven current dependency pages, and every path resolved by `regenerate.py show ast-reference/index.md`; source claims were checked at commit `53b76e6d3009b8e6434d41573524c7ce5c499d23`.
- Verified all 11 Mermaid hierarchy edges against `source/slang/slang-ast-base.h:133-825`, including the `SourceLoc`, modifier-storage, `Type : Val`, and `DeclRefBase : Val` claims.
- Counted concrete `FIDDLE` declarations in all six owning headers and confirmed the rounded values `~60`, `~95`, `~30`, `~120`, `~60`, and `~265`.
- Spot-checked more than 10 further claims, including `SyntaxNode` being the sole direct `SyntaxNodeBase` subclass, `DeclGroup : DeclBase`, deferred `UnparsedStmt` creation, work-graph `__intrinsic_type` declarations, and `[Differentiable]` mapping to `BackwardDifferentiableAttribute`.
- Resolved all 38 relative links, checked their anchors where present, and confirmed all 12 linked generated peers are manifest entries. The body contains zero line-number citations, so there were none to re-derive.
- Checked required headings and order, table columns, intro audience statement, Mermaid syntax, file-name and identifier sweeps, the size cap, and all mandatory front-matter fields.

## Findings

| ID    | Severity | Location                            | Description                                                                                                                                                                                                         | Evidence                                                                                                                                          | Recommendation                                                                                                                                                                                  |
| ----- | -------- | ----------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| F-001 | nit      | `## How to navigate`, lines 186-211 | The navigation guidance spans seven sentences, while the per-document prompt requires a three-to-four-sentence section. The content is accurate, but the section is longer than its specified navigation-hub shape. | `docs/generated/design/_meta/prompts/ast-reference-index.md:51-54` requires “3-4 sentence guidance”; the target section contains seven sentences. | Condense the section to three or four sentences while retaining the `base.md` starting point, family-page guidance, abstract-node location, and literal interpretation of the `Grammar` column. |
