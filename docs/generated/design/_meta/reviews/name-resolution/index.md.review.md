---
review_report: true
reviewer_model: gpt-6
reviewed_at: 2026-11-03T06:51:16Z
target_doc: name-resolution/index.md
target_doc_source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_watched_paths_digest: 45713374c06eca1abe41e503639aad74a658c785ee7461dd1ee68ec7bba0be9d
source_commit: c63c2fbd5115a98b2b2469f43618671653d40bf1
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
  major: 0
  minor: 2
  nit: 0
---

# Review report for name-resolution/index.md

## Summary

The document provides a useful navigation overview, and its detailed behavioral summaries are substantially supported by the source. Two small corrections remain: the introduction overgeneralizes builtin-operator handling, and the diagram assigns some transitions to the wrong topic pages.

## Items checked

- Read `_review.md`, `_common.md`, and the document-specific prompt; ran `python3 docs/generated/design/_meta/regenerate.py show name-resolution/index.md`.
- Verified against the **current working tree**, which the supplied review context identifies as identical to the document’s `source_commit` for all of `source/`.
- Spot-checked 14 claims: parser scope construction; deferred body parsing; parent/sibling traversal; lookup masks; duplicate result retention; facet-origin matching; lookup refinement; equal-ranked item retention; lookup visibility filtering; candidate visibility checks; builtin rewrite ordering; GLSL operator exceptions; scope-wiring/import check states; and breadcrumb expansion into AST expressions.
- Checked relevant implementations in `slang-parser.cpp`, `slang-lookup.cpp`, `slang-check-expr.cpp`, `slang-check-overload.cpp`, `slang-check-decl.cpp`, and `slang-check-inheritance.cpp`, plus the lookup and AST support headers.
- Found **zero line-number citations** in the target document’s body.
- Resolved all **53 Markdown links**, comprising **31 distinct destinations**; checked fragment anchors and generated-page manifest membership. Consulted dependency documents only for cross-reference checks.
- Checked required headings, four page-table rows and columns, diagram transition labels, glossary coverage, and introductory audience statement.
- Confirmed all six required document front-matter keys are present and well-formed. Checked the 10,121-byte document against its 32,768-byte cap, Mermaid conventions, and absence of raw Liquid openers.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | minor | Introduction, lines 41–44 | `builtin operators skip overload resolution entirely` is too broad. The later explanation correctly limits this to successful fast-path rewrites, but the introductory statement excludes valid fallback cases. | `source/slang/slang-check-expr.cpp:4991–5005` returns null for GLSL matrix operators and vector equality so normal overload resolution applies. `source/slang/slang-check-expr.cpp:5232–5247` also declines the short-circuit rewrite for non-scalar inputs. | Change the introductory clause to “builtin operators handled by the fast path skip overload resolution.” |
| F-002 | minor | `## Flow diagram`, lines 72–75 | Several transition labels do not identify the page owning the operation: the scope walk producing raw results is labeled `scopes.md`, while the visibility-filter transition is labeled `lookup.md`. This conflicts with the prompt’s explicit transition-ownership requirement and the introduction’s topic split. | `docs/generated/design/_meta/prompts/name-resolution-index.md`, required structure item 4, requires each transition to name its owning page. `source/slang/slang-lookup.cpp:815–836` implements the scope walk; `source/slang/slang-check-expr.cpp:1324–1330` implements visibility filtering. The corresponding destinations are `lookup.md#unqualified-lookup` and `visibility.md#where-visibility-is-filtered`. | Label `scopeWalk --> raw` with `lookup.md`, `raw --> visFilter` with `visibility.md`, and `visFilter --> narrow` with `lookup.md and overload-resolution.md`. |
