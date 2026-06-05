---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-05T14:19:45+00:00
target_doc: name-resolution/index.md
target_doc_source_commit: 52339028a2aa703271533454c6b9528a534bac31
target_doc_watched_paths_digest: 74f38318a36443c037d6981bf35771b5568a81348341a8264dae6148131877f4
source_commit: fb192be9f5b3b58555e034599e072158e5c48dfd
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: pass
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 1
severity_breakdown:
  critical: 0
  major: 0
  minor: 1
  nit: 0
---

# Review report for name-resolution/index.md

## Summary
The navigation page matches the requested structure and all links resolve. I found one factual alignment issue: the pipeline-context section says breadcrumb chains are turned into IR access patterns during AST-to-IR lowering, but the source shows breadcrumbs are expanded earlier by semantic checking.

## Items checked
- Ran `regenerate.py show name-resolution/index.md` and checked its manifest entry, per-doc prompt, watched files, and four depends-on peer docs.
- Read the target front matter and verified required keys, source commit, watched-path digest shape, title, intro, `## Pages`, flow diagram, pipeline context, and glossary section.
- Resolved all 26 relative links in the body, including peer name-resolution pages, pipeline pages, AST reference pages, syntax reference, glossary, and cross-cutting references.
- Spot-checked more than 10 factual claims against the peer docs, `slang-lookup.h`, `slang-ast-support-types.h`, and source snippets for breadcrumb consumption.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | minor | `## Where this fits in the pipeline`, lines 83-87 | The page says resolved `DeclRef` flows into AST-to-IR lowering "where breadcrumb chains are turned into concrete IR access patterns." Breadcrumb expansion happens in semantic checking, not in the lowerer. | `source/slang/slang-check-expr.cpp:873` defines `ConstructLookupResultExpr`; `source/slang/slang-check-expr.cpp:891` says collected breadcrumbs are "additional segments of the lookup path" expanded there. | Replace the downstream sentence with a checker-side statement, for example that semantic checking expands breadcrumbs into concrete AST expressions before those expressions are later lowered. |
