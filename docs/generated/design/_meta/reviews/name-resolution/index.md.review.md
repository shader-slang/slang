---
review_report: true
reviewer_model: gpt-5.6-sol
reviewed_at: 2026-08-04T12:05:49+00:00
target_doc: name-resolution/index.md
target_doc_source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_watched_paths_digest: 0f6cf3c4efb7c81823f964a380c40f6af78ea36f054829ad5fea14c87956c70a
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
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
  major: 0
  minor: 3
  nit: 0
---

# Review report for name-resolution/index.md

## Summary

The page is a useful and mostly accurate navigation hub, and every relative link resolves. Three minor findings remain. Most importantly, it incorrectly places imported-module sibling-scope wiring in `DeclCheckState::ScopesWired`; imports are wired by the header visitor at `SignatureChecked`.

## Items checked

- Ran `regenerate.py show name-resolution/index.md`; reviewed the common and per-doc contracts, all seven resolved watched files, and all four dependency pages.
- Spot-checked 15 factual claims at commit `53b76e6d3009b8e6434d41573524c7ce5c499d23`, including scope construction and traversal, `LookupResult` storage, facet deduplication, visibility filtering, lookup narrowing, breadcrumb expansion, parser-time lookup, sibling wiring, `ScopesWired` ordering, builtin-operator handling, and AST-to-IR handoff.
- Verified every source symbol named in the target, including `refineLookup`, `resolveOverloadedLookup`, `CompareLookupResultItems`, `filterLookupResultByVisibilityAndDiagnose`, `isDeclVisibleFromScope`, `TryCheckOverloadCandidateVisibility`, `BuiltinOperatorExpr`, and `BuiltinOperationKind`.
- Verified all 53 relative-link occurrences and every linked file at the recorded commit; checked all peer-page anchors and all glossary terms referenced by the target.
- The target body contains no numeric line-number citations; therefore there were zero line-number citations to re-derive.
- Verified all mandatory front-matter keys and the 64-digit hexadecimal watched-path digest.

## Findings

| ID    | Severity | Location                                       | Description                                                                                                                                                                                                                                                                                | Evidence                                                                                                                                                                                                                                                                                           | Recommendation                                                                                                                                                                                     |
| ----- | -------- | ---------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| F-001 | minor    | `### Where the boundaries blur`, lines 107-112 | The sentence says namespaces, `using` declarations, and imported modules all acquire `nextSibling` links when a declaration reaches `DeclCheckState::ScopesWired`. Imported modules are instead added by `SemanticsDeclHeaderVisitor::visitImportDecl`, which runs at `SignatureChecked`.  | `source/slang/slang-check-decl.cpp:17083-17115` performs `importModuleIntoScope` in `SemanticsDeclHeaderVisitor::visitImportDecl`; `source/slang/slang-check-decl.cpp:17864-17869` maps `ScopesWired` to `SemanticsDeclScopeWiringVisitor` and `SignatureChecked` to `SemanticsDeclHeaderVisitor`. | Split imported modules from the `ScopesWired` claim: retain namespaces and `using` declarations there, then state that imports splice module scopes during `ImportDecl` header/signature checking. |
| F-002 | minor    | `### Where the boundaries blur`, lines 87-96   | The wording `What collapses the rest` overstates what `refineLookup` and `resolveOverloadedLookup` do. `resolveOverloadedLookup` removes only strictly worse items; when `CompareLookupResultItems` returns equality, both items remain, so the chain is not a general deduplication step. | `source/slang/slang-check-expr.cpp:1426-1453` removes or rejects an item only for nonzero comparator results and adds the item when the result is zero; `source/slang/slang-lookup.cpp:95-113` also appends lookup items without equality checks.                                                  | Replace “collapses the rest” with language saying the chain prunes mask-mismatched and strictly worse paths while preserving equal or incomparable items for later overload or ambiguity handling. |
| F-003 | minor    | Opening paragraph, lines 12-22                 | The first body paragraph explains what the subtree covers but does not say who the intended reader is; the audience appears only in the second paragraph. This misses the universal first-paragraph contract even though the audience information itself is present.                       | `docs/generated/design/_meta/prompts/_common.md:65-66` requires the first paragraph to state both what the document covers and who its intended reader is.                                                                                                                                         | Add a short intended-reader clause to the first paragraph, while keeping the fuller audience explanation in the second paragraph if desired.                                                       |
