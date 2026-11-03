---
review_report: true
reviewer_model: gpt-6
reviewed_at: 2026-11-03T06:44:10Z
target_doc: name-resolution/overload-resolution.md
target_doc_source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_watched_paths_digest: ed4c628981e69f8de0d5fb8f66d904cf847d3ad3d10c67e7ad56242aa4190b49
source_commit: c63c2fbd5115a98b2b2469f43618671653d40bf1
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 5
severity_breakdown:
  critical: 1
  major: 2
  minor: 2
  nit: 0
---

# Review report for name-resolution/overload-resolution.md

## Summary

The document covers the required sections and accurately describes most of the resolution pipeline, but needs correction before acceptance. Most importantly, its l-value surcharge example misstates how ordinary call arguments are ranked. It also describes removed breadcrumb symbols and omits consequential lookup-comparator rules.

## Items checked

- Verified against the **current working tree**; `git diff` confirmed that `source/` matches the document’s source commit. Confirmed HEAD matches the supplied report commit.
- Read the review prompt, common generation contract, document-specific prompt, and resolved manifest entry.
- Checked all 189 line-number citation occurrences, including repeated citations and prose-only references.
- Spot-checked more than 20 behavioral claims: candidate enums, probe/finalize sequencing, arity, fixity, coercion, mutability, constraints, visibility, candidate retention, inference-failure diagnostics, partial application, comparator ordering, legacy fallback, operator eligibility, GLSL gates, conversion caching, and diagnostic deduplication.
- Checked every conversion-cost constant and numeric value against `slang-ast-support-types.h`.
- Resolved all 134 Markdown link occurrences, checked peer-page manifest membership and referenced heading anchors, and consulted dependency passages only for cross-reference checks.
- Verified required front-matter fields, section coverage, Mermaid conventions, Liquid safety, and the 73,117-byte document against its 90,112-byte cap.
- No files were written.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | `## Conversion costs`, line 594 | The `kConversionCost_LValueCast` row says the surcharge applies “whenever the source is an l-value,” and that `f(v)` with an ordinary `float` parameter pays it. Overload probing constructs the source qualification from the **parameter’s** l-value requirement, so that example does not acquire the surcharge merely because `v` is a variable. The surcharge also belongs to the initializer-conversion branch, rather than every conversion path. | `source/slang/slang-check-overload.cpp:897` constructs `QualType(arg.type, paramType.isLeftValue)` and passes it to `canCoerce` at line 914. `source/slang/slang-check-conversion.cpp:3017-3024` conditionally adds the surcharge within the selected-initializer branch; earlier paths, such as matrix-layout conversion at line 2370, return independently. | Describe the surcharge as conditional on the `QualType` reaching that initializer-conversion branch. Remove the ordinary `f(v)` versus `f(1)` example and the claim that this surcharge explains ordinary exact-match preference. |
| F-002 | major | `### The general path`, lines 1085–1091 | The receiver explanation cites nonexistent `Breadcrumb::Kind::This` and `ThisParameterMode` symbols. It also incorrectly says explicit member lookup for call/subscript operators necessarily creates such a breadcrumb. The current representation distinguishes implicit `ThisExpr` and `ThisType` breadcrumbs; explicit operator lookup retains the supplied receiver expression. | `source/slang/slang-ast-support-types.h:1388-1426` declares `ThisExpr`, `ThisType`, and the scope invariant. Neither obsolete symbol occurs in `source/slang`. `source/slang/slang-lookup.cpp:1067` begins member lookup with no breadcrumb, while `source/slang/slang-check-expr.cpp:5394-5399` passes the existing callee expression as the member base. | Replace this paragraph’s breadcrumb claim with the explicit receiver flow through `createLookupResultExpr` and `context.baseExpr`. For implicit receiver lookup, reference `ThisExpr`/`ThisType` and link to `lookup.md`; remove `ThisParameterMode` and its listed values. |
| F-003 | major | `### Tie-breaking comparator`, step 3, lines 710–731 | The required tie-breaking account substantially abbreviates `CompareLookupResultItems`, omitting rules that can determine the winner: non-generic versus generic extensions, derived aggregate members, subscript accessor supersets, and explicit versus synthesized declarations. Its stated non-extension preference also omits the synthesized-default-constructor exception. | `source/slang/slang-check-overload.cpp:2024-2053` contains the constructor exception and generic-extension comparison; lines 2081–2095 compare aggregate ancestry; lines 2100–2137 compare subscript accessor sets; lines 2143–2149 prefer declarations that are not effectively synthesized. | Extend step 3 with these rules in source order. Explicitly qualify the non-extension preference with the synthesized-default-constructor exception. Keep the additions within the existing comparator section. |
| F-004 | minor | `## Conversion costs`, lines 599–601 | The citation locating `canConvertImplicitly` at line `3328` points into cast-expression construction, 136 lines before the named function. Other citations in the document already use the correct location. | `source/slang/slang-check-conversion.cpp:3328` is `castExpr->functionExpr = typeExpr;`; `canConvertImplicitly` is defined at lines 3464–3470. | Change this citation from `3328` to `3464-3470`. |
| F-005 | minor | Source citations at lines 355, 789, 826, 990, and 996 | The page cites five source files absent from its watched paths: `slang-diagnostics.lua`, `slang-check-modifier.cpp`, `slang-lookup.cpp`, `slang-check-decl.cpp`, and `slang-compile-request.cpp`. The family contract requires expanding watched paths when such citations are necessary, with a corresponding note in the page. | `docs/generated/design/_meta/manifest.yaml:716-734` lists the watched paths and excludes these files. `docs/generated/design/_meta/prompts/_common.md`, “Name-resolution family contract,” explicitly requires expanding the manifest for citations outside those paths. | Add these five `source/slang/` paths to this document’s manifest entry and briefly note the expansion in `## Source`, or remove the direct implementation claims and defer to appropriate peer pages. |
