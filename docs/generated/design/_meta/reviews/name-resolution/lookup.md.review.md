---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-05T14:19:45+00:00
target_doc: name-resolution/lookup.md
target_doc_source_commit: 52339028a2aa703271533454c6b9528a534bac31
target_doc_watched_paths_digest: f2b871e4496ed32a327e98986317cc4eb48691d204aad2e863ea2eebf51fc801
source_commit: fb192be9f5b3b58555e034599e072158e5c48dfd
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
  major: 1
  minor: 1
  nit: 0
---

# Review report for name-resolution/lookup.md

## Summary
The lookup page covers the required concepts, algorithm sections, shadowing rules, and failure modes. I found two issues: one watched-path alignment problem for the namespace-shadowing citation, and one source contradiction where the page says visibility filtering drops duplicate lookup paths.

## Items checked
- Ran `regenerate.py show name-resolution/lookup.md` and checked the manifest entry, prompt, 11 resolved watched files, and depends-on docs.
- Verified the target front matter fields, required section order, and the lookup prompt's required `## Shadowing rules` section.
- Checked all 75 relative links for resolution and spot-checked peer links against `scopes.md`, `visibility.md`, `overload-resolution.md`, `ast-reference/values.md`, and `glossary.md`.
- Verified 55 source line-citation references against source at `52339028a2aa703271533454c6b9528a534bac31`, including `LookupMask`, `LookupOptions`, breadcrumbs, `_lookUpInScopes`, `_lookUpDirectAndTransparentMembers`, `AddToLookupResult`, and block-local shadowing.
- Spot-checked more than 10 factual claims about scope walking, sibling scopes, transparent members, interface default implementations, and lookup failure behavior.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | minor | `## Shadowing rules`, `### Module and namespace` | The page cites `slang-parser.cpp` for namespace collapse, but `slang-parser.cpp` is not one of this doc's resolved watched files. Because the claim depends on an unwatched source file, changes to `parseNamespaceDecl` would not affect this page's watched-path digest. | `docs/generated/design/_meta/manifest.yaml:417-430` lists the watched paths and omits `source/slang/slang-parser.cpp`; the cited implementation is `source/slang/slang-parser.cpp:4086`. | Add `source/slang/slang-parser.cpp` to `name-resolution/lookup.md` watched paths, or replace this source citation with a link to `scopes.md` where parser scope construction is already in scope. |
| F-002 | major | `## Shadowing rules`, `#### Deduplication: there isn't any at the LookupResult level` | The page says visibility filtering in `TryCheckOverloadCandidateVisibility` drops duplicates that point at identical visible declarations. The cited function only checks whether the candidate is visible from the source scope and returns false for invisible candidates; it does not compare declarations or deduplicate lookup paths. | `source/slang/slang-check-overload.cpp:265` defines `TryCheckOverloadCandidateVisibility`; `source/slang/slang-check-overload.cpp:275` calls `isDeclVisibleFromScope`, and `source/slang/slang-check-overload.cpp:283` returns false only for an invisible candidate. | Remove the visibility-filtering duplicate claim, or replace it with a source-backed description of where duplicate or tied candidates are ranked after lookup. |
