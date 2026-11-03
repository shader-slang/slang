---
review_report: true
reviewer_model: gpt-6
reviewed_at: 2026-11-03T06:44:59Z
target_doc: name-resolution/lookup.md
target_doc_source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_watched_paths_digest: d3dc6d6755c9a9d2b2dff12422b5e7bdc0cd26f90429fc52ddd7d6d51ab372b0
source_commit: c63c2fbd5115a98b2b2469f43618671653d40bf1
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: pass
  style_consistency: partial
  source_alignment: partial
  front_matter_validity: pass
finding_count: 7
severity_breakdown:
  critical: 1
  major: 0
  minor: 6
  nit: 0
---

# Review report for name-resolution/lookup.md

## Summary

The document covers the required topics and largely matches the implementation, but needs several corrections. Most importantly, it describes `IgnoreInheritance` as excluding all inherited facets, although interface facets bypass that option’s check. The remaining findings concern completion behavior, a nonexistent field, diagnostic behavior, scope termination, and contract boundaries.

## Items checked

- Reviewed against the **current working tree**, which the operator states equals the document’s `source_commit` for all of `source/`.
- Read `_review.md`, `_common.md`, and the document-specific prompt; ran `regenerate.py show name-resolution/lookup.md`.
- Checked all body line-number citations, including 192 explicit `line`/`lines` expressions and their continued ranges. No substantial line-number drift found.
- Spot-checked more than 20 behavioral claims, including masks, options, result accumulation, refinement, local hiding, scope traversal, facet filtering, pointer dereference, existential opening, transparent-member production, breadcrumb expansion, namespace wiring, and diagnostics.
- Resolved all 151 Markdown links, covering 35 distinct targets; checked target-file existence at the recorded commit, peer manifest membership, and linked heading anchors. Dependency documents were consulted only for cross-reference checks.
- Verified required sections, front-matter keys and formats, Mermaid conventions, and Liquid safety. The document is 55,915 bytes, below its 65,536-byte cap.
- No files were written.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | `## Concepts`, lines 184–185; `### Member lookup`, lines 479–483 | `IgnoreInheritance` is described as returning “only direct members” and skipping non-`Self` facets. Interface facets bypass this check, so the stated filtering algorithm is incorrect. | `source/slang/slang-lookup.cpp:420–429`: the interface branch tests `IgnoreBaseInterfaces`; the `IgnoreInheritance` test is an **`else if`**. | State that `IgnoreInheritance` filters non-interface inherited facets, subject to the extension exception. Explain that interface facets are controlled separately by `IgnoreBaseInterfaces`. |
| F-002 | minor | `## Concepts`, lines 84–89; `### Unqualified lookup`, lines 376–377 | The direct-member entry point does not “always” run with `LookupOptions::None`: initialization can add `Completion`. Also, the claim that each candidate is compared against `declToExclude` does not hold for completion-mode direct members. | `source/slang/slang-lookup.cpp:299–301` adds `Completion`; lines 316–317 call that initializer with `None`. Lines 197–212 omit the exclusion test present at lines 230–231. | Say the entry point initially supplies `None`, with completion-token detection still applicable. Restrict the direct-member `declToExclude` statement to named lookup. |
| F-003 | minor | `### Unqualified lookup`, lines 359–362 | `request.name` is not a field of `LookupRequest`. The requested name is passed separately. | `source/slang/slang-ast-support-types.h:1585–1605` declares no `name` field; `source/slang/slang-lookup.cpp:222` calls `getDirectMemberDeclsOfName(name)`. | Replace `request.name` with “the `name` argument.” |
| F-004 | minor | `## Edge cases and failure modes`, lines 1057–1064 | “Reports … except in language-server mode” implies that language-server mode suppresses `RequiredConstraintIsNotChecked`. It changes the returned result, but the diagnostic has already been emitted. | `source/slang/slang-check-expr.cpp:1428–1435` diagnoses and sets `outDiagnosed` before testing `isInLanguageServer()`. | State that both modes diagnose; language-server mode subsequently returns the original unfiltered result. |
| F-005 | minor | `### Unqualified lookup`, lines 275–280 | The walk does not necessarily terminate “at the module root.” It follows parents until null unless a qualifying hit stops it earlier; a module scope can have an outer parent. | `source/slang/slang-lookup.cpp:831` tests `scope != endScope`, with null `endScope`; `source/slang/slang-parser.cpp:6653–6676` establishes `outerScope` before pushing the program scope, and lines 172–174 assign the parent. | Replace “terminates at the module root” with “continues until the parent chain reaches null, unless the early-stop rule below applies.” |
| F-006 | minor | `### Transparent members`, lines 596–599 | The document explicitly cites `slang-compile-request.cpp` outside its watched paths. Acknowledging this does not satisfy the family contract’s requirement to expand the watched set. | `docs/generated/design/_meta/prompts/_common.md`, “Name-resolution family contract,” Scope rule; `regenerate.py show` omits this file. The cited behavior exists at `source/slang/slang-compile-request.cpp:691–698`. | Remove this peripheral translation-unit language-resolution claim and citation, or add the file to this document’s manifest watched paths and record that expansion. |
| F-007 | minor | `### Transparent members`, lines 584–607; `#### Deduplication`, lines 802–814 | The page expands into parser dialect/instance-name mechanics and pairwise overload ranking, despite the contracts excluding parsing details and overload-ranking explanations. | `docs/generated/design/_meta/prompts/_common.md`, name-resolution forbidden content; `docs/generated/design/_meta/prompts/name-resolution-lookup.md`, Forbidden content. The detailed implementations are at `source/slang/slang-parser.cpp:4368–4411` and `source/slang/slang-check-expr.cpp:1441–1517`. | Retain the lookup-relevant statement that unnamed buffer blocks produce transparent variables, with its producer citation. Reduce the ranking discussion to the caller handoff and the existing overload-resolution link. |
