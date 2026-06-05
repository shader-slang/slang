---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-05T13:46:17+00:00
target_doc: name-resolution/overload-resolution.md
target_doc_source_commit: 52339028a2aa703271533454c6b9528a534bac31
target_doc_watched_paths_digest: 4b84c7f99f5cc6586a2852af0165acc8c35f69765bd0593a6595e5852f1750ec
source_commit: 05132edd86435f217f95634406f85184e58991f8
checklist:
  factual_accuracy: pass
  cross_references: pass
  completeness: partial
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

# Review report for name-resolution/overload-resolution.md

## Summary
The overload-resolution page is mostly source-aligned for candidate filtering, conversion costs, comparator ordering, partial generics, and operator caching. The main issue is that the partial-generic section steps into IR-lowering behavior, which the prompt forbids for this page and which depends on an unwatched source file.

## Items checked
- Ran `regenerate.py show name-resolution/overload-resolution.md` and checked the manifest entry, prompt, five resolved watched files, and depends-on docs.
- Verified front matter, required section order, `## Conversion costs`, `## Partial generic application`, `## Operator overloading`, `## Edge cases and failure modes`, and `## See also`.
- Checked all 56 relative links for resolution, including peer name-resolution, AST reference, pipeline, and glossary links.
- Verified 44 source line-citation references against source at `52339028a2aa703271533454c6b9528a534bac31`, including `OverloadCandidate`, `OverloadResolveContext`, `TryCheckOverloadCandidate`, conversion-cost constants, `CompareOverloadCandidates`, partial-generic flag sites, and operator-cache sites.
- Spot-checked more than 10 factual claims about candidate statuses, `JustTrying` / `ForReal`, implicit conversion costs, comparator tie-breaking, partial generic application, and ambiguous-overload reporting.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Partial generic application` | The paragraph about unresolved `PartiallyAppliedGenericExpr` reaching lowering is IR-lowering behavior, which the overload-resolution prompt explicitly excludes. It also cites `slang-lower-to-ir.cpp`, which is not in this doc's watched paths. | `docs/generated/design/_meta/prompts/name-resolution-overload-resolution.md:107-114` forbids IR-level lowering of resolved overloads; `docs/generated/design/_meta/manifest.yaml:455-462` omits `source/slang/slang-lower-to-ir.cpp`; the cited lowerer hook is `source/slang/slang-lower-to-ir.cpp:5757`. | Remove the lowerer/ICE sentence from this page, or replace it with a resolver-side invariant that stays within the watched overload-resolution sources. |
| F-002 | minor | `## Algorithm`, arity step | The arity step cites `slang-diagnostics.lua` for diagnostics, but that file is not watched for this page. The diagnostic citation can drift without changing this page's watched-path digest. | `docs/generated/design/_meta/manifest.yaml:455-462` lists the watched paths and omits `source/slang/slang-diagnostics.lua`; the doc links to `source/slang/slang-diagnostics.lua` from the arity step. | Add the diagnostics source used for these identifiers to the watched paths, or avoid source-linking diagnostics from this page. |
