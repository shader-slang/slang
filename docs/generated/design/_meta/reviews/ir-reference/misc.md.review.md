---
review_report: true
reviewer_model: gpt-6
reviewed_at: 2026-11-03T08:07:11Z
target_doc: ir-reference/misc.md
target_doc_source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_watched_paths_digest: 1155790e3696c9cddf4f2405223ad2c7acc6846e935a3f2e3b8e67beb91416f3
source_commit: 8341c7e537a3f5cb3a60cd062066ffd630da40d8
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: partial
  source_alignment: partial
  front_matter_validity: pass
finding_count: 7
severity_breakdown:
  critical: 2
  major: 3
  minor: 1
  nit: 1
---

# Review report for ir-reference/misc.md

## Summary

The document needs corrections before acceptance. Most checked encodings and citations match the source, but the `GetArrayLength` entry incorrectly denies an intrinsic lowering producer, and the expansion callout attributes specialization behavior to the wrong pass. Coverage and several mandatory table conventions also need correction.

## Items checked

- Read `_review.md`, `_common.md`, the document-specific prompt, and `regenerate.py show ir-reference/misc.md`.
- Verified against the current working tree, which the supplied review context identifies as matching the document’s source commit throughout `source/`.
- Checked all located line-number citations, covering 135 distinct source locations or ranges, including repeated citations and implicit file references.
- Spot-checked more than 20 behavioral claims, including capability construction, pack branching, witness packs, type predicates, size/count queries, opaque-handle lowering, storage casts, annotation caching, dictionary keys, string-hash validation, CUDA launch encoding, and barrier emission.
- Resolved all 108 Markdown links across 50 distinct destinations; checked generated-page manifest membership and the `values.md#conversions` anchor.
- Compared the 66 opcode rows with sibling tables and Lua declarations. Found no duplicate ownership; checked the stated count of 12 handwritten wrappers.
- Checked required sections, table columns, front-matter keys and formats, Mermaid syntax, and Liquid safety. Dependency pages were consulted only for cross-reference checks.
- Performed read-only inspection; no files were written.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | `## Opcodes`, line 306 | `GetArrayLength` is labeled `no lowering producer`, and the summary claims `Array<T>::getCount` follows `visitGetArrayLengthExpr`. These are different paths: the intrinsic declaration produces the opcode through ordinary intrinsic call lowering. A later fold does not establish absence of a producer. | `source/slang/core.meta.slang:2330` declares `getCount()` with `kIROp_GetArrayLength`; `source/slang/slang-lower-to-ir.cpp:991–1019` emits intrinsic opcodes through `emitCallToDeclRef`. `source/slang/slang-ir-peephole.cpp:411–416` subsequently replaces `GetArrayLength` with the array extent. | Name `Array<T,N>::getCount` via `emitCallToDeclRef` as the producer. Distinguish this from the synthesized `GetArrayLengthExpr` visitor, which directly returns the extent. Remove the claim that absence from a dump proves no instruction was constructed. |
| F-002 | critical | `### Expand and Each`, lines 489–491; opcode row 221 | The callout says `slang-ir-lower-expand-type.cpp` “re-emits the pattern once per slot once the pack length is known.” That pass constructs an indexed `IRExpand` region and rewrites `Each` accesses; specialization performs the per-element expansion. The row also omits this intermediate producer for `ExpandType` and `ExpandIntValPack`. | `source/slang/slang-ir-lower-expand-type.cpp:86–108` constructs `IRExpand`; lines 24–32 convert `Each` to tuple access. `source/slang/slang-ir-specialize.cpp:3317–3408` determines cardinality and clones the body per element. `source/slang/slang-lower-to-ir.cpp:2221,2255` initially builds `ExpandTypeOrVal`. | Attribute region construction and `Each` rewriting to `lowerExpandTypeImpl`/`clonePatternValImpl`; attribute per-slot expansion to `maybeSpecializeExpand`. Clarify the direct `ExpandExpr` producer versus the `ExpandTypeOrVal` conversion path. |
| F-003 | major | `### Coverage gaps against sibling pages`, lines 463–473 | `Every concrete Lua entry no other ir-reference/ page claims is listed above` is false: `reportOptiXIntersection` is absent from every family opcode table. | `source/slang/slang-ir-insts.lua:1670` declares the concrete opcode; `source/slang/hlsl.meta.slang:21379–21381` declares its intrinsic producer. The sibling table census contains no corresponding row. | Assign this opcode to the ray-tracing section of `resources-and-atomics.md`, update the relevant prompts as required by the family-specific routing rule, and cross-link that ownership here. Correct the exhaustive-coverage claim until ownership is complete. |
| F-004 | major | Operand cells, lines 330–331, 338, 388–389, 406–407 | Six rows use the explicitly forbidden `(variadic, min=N)` notation for unnamed minimum operands. `liveRangeEnd` instead supplies an accessor-derived name without the required `†` explanation. | `source/slang/slang-ir-insts.lua:1672,2802–2803,2998,3004–3005` supplies minimum counts or an empty entry, not those variadic declarations. `source/slang/slang-ir-insts.h:2651` supplies `getReferenced()`. `_common.md`, IR-reference operand-column contract, requires `(N unnamed)` or explained accessor-derived names. | Use `(2 unnamed)` for the two storage casts, `Annotation`, `WitnessTableAnnotation`, and `liveRangeStart`; use `(1 unnamed)` for `GetLegalizedSPIRVGlobalParamAddr`. Mark `referenced` as accessor-derived with `†` and add a legend. Preserve builder discrepancies in the explanatory prose. |
| F-005 | major | AST-origin cells, lines 221–232, 258, 303–305, 459 | Numerous AST-produced rows name only AST classes or line numbers, omitting the actual `visit*` producer required by the column contract. | `_common.md`, IR-reference AST-origin contract, requires the AST class plus visitor. Examples exist at `source/slang/slang-lower-to-ir.cpp:2304` (`visitTypePackSubtypeWitness`), 5448 (`visitSizeOfLikeExpr`), 5509 (`visitPackQueryExpr`), 5531 (`visitShapePackTransformExpr`), and 6991 (`visitIsTypeExpr`). | Add the relevant visitor names to each affected AST-origin cell, including the shared pack-query, shape-transform, and size-query visitors. For indirect production, identify the intermediate pass as described in F-002. |
| F-006 | minor | `## Source`, lines 95–102 | The blanket statement that cited `slang-ir-*.cpp` passes “are not watched” is stale. Several cited producer passes are explicitly watched. | `docs/generated/design/_meta/manifest.yaml:1013–1018` lists peephole, PyTorch binding, buffer-element lowering, liveness, phi elimination, and translation sources. | State that the entire glob is excluded but a selected subset is watched. Limit the freshness caveat to producers outside that subset. |
| F-007 | nit | Introduction, lines 12–27; opcode summary line 458 | The intended reader appears in a separate second paragraph, although the universal contract requires it in the first. The dictionary-value summary also uses `*weakly*`, contrary to the summary-column restriction to inline-code markup only. | `docs/generated/design/_meta/prompts/_common.md`, universal content rules and IR-reference Summary-column contract. | Merge the intended-reader sentence into the opening paragraph and remove emphasis markup around “weakly.” |