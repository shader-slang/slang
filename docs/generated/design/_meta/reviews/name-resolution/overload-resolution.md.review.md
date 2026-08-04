---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-12T13:39:11+00:00
target_doc: name-resolution/overload-resolution.md
target_doc_source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_watched_paths_digest: d1038851c2165bad76b25d0691b064ceae331bd0dab7b88d9c69cfbfdc90dff2
source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: pass
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 5
severity_breakdown:
  critical: 0
  major: 3
  minor: 2
  nit: 0
---

# Review report for name-resolution/overload-resolution.md

## Summary
The page has the required structure and all relative links resolve, but several algorithm details do not match the watched source. The most important issue is that the type-checking and direction-checking steps describe implicit-conversion rejection and parameter-direction enforcement that the overload pipeline does not actually perform in those functions.

## Items checked
- Ran `regenerate.py show name-resolution/overload-resolution.md` and verified the target prompt, size cap, dependencies, and five resolved watched source files.
- Read the target document including front matter, the per-doc prompt, `_common.md`, `_review.md`, and the dependency pages `lookup.md`, `visibility.md`, `expressions.md`, `values.md`, and `glossary.md`.
- Resolved all 57 Markdown links in the target document, including source links, peer name-resolution links, AST reference links, pipeline links, and the glossary link.
- Verified the required sections: `## Source`, `## Concepts`, `## Algorithm`, `## Conversion costs`, `## Partial generic application`, `## Operator overloading`, `## Edge cases and failure modes`, and `## See also`.
- Spot-checked more than 10 factual/source-alignment claims against `slang-check-overload.cpp`, `slang-check-impl.h`, `slang-check-conversion.cpp`, `slang-ast-support-types.h`, and `slang-ast-expr.h`, including candidate flavors/statuses, `JustTrying`/`ForReal`, filter-step order, conversion-cost accumulation, comparator tie-breaking, partial generic storage, operator cache keys, and diagnostics.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Algorithm`, lines 180-189 and `## Finalize phase`, lines 315-320 | The page treats `Flavor::UnspecializedGeneric` as if it is processed by `TryCheckGenericOverloadCandidateTypes` and can be finalized into a `PartiallyAppliedGenericExpr`. In source, `TryCheckOverloadCandidateTypes` only delegates for `Flavor::Generic`, while `UnspecializedGeneric` is created after inference fails with `Status::GenericArgumentInferenceFailed` and is handled by the early error path in `CompleteOverloadCandidate`, not by the partial-generic construction switch. | `source/slang/slang-check-overload.cpp:791-794` delegates only `Flavor::Generic`; `source/slang/slang-check-overload.cpp:2846-2851` creates an `UnspecializedGeneric` candidate with `GenericArgumentInferenceFailed`; `source/slang/slang-check-overload.cpp:1393-1405` emits the inference-failed diagnostic before the final construction switch. | Remove `UnspecializedGeneric` from the type-check delegation and partial-generic construction descriptions; describe it as the recorded failed-inference candidate used for diagnostics. |
| F-002 | major | `## Algorithm`, lines 190-197 and `## Conversion costs`, lines 325-383 | The page says each argument's cost is computed by `getImplicitConversionCostWithKnownArg` and first rejected by `canConvertImplicitly`, with only accepted costs contributing to ranking. The overload candidate type step actually calls `canCoerce(..., &cost)` and directly adds that cost; `getImplicitConversionCostWithKnownArg` is used in conversion-overload handling, and high-cost conversions can remain possible so they can be selected and diagnosed when reified. | `source/slang/slang-check-overload.cpp:824-837` calls `canCoerce` and adds `cost`; `source/slang/slang-check-conversion.cpp:2465-2482` and `source/slang/slang-check-conversion.cpp:2543-2549` show `getImplicitConversionCostWithKnownArg` in conversion-overload handling; `source/slang/slang-check-conversion.cpp:2551-2556` says too-high implicit conversion costs are reported as possible so overloads involving them can be selected. | Rewrite the conversion-cost prose to say overload candidate probing uses `canCoerce` and accumulates its reported cost, and reserve `canConvertImplicitly`/`getImplicitConversionCostWithKnownArg` for the contexts where the source actually uses them. |
| F-003 | major | `## Algorithm`, lines 198-203 | The page says `TryCheckOverloadCandidateDirections` enforces `in`, `out`, `inout`, and `ref` parameter directions and rejects non-l-values for output parameters. The function's source comment says that l-value/parameter checking is currently done elsewhere and that this step only checks mutability of the implicit `this` parameter where necessary. | `source/slang/slang-check-overload.cpp:1018-1024` says direction/l-value checking is done elsewhere and this step checks only `this` mutability; `source/slang/slang-check-overload.cpp:1030-1039` rejects a mutating method on an immutable base expression. | Replace the direction-step description with the actual mutating-`this` check and name `MutatingMethodOnImmutableValue`/related diagnostics, rather than claiming general output-argument l-value enforcement here. |
| F-004 | minor | `## Partial generic application`, lines 442-446 | The page says `PartiallyAppliedGenericExpr` carries "the bound substitution and the unresolved generic parameters." The AST node stores `baseGenericDeclRef` and `providedOrdinaryArgs`; the source comment explicitly says witness arguments are not stored there and are formed later after remaining ordinary arguments are inferred. | `source/slang/slang-ast-expr.h:955-964` lists `originalExpr`, `baseExpr`, `baseGenericDeclRef`, and `providedOrdinaryArgs`; `source/slang/slang-check-overload.cpp:1552-1558` stores only ordinary arguments from the candidate substitution. | Rephrase the data-shape claim to say the node carries the generic decl-ref plus the already-provided ordinary argument prefix; avoid saying it stores unresolved parameters or a full bound substitution. |
| F-005 | minor | `## Operator overloading`, lines 462-476 | The operator-cache key description omits `isGLSLMode` and says the key captures the operator name plus operand `BasicType` pair. The source key has `operatorName`, `isGLSLMode`, and two `BasicTypeKey` entries; `fromOperatorExpr` fills `operatorName` from an `IntrinsicOpModifier` on a candidate declaration rather than directly from the surface operator name. | `source/slang/slang-check-impl.h:197-209` declares `operatorName`, `isGLSLMode`, and `args[2]`; `source/slang/slang-check-impl.h:230-264` sets `operatorName = intrinsicOp->op`; `source/slang/slang-check-overload.cpp:3166-3172` sets `key.isGLSLMode` before cache lookup. | Update the cache-key prose to mention `isGLSLMode` and that the cached operator identifier is derived from the intrinsic op modifier found on applicable overloaded definitions. |

## No-issues notes
- The target front matter has every required key, and the digest value is a 64-character lowercase hexadecimal SHA-256-like string.
- The probe-phase order in the Mermaid diagram matches `TryCheckOverloadCandidate` for arity, fixity, types, directions, constraints, visibility, and applicable status.
- The partial-generic flag sites at lines 413, 490, 578, and 665 and the final `PartiallyAppliedGenericExpr` creation at line 1547 are real source locations.
- Ambiguous and no-applicable diagnostic identifiers in the edge-case section match entries in `slang-diagnostics.lua`.
