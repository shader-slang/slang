---
remediation_report: true
remediator_model: claude-opus-5
remediated_at: 2026-08-04T14:45:00Z
target_doc: name-resolution/overload-resolution.md
review_report: ../../reviews/name-resolution/overload-resolution.md.review.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: 53b76e6d3009b8e6434d41573524c7ce5c499d23
actions:
  fixed: 5
  rejected_bogus: 0
  rejected_out_of_scope: 1
  deferred: 1
  escalated: 0
---

# Remediation report for name-resolution/overload-resolution.md

## Summary
Five findings were verified at the recorded commit and fixed: the intro's
unqualified bypass claim, two wrong candidate-source bullets, the
`kConversionCost_RankPromotion` meaning, the invented partial-generic closing
contexts, and an unsupported edge-case bullet that was removed. The digest
finding is reserved for the operator, and the `core.meta.slang` scope finding
is deferred because it needs a manifest change. Breakdown: 5 fixed, 0
rejected-bogus, 1 rejected-out-of-scope, 1 deferred, 0 escalated.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Confirmed: `source/slang/slang-check-expr.cpp:4794-4797` returns `nullptr` for GLSL-scope matrix operators and vector equality, and `:4821-4822` returns `nullptr` for mixed-type shifts, so those operands resume normal resolution. | Intro: bypass restricted to the operand shapes `convertToBuiltinArithmeticOp` accepts, with one sentence naming the declined cases that fall through to the general path. |
| F-002 | fixed | Confirmed: `source/slang/slang-check-overload.cpp:3098-3101` takes one `LookupResultItem` and dispatches by decl kind, while `:3166-3181` is the `LookupResult` iterator. `:2612-2623` sets no `exprVal`; `AddFuncExprOverloadCandidate` at `:2625-2639` does, and is called for a function-typed `ParamDecl` at `:3151`. | `### Probe phase`: `AddDeclRefOverloadCandidates` bullet rewritten as the per-item dispatcher naming `AddOverloadCandidates` as the iterator; `AddFuncOverloadCandidate(FuncType*)` bullet extended with the `AddFuncExprOverloadCandidate` sibling. |
| F-003 | fixed | Confirmed: `source/slang/slang-ast-support-types.h:121-124` groups the constant under "Conversion that is lossless and keeps the 'kind' of the value the same"; nothing preserves rank. | `## Conversion costs` table: `kConversionCost_RankPromotion` meaning changed to "lossless promotion to a higher rank within the same conversion kind". |
| F-004 | fixed | Confirmed: `source/slang/slang-check-overload.cpp:3228-3239` is the only semantic consumer that resumes inference, handing `baseGenericDeclRef` and `providedOrdinaryArgs` to `addOverloadCandidatesForCallToGeneric`. `source/slang/slang-check-expr.cpp:2345` only unwraps the base. No ascription or second generic-application path exists. | `## Partial generic application`: the speculative "type ascription / later `GenericAppExpr<...>` / another argument" sentence replaced with the call-site inference path plus its citation. |
| F-005 | fixed | Confirmed unsupported: `AddFuncExprOverloadCandidate` (`source/slang/slang-check-overload.cpp:2625-2639`) never sets `candidate.item`, while `CompareLookupResultItems` dereferences `left.declRef.getDecl()` at `:1926` and `CompareOverloadCandidates` ranks by `left->item.declRef` at `:2428-2434`. The per-doc prompt's required edge-case list (`docs/generated/design/_meta/prompts/name-resolution-overload-resolution.md:91-105`) does not include this case. | `## Edge cases and failure modes`: the "First-class function value vs declared callable" bullet deleted. |
| F-006 | rejected-out-of-scope | `docs/generated/design/_meta/prompts/_remediate.md:97-100` reserves `watched_paths_digest` for the operator's `mark-fresh` run and forbids the remediator from editing it. | — |
| F-007 | deferred | The finding is correct: `docs/generated/design/_meta/manifest.yaml:528-536` omits `source/slang/core.meta.slang`, so `_common.md:185-188` is violated. The remedy the scope rule prescribes is a `watched_paths` expansion, which `_remediate.md:93-94` forbids the remediator from making and `:72-78` names as the archetypal deferral. Follow-up: add `source/slang/core.meta.slang` to this page's `watched_paths`, then link the cited `vector<T,4>` and `__init<T : __EnumType>` declarations. Deleting the claims instead would drop accurate cost-model content the prompt asks for. | — |
