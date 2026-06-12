---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-05T14:19:45+00:00
target_doc: name-resolution/overload-resolution.md
target_doc_source_commit: 52339028a2aa703271533454c6b9528a534bac31
target_doc_watched_paths_digest: 4b84c7f99f5cc6586a2852af0165acc8c35f69765bd0593a6595e5852f1750ec
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
  major: 2
  minor: 1
  nit: 0
---

# Review report for name-resolution/overload-resolution.md

## Summary
The overload-resolution page is mostly source-aligned for candidate filtering, conversion costs, comparator ordering, partial generics, and operator caching. I found three issues: the partial-generic section steps into forbidden IR-lowering behavior, a diagnostic citation uses an unwatched file, and the no-applicable edge-case prose overgeneralizes the resolver's failure path.

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
| F-003 | major | `## Edge cases and failure modes`, no-candidate bullet | The page says that when no candidate is `Applicable`, the resolver re-runs the single highest-scoring candidate in `ForReal` mode so the user sees a specific diagnostic. The source has a separate path for multiple equally-good non-applicable candidates that emits `NoApplicableOverloadForNameWithArgs` or `NoApplicableWithArgs` directly, without calling `CompleteOverloadCandidate`. | `source/slang/slang-check-overload.cpp:3181` enters the multiple-best-candidates branch; `source/slang/slang-check-overload.cpp:3216` handles non-applicable tied candidates; `source/slang/slang-check-overload.cpp:3223` and `source/slang/slang-check-overload.cpp:3230` emit the no-applicable diagnostics. The `CompleteOverloadCandidate` call is only in the single-best branch at `source/slang/slang-check-overload.cpp:3339` and `source/slang/slang-check-overload.cpp:3397`. | Qualify the edge case to distinguish tied non-applicable candidates from a single best non-applicable candidate, and name the direct no-applicable diagnostics for the tied path. |
