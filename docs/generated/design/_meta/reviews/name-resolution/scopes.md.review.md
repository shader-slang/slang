---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-05T13:46:17+00:00
target_doc: name-resolution/scopes.md
target_doc_source_commit: 52339028a2aa703271533454c6b9528a534bac31
target_doc_watched_paths_digest: fd3d948ba51668efc2bdab940c9ced97bfd743835e43a3a4ae3b6a3a358cd405
source_commit: 05132edd86435f217f95634406f85184e58991f8
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: pass
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

# Review report for name-resolution/scopes.md

## Summary
The scopes page covers the requested source, concepts, rules, edge cases, and see-also links. I found two minor issues: one wording claim overstates parser use of `addSiblingScopeForContainerDecl`, and one lookup header citation falls outside the watched paths.

## Items checked
- Ran `regenerate.py show name-resolution/scopes.md` and checked the manifest entry, prompt, nine resolved watched files, and depends-on docs.
- Verified front matter, required section order, `## Source`, `## Concepts`, `## Rules`, `## Edge cases and failure modes`, and `## See also`.
- Checked all 64 relative links for resolution, including peer name-resolution links, AST reference links, parser/checker source links, pipeline links, and glossary links.
- Verified 45 source line-citation references against source at `52339028a2aa703271533454c6b9528a534bac31`, including `Scope`, `ScopeDecl`, `ScopeStmt`, parser scope helpers, sibling-scope construction, namespace collapse, HLSL `UnscopedForStmt`, and `UsingDecl` capture.
- Spot-checked more than 10 factual claims about scope-bearing AST nodes, parser push/pop behavior, implicit generic/interface scopes, sibling scopes, and lookup walking order.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | minor | `## Source` | The source paragraph says `addSiblingScopeForContainerDecl` is "used by both the parser and the checker", but the recorded source snapshot has no parser call site. The helper is declared/defined and then called from checker/session setup code. | `source/slang/slang-ast-decl.h:1116` declares the helper, `source/slang/slang-check-expr.cpp:315` defines it, and call sites at `source/slang/slang-check-decl.cpp:16027` and `source/slang/slang-session.cpp:2239` are not parser code. | Change the sentence to say the helper is used by semantic-checking and session/module setup code, not by the parser. |
| F-002 | minor | `### Scope walking order during lookup` | The page links to `slang-lookup.h` for lookup entry points, but that header is not in this doc's watched paths, so changes there would not affect freshness for `scopes.md`. | `docs/generated/design/_meta/manifest.yaml:399-410` lists the watched paths and omits `source/slang/slang-lookup.h`; the page cites `source/slang/slang-lookup.h` in this section. | Add `source/slang/slang-lookup.h` to watched paths, or replace the direct source citation with a link to `lookup.md`. |
