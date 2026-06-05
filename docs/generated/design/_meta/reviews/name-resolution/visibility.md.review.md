---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-05T13:46:17+00:00
target_doc: name-resolution/visibility.md
target_doc_source_commit: 52339028a2aa703271533454c6b9528a534bac31
target_doc_watched_paths_digest: 7f835b0f4fb5f3c95c0f2466c182e4c6530306b32626d162568da304471b39ae
source_commit: 05132edd86435f217f95634406f85184e58991f8
checklist:
  factual_accuracy: pass
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

# Review report for name-resolution/visibility.md

## Summary
The visibility page matches the required structure and its main claims about modifiers, defaults, filtering, diagnostics, and edge cases are source-backed. I found one watched-path issue: the page cites the overload visibility step in `slang-check-overload.cpp`, but that file is not watched for this doc.

## Items checked
- Ran `regenerate.py show name-resolution/visibility.md` and checked the manifest entry, prompt, nine resolved watched files, and depends-on docs.
- Verified front matter, required section order, `## Source`, `## Concepts`, `## Rules`, `## Edge cases and failure modes`, and `## See also`.
- Checked all 35 relative links for resolution, including source links, peer name-resolution links, AST reference links, and glossary links.
- Verified 23 source line-citation references against source at `52339028a2aa703271533454c6b9528a534bac31`, including `VisibilityModifier`, `DeclVisibility`, `ModuleDecl::defaultVisibility`, `getDeclVisibility`, `filterLookupResultByVisibilityAndDiagnose`, `isDeclVisibleFromScope`, `checkVisibility`, and `IgnoreForLookupModifier`.
- Spot-checked more than 10 factual claims about per-keyword semantics, legacy versus modern defaults, private extension access, effective type visibility, diagnostics, synthesized visibility propagation, and language-version interaction.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | minor | `## Source` and `### Where visibility is filtered` | The page cites `slang-check-overload.cpp` for `TryCheckOverloadCandidateVisibility`, but `slang-check-overload.cpp` is not in this doc's resolved watched files. Changes to the overload-time visibility hook would not affect this page's watched-path digest. | `docs/generated/design/_meta/manifest.yaml:437-448` lists the watched paths and omits `source/slang/slang-check-overload.cpp`; the cited hook is `source/slang/slang-check-overload.cpp:265`. | Add `source/slang/slang-check-overload.cpp` to the watched paths, or move the overload-specific citation to `overload-resolution.md` and keep this page to watched visibility sources. |
