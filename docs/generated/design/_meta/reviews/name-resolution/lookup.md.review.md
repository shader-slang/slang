---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-12T13:39:31+00:00
target_doc: name-resolution/lookup.md
target_doc_source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_watched_paths_digest: a7949e2fefe07d4849f23549562f5e13de05d1d144fa356c7ca6858d3f5d3c21
source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
checklist:
  factual_accuracy: pass
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: pass
  front_matter_validity: pass
finding_count: 1
severity_breakdown:
  critical: 0
  major: 1
  minor: 0
  nit: 0
---

# Review report for name-resolution/lookup.md

## Summary
The lookup page is mostly aligned with the current lookup source and satisfies the main concepts, algorithm, shadowing, and peer-link requirements. I found one completeness issue: the edge-case section omits the prompt-required mask-refinement case where an overloaded lookup result has non-matching items silently filtered away.

## Items checked
- Ran `regenerate.py show name-resolution/lookup.md`; confirmed the per-doc prompt, size cap, three dependency docs, and 11 resolved watched source files.
- Read the target document including front matter, the per-doc prompt, `_common.md`, `scopes.md`, `ast-reference/values.md`, and `glossary.md`.
- Verified the front matter fields and digest shape, required section order, `## Algorithm`, required `## Shadowing rules`, `## Edge cases and failure modes`, and `## See also`.
- Verified source line-citation clusters against `slang-lookup.h`, `slang-lookup.cpp`, `slang-ast-support-types.h`, `slang-ast-base.h`, `slang-ast-decl.h`, `slang-ast-decl.cpp`, `slang-ast-modifier.h`, `slang-check-decl.cpp`, `slang-check-stmt.cpp`, and `slang-diagnostics.lua`.
- Spot-checked more than 10 factual/source-alignment claims: lookup entry points, `LookupMask` bits, `LookupOptions` flags, breadcrumb kinds, scope and sibling walking, transparent-member recursion, pointer auto-deref, `ThisType` handling, block-local hiding, namespace sibling wiring, interface default implementation lookup, ambiguous-reference diagnostics, and `ErrorType` member lookup behavior.
- Resolved peer links used by the page to the name-resolution, ast-reference, pipeline, and glossary documents present under `docs/generated/design/`.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Edge cases and failure modes`, lines 487-491 | The edge-case section covers `LookupResult` with multiple items matching the mask, but it does not cover the required case where an overloaded `LookupResult` has multiple items and only one matches a later `LookupMask`; in that case the non-matching items are filtered silently by `refineLookup`. | `docs/generated/design/_meta/prompts/name-resolution-lookup.md:129-132` requires this case. `source/slang/slang-lookup.cpp:128-143` implements `refineLookup` by iterating `inResult.items`, skipping items that fail `DeclPassesLookupMask`, and adding only passing items to the returned result. | Expand the first edge-case bullet to explicitly describe mask refinement through `refineLookup`, including the single-survivor case and the silent filtering of non-matching items, with a citation to `source/slang/slang-lookup.cpp:128-143`. |

## No-issues notes
- The page correctly documents that `LookupResult` itself does not deduplicate items; `AddToLookupResult` only appends or promotes to the `items` list.
- The named `LookupMask`, `LookupOptions`, and breadcrumb `Kind` values match `slang-ast-support-types.h` at the target source commit.
- The ambiguous-reference diagnostic citation matches the current canonical diagnostic entry in `slang-diagnostics.lua`.
