---
review_report: true
reviewer_model: gpt-5.6-sol
reviewed_at: 2026-08-04T12:07:26+00:00
target_doc: name-resolution/lookup.md
target_doc_source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_watched_paths_digest: 908d68e75b69302955968bbdfcb859a23def6d820df3e4565050f206b89519c1
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: pass
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: partial
finding_count: 4
severity_breakdown:
  critical: 0
  major: 1
  minor: 3
  nit: 0
---

# Review report for name-resolution/lookup.md

## Summary
The page is comprehensive, all line-number citations are accurate at the recorded source commit, and its required sections and links are present. Four findings remain. Most importantly, the front-matter digest no longer matches the manifest's expanded watched set, so the document metadata does not describe the inputs currently assigned to the page.

## Items checked
- Ran `regenerate.py show name-resolution/lookup.md`; confirmed the per-doc prompt, 64 KiB size cap, three dependency docs, and 15 resolved watched source files.
- Read the target document including front matter, the per-doc prompt, `_common.md`, `scopes.md`, `ast-reference/values.md`, and `glossary.md`.
- Confirmed `HEAD` equals the target's recorded source commit and verified all 61 line-number citation expressions across the 15 watched source files.
- Spot-checked more than 10 factual claims, including entry-point signatures, mask and option bits, request/result invariants, breadcrumb kinds and order, scope traversal, facet filtering, transparent-member recursion, pointer auto-deref, block-local hiding, lookup accelerators, parser keyword lookup, and ambiguity diagnostics.
- Ran the document lint, confirmed the 40,940-byte page is under its size cap, and recomputed the watched-path digest.
- Resolved all relative links and checked the cited peer-document anchors and manifest membership.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | Front matter, line 6 | The recorded `watched_paths_digest` is stale after the manifest's watched set was expanded. The page records `908d68e75b69302955968bbdfcb859a23def6d820df3e4565050f206b89519c1`, while `regenerate.py digest name-resolution/lookup.md` now returns `4824386ae8ea099d1c1c5fce3ad1de39a629e6fdc82526541b91ba7a41ee5cea`; the mandatory metadata therefore does not identify the current input set. | `docs/generated/design/name-resolution/lookup.md:6` contains the old digest. `docs/generated/design/_meta/manifest.yaml:481-498` defines the current 15-file watched set. | Regenerate or refresh the page against the current manifest so line 6 records `4824386ae8ea099d1c1c5fce3ad1de39a629e6fdc82526541b91ba7a41ee5cea`. |
| F-002 | minor | `## Source`, lines 55-68 | The page says four cited files “live outside this page's watched paths,” but all four are now watched. This leaves the source inventory inconsistent with the manifest and incorrectly tells maintainers that these claims are not tracked for drift. | `docs/generated/design/_meta/manifest.yaml:494-497` includes `slang-check-impl.h`, `slang-check-expr.cpp`, `slang-check-overload.cpp`, and `slang-parser.cpp`. | Remove the assertion that these files are outside the watched paths and present them as part of the normal source inventory. |
| F-003 | minor | `### Unqualified lookup`, lines 255-264 | The `AggTypeDeclBase` dispatch is described as “i.e.” lookup inside a `struct`, `interface`, `enum`, or `extension`, but that list is not exhaustive: `ClassDecl` and other `AggTypeDecl` subclasses also take this branch. | `source/slang/slang-ast-decl.h:360-386` makes both `ExtensionDecl` and `AggTypeDecl` derive from `AggTypeDeclBase`; `source/slang/slang-ast-decl.h:428-431` declares `ClassDecl : AggTypeDecl`. | Replace “i.e.” with “for example,” or explicitly include classes and state that all `AggTypeDeclBase` subclasses take the branch. |
| F-004 | minor | `#### Deduplication`, lines 599-603 | The sentence says the “same `DeclRef`” can appear both as an interface requirement and as the concrete member satisfying it. Those are distinct declaration candidates; the comparator explicitly distinguishes the interface requirement from the concrete function and prefers the latter. The no-deduplication conclusion is correct, but this example conflates duplicate paths to one decl-ref with competing decl-refs for different declarations. | `source/slang/slang-check-overload.cpp:1944-1958` describes lookup returning “both an interface requirement and the concrete function” and tests the two declarations separately with `isInterfaceRequirement`. | Split the examples: retain direct-versus-transparent lookup as the same-decl-ref case, and describe interface requirement versus concrete implementation as distinct candidates resolved later by `CompareLookupResultItems`. |

## No-issues notes
- The `cbuffer` breadcrumb order is correctly documented as `Member -> Deref`, matching construction and checker consumption.
- The named `LookupMask`, `LookupOptions`, and breadcrumb `Kind` values match `slang-ast-support-types.h` at the target source commit.
- The required algorithm, shadowing, edge-case, and see-also sections are present in the prescribed order.
