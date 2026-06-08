---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-05T14:56:42+00:00
target_doc: name-resolution/scopes.md
target_doc_source_commit: 52339028a2aa703271533454c6b9528a534bac31
target_doc_watched_paths_digest: fd3d948ba51668efc2bdab940c9ced97bfd743835e43a3a4ae3b6a3a358cd405
source_commit: fb192be9f5b3b58555e034599e072158e5c48dfd
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 3
severity_breakdown:
  critical: 0
  major: 1
  minor: 2
  nit: 0
---

# Review report for name-resolution/scopes.md

## Summary
The scopes page has the required top-level structure, valid front matter, and resolving links. The main issue is that it says `WhileStmt` and `DoWhileStmt` do not own scopes even though both inherit through `LoopStmt` and `BreakableStmt` from `ScopeStmt`, so the page omits required scope-bearing statement classes.

## Items checked
- Ran `regenerate.py show name-resolution/scopes.md` and checked the manifest entry, prompt, nine resolved watched files, and depends-on docs.
- Verified front matter, required section order, `## Source`, `## Concepts`, `## Rules`, `## Edge cases and failure modes`, and `## See also`.
- Checked all 64 relative links for resolution, including peer name-resolution links, AST reference links, parser/checker source links, pipeline links, and glossary links.
- Verified 45 source line-citation references against source at `52339028a2aa703271533454c6b9528a534bac31`, including `Scope`, `ScopeDecl`, `ScopeStmt`, parser scope helpers, sibling-scope construction, namespace collapse, HLSL `UnscopedForStmt`, and `UsingDecl` capture.
- Spot-checked more than 10 factual claims about scope-bearing AST nodes, parser push/pop behavior, implicit generic/interface scopes, sibling scopes, and lookup walking order.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `### Scope-bearing AST nodes` | The page says `WhileStmt` and `DoWhileStmt` do not own a scope and omits them from the scope-bearing node table, but both are concrete `LoopStmt` subclasses and `LoopStmt` inherits from `BreakableStmt`, which inherits from `ScopeStmt`. | `source/slang/slang-ast-stmt.h:100` declares `BreakableStmt : public ScopeStmt`, `source/slang/slang-ast-stmt.h:209` declares `LoopStmt : public BreakableStmt`, and `source/slang/slang-ast-stmt.h:234` plus `source/slang/slang-ast-stmt.h:242` declare `WhileStmt` and `DoWhileStmt` as `LoopStmt` subclasses. | Add `WhileStmt` and `DoWhileStmt` to the scope-bearing statement coverage and remove the claim that they do not own a scope. |
| F-002 | minor | `## Source` | The source paragraph says `addSiblingScopeForContainerDecl` is "used by both the parser and the checker", but the recorded source snapshot has no parser call site. The helper is declared and defined, then called from checker and session setup code. | `source/slang/slang-ast-decl.h:1116` declares the helper, `source/slang/slang-check-expr.cpp:315` defines it, and call sites at `source/slang/slang-check-decl.cpp:16027` and `source/slang/slang-session.cpp:2239` are not parser code. | Change the sentence to say the helper is used by semantic-checking and session or module setup code, not by the parser. |
| F-003 | minor | `### Scope walking order during lookup` | The page links to `slang-lookup.h` for lookup entry points, but that header is not in this doc's watched paths, so changes there would not affect freshness for `scopes.md`. | `regenerate.py show name-resolution/scopes.md` lists watched paths and omits `source/slang/slang-lookup.h`; the page cites `source/slang/slang-lookup.h` in this section. | Add `source/slang/slang-lookup.h` to watched paths, or replace the direct source citation with a link to `lookup.md`. |
